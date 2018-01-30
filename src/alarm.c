/*
 ===============================================================================
 Name        : alarm_12_2017.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */
//TODO: Change INDCT to start a timer and blink when armed
//TODO: Swap INDCT and Siren names

#include "chip.h"
#include <cr_section_macros.h>
#include "sys_config.h"
#include "alarm_settings.h"
#include "menus.h"
#include <stdio.h>
#include <string.h>

volatile uint32_t systemTick;
volatile uint8_t updateTime = 0;
volatile uint8_t onPressed = 0;
extern struct users_S *c_user;
extern struct users_S users[];
extern struct ALARM_SYSTEM_S automation_O[];
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S motion_lights[];
extern struct LIGHT_AUTO_S light_auto[];
extern struct X_LIGHT_AUTO_S x_light_auto[];
extern uint32_t timeOut;
extern uint32_t countDown;
extern uint32_t delayInt;

enum ALARMSTATE_T
{
	ARM, ARMING, ARMED, DISARM, DISARMED, ACTIVATE_ALARM, ALARM_ACTIVATED
};
enum ARMEDSTATE_T
{
	AWAY, AWAY_NO_INT_AUTO, AWAY_NO_EXT_AUTO, AWAY_NO_AUTO, STAY
};

uint8_t ARM_DELAY = ARM_DELAY_D;  //ee
uint8_t ENTRY_DELAY = ENTRY_DELAY_D;  //ee
uint8_t DARK_THRESHOLD = DARK_THRESHOLD_D; //ee

enum ALARMSTATE_T ALARMSTATE = DISARM;
enum ARMEDSTATE_T ARMEDSTATE = STAY;
uint8_t readyToArm = 2;
uint8_t pinAttempts;
uint8_t dispDimmed;
uint32_t dimTimer;
uint8_t intLightsState;
uint8_t extLightsState;
uint8_t activeSensors[17];

STATIC INLINE void updateDisplayTime(void)
{
	dispDateTime();
	updateTime = 0;
}

STATIC INLINE void setCountDown(uint32_t secs)
{
	timeOut = ENABLE;
	delayInt = secs;
	Chip_TIMER_Reset(LPC_TIMER0);
	Chip_TIMER_Reset(LPC_TIMER1);
	LPC_TIMER0->PR = countDown;
	Chip_TIMER_SetMatch(LPC_TIMER1, 0, secs * 1000);
	Chip_TIMER_Enable(LPC_TIMER1);
	Chip_TIMER_Enable(LPC_TIMER0);

}

STATIC INLINE void disableCountDown(void)
{
	Chip_TIMER_Disable(LPC_TIMER0);
	Chip_TIMER_Disable(LPC_TIMER1);
	Chip_TIMER_ClearMatch(LPC_TIMER1, 0);
	//NVIC_ClearPendingIRQ(TIMER1_IRQn);

	timeOut = DISABLE;

	setIOpin(&automation_O[ARM_I], DISABLE);
}

STATIC INLINE void flashARMLED()
{
	setIOpin(&automation_O[ARM_I], ENABLE);
	pause(100);
	setIOpin(&automation_O[ARM_I], DISABLE);
	updateTime = 0;
}

uint8_t getPIN(void);
void checkMenu(void);
void checkAlarmSubMenu(void);
void checkIntLightSubMenu(void);
void checkExtLightSubMenu(void);
void changeTimeMenu(void);
void checkStatus(void);
void inputBuffers(void);
void changeDarkTH(void);
void changeArmDelay(void);
void armingDelay(void);
void changeEntryDelay(void);
uint8_t pollAlarmSensors(void);
uint8_t pollAutomation(void);
uint8_t checkReadyToArm(void);
uint8_t entryDelay(uint8_t active);
void editSensor(uint8_t sensorid);
void checkMotionLightStatus(void);
void editMotionLightSensor(uint8_t sensorid);
void showAllSensorStat(void);
void showAllXMSStat(void);
void checkPIN(void);

int main(void)
{
	setUpSystem();
	IN_BUFF_ON();

	uint32_t stateTimer = systemTick;
	dimTimer = systemTick;
	uint8_t sensorsActive = 0;

	while (1)
	{
		if (CHECK_STATE_TIMER())
		{
			switch (ALARMSTATE)
			{
			case ARM:
				dispClear();
				ENABLE_ON_PWR();
				ALARMSTATE = ARMED;
				break;
			case ARMING:
				armingDelay();
				ALARMSTATE = ARM;
				break;
			case ARMED:
				sensorsActive = pollAlarmSensors();
				pollAutomation();
				if (sensorsActive)
					if (!entryDelay(sensorsActive))
						ALARMSTATE = ACTIVATE_ALARM;
				if (updateTime) flashARMLED();
				break;
			case DISARM:
				setIOpin(&automation_O[SIREN], DISABLE);
				RESET_PIN_ATTEMPTS();
				DISABLE_ON_PWR();
				setIOpin(&automation_O[ERROR], DISABLE);
				dispMainDARD(c_user->name);
				displayNormal();
				displayON();
				ALARMSTATE = DISARMED;
				readyToArm = 2;  //SO IT DISPLAYS AFTER WRONG KEYPRESS
				break;
			case DISARMED:
				if (updateTime)
					updateDisplayTime();
				if (!dispDimmed && CHECK_DIM_TIMER())
					displayDim();
				if (dispDimmed == 1 && CHECK_OFF_TIMER())
					displayOFF();
				if (checkReadyToArm() != readyToArm)
					displayReadyToArm();
				checkMenu();
				pollAutomation();
				break;
			case ACTIVATE_ALARM:
				setIOpin(&automation_O[SIREN], ENABLE);
				//
				ALARMSTATE = ALARM_ACTIVATED;
				break;
			case ALARM_ACTIVATED:
				checkPIN();
				break;
			}
			RESET_STATE_TIMER();
		}
	}
}

uint8_t getPIN(void)
{
	uint32_t on_pressed_time = systemTick;
	uint32_t kpData[4] = { 0, 0, 0, 0 };
	uint8_t match = 0;

	while (ON_PRESSED())
	{
		if (systemTick > (on_pressed_time + ON_PRESSED_TIMEOUT))
		{
			ALARMSTATE = ACTIVATE_ALARM;
			return 0;
		}
	}
	onPressed = 0;
	DISABLE_ON_PWR();
	for (uint8_t keys = 0; keys < 4; keys++)
	{

		kpData[keys] = getKP(KP_TIMEOUT_DEFAULT_MS + 1000);
		on_pressed_time = systemTick;
		while (getKP(150))
		{
			if (systemTick > (on_pressed_time + KP_TIMEOUT_DEFAULT_MS))
			{
				ENABLE_ON_PWR();
				pinAttempts++;
				return 0;
			}
		}
		sendData('*');
		if (!kpData[keys])
		{
			ENABLE_ON_PWR();
			pinAttempts++;
			return 1;
		}
	}
	for (uint8_t usersLoop = 1; usersLoop < 5; usersLoop++)
	{
		for (uint8_t pinLoop = 0; pinLoop < 4; pinLoop++)
		{
			if (kpData[pinLoop] == users[usersLoop].pin[pinLoop])
				match++;
		}
		if (match == 4)
		{
			c_user = &users[usersLoop];
			ALARMSTATE = DISARM;
			return 2;
		}
		else
			match = 0;
	}
	ENABLE_ON_PWR();
	pinAttempts++;
	return 0;
}

void checkMenu(void)
{
	uint32_t selection = 0;

	selection = getKP(KP_TIMEOUT_DEFAULT_MS);
	while (getKP(100))
	{
		__NOP();
	}

	if (selection)
	{
		if (dispDimmed)
			displayNormal();
		ALARMSTATE = DISARM;
		switch (selection)
		{
		case KP_Mplus:
			if (!readyToArm || !OE_INPUT_ON())
				return;
			checkAlarmSubMenu();
			break;
		case KP_MRC:
			checkIntLightSubMenu();
			break;
		case KP_Mminus:
			checkExtLightSubMenu();
			break;
		case KP_root:
			changeTimeMenu();
			break;
		case KP_equal:
			checkStatus();
			break;
		case KP_pcnt:
			inputBuffers();
			break;
		case KP_div:
			changeDarkTH();
			break;
		case KP_plus:
			changeArmDelay();
			break;
		case KP_minus:
			changeEntryDelay();
			break;
		case KP_dec:
			checkMotionLightStatus();
			break;
		}
	}
}
void checkAlarmSubMenu(void)
{
	uint32_t selection = 0;

	dispArmedType();
	selection = getKP(KP_TIMEOUT_SUBMENU_MS);
	while (getKP(100))
	{
		__NOP();
	}
	if (selection)
	{
		switch (selection)
		{
		case KP_Mplus:
			ARMEDSTATE = AWAY;
			break;
		case KP_root:
			ARMEDSTATE = AWAY_NO_INT_AUTO;
			break;
		case KP_div:
			ARMEDSTATE = AWAY_NO_EXT_AUTO;
			break;
		case KP_minus:
			ARMEDSTATE = AWAY_NO_AUTO;
			break;
		case KP_equal:
			ARMEDSTATE = STAY;
			break;
		}
		ALARMSTATE = ARMING;
	}
}

void checkIntLightSubMenu(void)
{
	uint32_t selection = 0;
	uint32_t lightBit = 0;
	uint8_t lightState = 0;

	dispIntLight();
	selection = getKP(KP_TIMEOUT_SUBMENU_MS);
	while (getKP(100))
	{
		__NOP();
	}

	if (selection)
	{
		switch (selection)
		{
		case KP_7: //MAIN LIGHTS ON
			lightBit = 0;
			lightState = 1;
			break;
		case KP_4: //MAIN LIGHTS OFF
			lightBit = 0;
			lightState = 0;
			break;
		case KP_8: //SUPP LIGHTS ON
			lightBit = 1;
			lightState = 1;
			break;
		case KP_5: //SUPP LIGHTS OFF
			lightBit = 1;
			lightState = 0;
			break;
		case KP_1: //ALL LIGHTS ON
			lightBit = 2;
			lightState = 1;
			break;
		case KP_0: //ALL LIGJHTS OFF
			lightBit = 2;
			lightState = 0;
			break;
		case KP_pcnt: //FAN ON
			lightBit = 3;
			lightState = 1;
			break;
		case KP_times: //FAN OFF
			lightBit = 3;
			lightState = 0;
			break;
		default:
			return;
		}

		setIOpin(&automation_O[L_I_M], 1);
		pause(2); //give light controller time to react
		setIOpin(&automation_O[L_I_M], 1);
		pause(2); //light controller waits until LT_MAIN goes low to read

		switch (lightBit)
		{
		case 0:
			setIOpin(&automation_O[L_I_M], lightState);
			break;
		case 1:
			setIOpin(&automation_O[L_I_S], lightState);
			break;
		case 2:
			setIOpin(&automation_O[L_I_M], lightState);
			setIOpin(&automation_O[L_I_S], lightState);
			break;
		case 3:
			setIOpin(&automation_O[FAN], lightState);
		}

		pause(10);

		setIOpin(&automation_O[L_I_M], 0);
		setIOpin(&automation_O[L_I_S], 0);
		setIOpin(&automation_O[FAN], 0);
	}
}

void checkExtLightSubMenu(void)
{
	uint32_t selection = 0;

	dispExtLight();
	selection = getKP(KP_TIMEOUT_SUBMENU_MS);
	while (getKP(100))
	{
		__NOP();
	}

	if (selection)
	{/*
	 switch (selection)
	 {
	 case KP_7: //MAIN LIGHTS ON
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, LT_MAIN_p0_O);
	 break;
	 case KP_4: //MAIN LIGHTS OFF
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, LT_MAIN_p0_O);
	 break;
	 case KP_8: //SUPP LIGHTS ON
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 case KP_5: //SUPP LIGHTS OFF
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 case KP_9: //ALL LIGHTS ON
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, LT_MAIN_p0_O);
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 case KP_6: //ALL LIGJHTS OFF
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, LT_MAIN_p0_O);
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 case KP_pcnt: //FAN ON
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 2, FAN_p2_O);
	 break;
	 case KP_times: //FAN OFF
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 2, FAN_p2_O);
	 break;
	 case KP_1: //ALL LIGHTS ON
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, LT_MAIN_p0_O);
	 Chip_GPIO_SetPinOutHigh(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 case KP_0: //ALL LIGJHTS OFF
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, LT_MAIN_p0_O);
	 Chip_GPIO_SetPinOutLow(LPC_GPIO, 3, LT_SUPP_p3_O);
	 break;
	 }*/
	}
}

void changeTimeMenu(void)
{
	uint32_t selection[2] = { 0, 0 };
	uint32_t value = 0;
	RTC_TIME_T tempTime;

	//IN_BUFF_ON();

	dispTimeChange(HR);
	if (!getKPInput(selection, 2))
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > 23)
		return;
	tempTime.time[RTC_TIMETYPE_HOUR] = value;
	pause(500);

	dispTimeChange(MINIT);
	if (!getKPInput(selection, 2))
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > 59)
		return;
	tempTime.time[RTC_TIMETYPE_MINUTE] = value;
	pause(500);

	dispTimeChange(MOS);
	if (!getKPInput(selection, 2))
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > 12)
		return;
	tempTime.time[RTC_TIMETYPE_MONTH] = value;
	pause(500);

	dispTimeChange(DAY);
	if (!getKPInput(selection, 2))
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > 31)
		return;
	tempTime.time[RTC_TIMETYPE_DAYOFMONTH] = value;
	pause(500);

	dispTimeChange(YR);
	if (!getKPInput(selection, 2))
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > 50)
		return;
	tempTime.time[RTC_TIMETYPE_YEAR] = (2000 + value);
	pause(500);

	tempTime.time[RTC_TIMETYPE_SECOND] = 0;
	Chip_RTC_SetFullTime(LPC_RTC, &tempTime);
}

void checkStatus(void)
{
	uint32_t selection[2] = { 0, 0 };

	uint32_t value = 0;

	dispSensorStatus(0);
	setCursor(0, 9);
	if (!getKPInput(selection, 2))
	{
		showAllSensorStat();
		return;
	}
	if (selection[0] == 255)
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > NUM_OF_SYSTEMS)
		return;

	uint32_t menuTimer = systemTick;

	dispSensor(value);

	selection[0] = 0;

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		while (getKP(100))
		{
			__NOP();
		}
		if (selection[0])
			break;
	}

	if (selection[0] == KP_equal)
	{
		editSensor(value);
	}

	menuTimer = systemTick;

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		__NOP();
	}
}

void editSensor(uint8_t sensorid)
{
	uint32_t menuTimer = systemTick;
	uint32_t selection[3] = { 0, 0, 0 };
	uint32_t value;
	uint8_t byteStorage[4];

	dispSensorEdit(sensorid);

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		while (getKP(100))
		{
			__NOP();
		}
		switch (selection[0])
		{
		case KP_1:
			alarm_system_I[sensorid].active = (
					alarm_system_I[sensorid].active ? 0 : 1);
			saveByte((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 1,
					alarm_system_I[sensorid].active);
			break;
		case KP_2:
			alarm_system_I[sensorid].req_to_arm = (
					alarm_system_I[sensorid].req_to_arm ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 2),
					alarm_system_I[sensorid].req_to_arm);
			break;
		case KP_3:
			alarm_system_I[sensorid].armedstate = (
					alarm_system_I[sensorid].armedstate == 0 ? 4 : 0);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 3),
					alarm_system_I[sensorid].armedstate);
			break;
		case KP_4:
			alarm_system_I[sensorid].sig_active_level = (
					alarm_system_I[sensorid].sig_active_level ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 4),
					alarm_system_I[sensorid].sig_active_level);
			break;
		case KP_5:
			setCursor(0, 16);
			selection[0] = 0;
			if (!getKPInput(selection, 3))
				break;
			if (selection[0] == 255)
				break;
			value = (selection[0] * 100) + (selection[1] * 10) + selection[2];
			if (value <= 999)
				alarm_system_I[sensorid].delay = value;
			intTobytes(byteStorage, value);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 5),
					byteStorage[0]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 6),
					byteStorage[1]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 7),
					byteStorage[2]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 8),
					byteStorage[3]);
		}
		dispSensorEdit(sensorid);
	}
}
//AUTOMATION SENSORS
void checkMotionLightStatus(void)
{
	uint32_t selection[2] = { 0, 0 };

	uint32_t value = 0;

	dispSensorStatus(0);
	setCursor(0, 9);
	if (!getKPInput(selection, 1))
	{
		showAllXMSStat();
		return;
	}
	if (selection[0] == 255)
		return;
	value = selection[0];
	if (value > X_MOTION_DETECTORS)
		return;

	uint32_t menuTimer = systemTick;

	dispMotionSensor(value);

	selection[0] = 0;

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		while (getKP(100))
		{
			__NOP();
		}
		if (selection[0])
			break;
	}

	if (selection[0] == KP_dec)
	{
		editMotionLightSensor(value);
	}

	menuTimer = systemTick;

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		__NOP();
	}
}

void editMotionLightSensor(uint8_t sensorid)
{
	uint32_t menuTimer = systemTick;
	uint32_t selection[3] = { 0, 0, 0 };
	uint32_t value;
	uint8_t byteStorage[4];

	dispMotionSensorEdit(sensorid);

	while (systemTick < menuTimer + KP_TIMEOUT_SUBMENU_MS)
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		while (getKP(100))
		{
			__NOP();
		}
		switch (selection[0])
		{
		case KP_1:
			motion_lights[sensorid].active = (
					motion_lights[sensorid].active ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 1),
					motion_lights[sensorid].active);
			break;
		case KP_2:
			motion_lights[sensorid].sig_active_level = (
					motion_lights[sensorid].sig_active_level ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 2),
					motion_lights[sensorid].sig_active_level);
			break;
		case KP_3:
			setCursor(0, 8);
			selection[0] = 0;
			if (!getKPInput(selection, 3))
				break;
			if (selection[0] == 255)
				break;
			value = (selection[0] * 100) + (selection[1] * 10) + selection[2];
			if (value <= 999)
				motion_lights[sensorid].delay = value;
			intTobytes(byteStorage, value);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 3),
					byteStorage[0]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 4),
					byteStorage[1]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 5),
					byteStorage[2]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + MS_OFFSET + 6),
					byteStorage[3]);
		}
		dispMotionSensorEdit(sensorid);
	}
}
void changeDarkTH(void)
{
	uint32_t selection[3];
	uint32_t thValue = 0;

	dispDarkTH();
	if (!getKPInput(selection, 3))
		return;

	thValue = selection[0] * 100;
	thValue += selection[1] * 10;
	thValue += selection[2];

	if (thValue > 255)
		return;

	DARK_THRESHOLD = (uint8_t) thValue;
	saveByte(DARK_THRESHOLD_OFFSET, DARK_THRESHOLD);
}

void changeArmDelay(void)
{
	uint32_t selection[3];
	uint32_t adValue = 0;

	dispArmDelay();
	if (!getKPInput(selection, 3))
		return;

	adValue = selection[0] * 100;
	adValue += selection[1] * 10;
	adValue += selection[2];

	if (adValue > 255)
		return;

	ARM_DELAY = (uint8_t) adValue;
	saveByte(ARM_DELAY_OFFSET, ARM_DELAY);
}

void changeEntryDelay(void)
{
	uint32_t selection[3];
	uint32_t edValue = 0;

	dispEntryDelay();
	if (!getKPInput(selection, 3))
		return;

	edValue = selection[0] * 100;
	edValue += selection[1] * 10;
	edValue += selection[2];

	if (edValue > 255)
		return;

	ENTRY_DELAY = (uint8_t) edValue;
	saveByte(ENTRY_DELAY_OFFSET, ENTRY_DELAY);
}

void SysTick_Handler(void)
{
	systemTick++;
}

void RTC_IRQHandler(void)
{
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	updateTime = 1;
}

void inputBuffers(void)
{
	uint32_t selection[1];
	selection[0] = 0;
	dispInputBuffers();
	if (!getKPInput(selection, 1))
		return;
	if (selection[0] > 1)
		return;
	if (!selection[0])
	{
		IN_BUFF_OFF();
	}
	else
		IN_BUFF_ON();
}

uint8_t pollAlarmSensors(void)
{
	uint8_t sensor = 0;
	uint8_t numActiveSensors = 0;
	if (OE_INPUT_ON())
	{
		for (sensor = 0; sensor < NUM_OF_SYSTEMS; sensor++)
		{
			if (alarm_system_I[sensor].active)
			{
				if (getIOpin(&alarm_system_I[sensor]))
				{
					activeSensors[numActiveSensors] = sensor;
					numActiveSensors++;
				}
			}
		}
	}
	return numActiveSensors;
}

uint8_t checkReadyToArm(void)
{
	uint8_t trippedSensors = 0;
	trippedSensors = pollAlarmSensors();

	if (!OE_INPUT_ON())
	{
		setIOpin(&automation_O[ERROR], ENABLE);
		return 0;
	}
	else
		setIOpin(&automation_O[ERROR], DISABLE);

	for (uint8_t t_s = 0; t_s < trippedSensors; t_s++)
	{
		if (alarm_system_I[activeSensors[t_s]].req_to_arm)
		{
			return 0;
		}
	}
	return 1;
}

void armingDelay(void)
{
	setCountDown(ARM_DELAY);
	displayArming();
	while (timeOut)
	{
		__NOP();
	}
}

uint8_t entryDelay(uint8_t active)
{
	if (activeSensors[0] == DOOR_MAIN)
	{
		setCountDown(ENTRY_DELAY);
		while (timeOut)
		{
			if (onPressed)
			{
				displayPIN();
				if (getPIN() == 2)
					return 1;
				else
					dispClear();
				if (PIN_TRIES_EXCEEDED())
					return 0;
			}
		}
	}
	return 0;
}

uint8_t pollAutomation(void)
{
	uint8_t sensor = 0;
	uint8_t numActiveSensors = 0;
	RTC_TIME_T checkTime;

	if (OE_INPUT_ON() && isDark(1))
	{
		for (sensor = 0; sensor < X_MOTION_DETECTORS; sensor++)
		{
			if ((motion_lights[sensor].active) && (motion_lights[sensor].timestamp == 0))
			{
				if (getIOpin(&motion_lights[sensor]))
				{
					setIOpin(&automation_O[motion_lights[sensor].device], 1);
					motion_lights[sensor].timestamp = systemTick;
				}
			}
			//check if time to turn off
			if (motion_lights[sensor].timestamp > 0)
			{
				if (systemTick > motion_lights[sensor].timestamp + (motion_lights[sensor].delay * (1000 * 60)))
				{
					setIOpin(&automation_O[motion_lights[sensor].device], 0);
					motion_lights[sensor].timestamp = 0;
				}
			}
		}
		Chip_RTC_GetFullTime(LPC_RTC, &checkTime);
		//check if time to turn on interior lights
		for (uint8_t iLights = 0; iLights < no_of_turnon_times; iLights++)
		{

		}

		//check if time to strobe exterior lights
	}
	return numActiveSensors;
}

void showAllSensorStat(void)
{
	dispSensAll();

	do
	{
		setCursor(0, 2);

		for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
		{
			sendChar(getIOpin(&alarm_system_I[sensorid]) + 48);
		}
	} while (!getKP(200));

	while (getKP(100))
	{
		__NOP();
	}
}

void showAllXMSStat(void)
{
	dispAllXMSStat();

	do
	{
		setCursor(1, 1);

		for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
		{
			sendChar(getIOpin(&motion_lights[sensorid]) + 48);
			sendChar('-');
			sendChar(getIOpin(&automation_O[motion_lights[sensorid].device]) + 48);
			if (sensorid < 3)
			{
				sendChar('|');
				sendChar(' ');
			}
		}
	} while (!getKP(200));

	while (getKP(100))
	{
		__NOP();
	}
}

void checkPIN(void)
{
	if (onPressed)
	{
		displayPIN();
		if (getPIN() == 2)
			ALARMSTATE = DISARM;
		else
			dispClear();
	}
}
