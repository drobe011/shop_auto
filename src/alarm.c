/*
 ===============================================================================
 Name        : alarm_12_2017.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

#include "chip.h"
#include <cr_section_macros.h>
#include "sys_config.h"
#include "alarm_settings.h"
#include "menus.h"
#include <stdio.h>
#include <string.h>

extern uint32_t systemTick;
extern uint8_t updateTimeFlag;
extern uint8_t onPressedFlag;
extern struct users_S *c_user;
extern struct users_S users[];
extern struct ALARM_SYSTEM_S alarm_system_O[];
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S motion_lights[];
extern struct LIGHT_AUTO_S light_auto[];
extern struct X_LIGHT_AUTO_S x_light_auto[];
extern uint32_t timer_timeOutFlag;
extern uint32_t timers_countDown;
extern uint32_t timer0_interval;

enum ALARMSTATE_T
{
	ARM, ARMING, ARMED, DISARM, DISARMED, ACTIVATE_ALARM, ALARM_ACTIVATED
};
enum ARMEDSTATE_T
{
	AWAY, NO_ALARM, AWAY_NO_EXT_AUTO, AWAY_NO_AUTO, STAY
};

uint8_t ARM_DELAY = ARM_DELAY_D;  //ee
uint8_t ENTRY_DELAY = ENTRY_DELAY_D;  //ee
uint8_t DARK_THRESHOLD = DARK_THRESHOLD_D; //ee

enum ALARMSTATE_T ALARMSTATE = DISARM;
enum ARMEDSTATE_T ARMEDSTATE = STAY;
uint8_t readyToArm;
uint8_t pinAttempts;
uint8_t dispDimmed;
uint32_t dimTimer;
uint8_t intLightsState;
uint8_t extLightsState;
uint8_t activeSensors[17];

STATIC INLINE void updateDisplayTime(void)
{
	dispDateTime();
	updateTimeFlag = 0;
}

STATIC INLINE void setCountDown(uint32_t secs)
{
	timer_timeOutFlag = ENABLE;
	timer0_interval = secs;
	Chip_TIMER_Reset(LPC_TIMER0);
	Chip_TIMER_Reset(LPC_TIMER1);
	LPC_TIMER0->PR = timers_countDown;
	Chip_TIMER_SetMatch(LPC_TIMER1, 0, secs * 1000);
	Chip_TIMER_Enable(LPC_TIMER1);
	Chip_TIMER_Enable(LPC_TIMER0);
}

STATIC INLINE void disableCountDown(void)
{
	Chip_TIMER_Disable(LPC_TIMER0);
	Chip_TIMER_Disable(LPC_TIMER1);
	Chip_TIMER_ClearMatch(LPC_TIMER1, 0);
	NVIC_ClearPendingIRQ(TIMER1_IRQn);

	timer_timeOutFlag = DISABLE;

	setIOpin(&alarm_system_O[BUZZR], DISABLE);
}

STATIC INLINE void flashARMLED()
{
	setIOpin(&alarm_system_O[ARM_I], ENABLE);
	pause(100);
	setIOpin(&alarm_system_O[ARM_I], DISABLE);
	updateTimeFlag = 0;
}

//Not really good debounce, however keypad has consistent behavior
STATIC INLINE void debouncer(void)
{
	while (getKP(100))
	{
		__NOP();
	}
}

uint8_t getPIN(void);
uint8_t pollAlarmSensors(void);
uint8_t pollAutomation(void);
uint8_t checkReadyToArm(void);
uint8_t entryDelay(uint8_t active);
void checkMenu(void);
void checkPIN(void);
void showUpTime(void);

void menu_Arm(void);
void menu_Lights_Int(void);
void menu_Lights_Ext(void);
void menu_changeTime(void);
void menu_inputStatus(void);
void subMenu_inputStatusAll(void);
void subMenu_edit_Inputs(uint8_t sensorid);
void menu_outputStatus(void);
void subMenu_outputStatusAll(void);
void subMenu_edit_Outputs(uint8_t sensorid);
void menu_edit_IObuffers(void);
void menu_edit_DarkTH(void);
void menu_edit_ArmDelay(void);
void armingDelay(void);
void menu_edit_EntryDelay(void);
void menu_ExtMotionSensorStatus(void);
void subMenu_ExtMotionSensorAll(void);
void subMenu_edit_ExtMotionSensor(uint8_t sensorid);
void subMenu_edit_AUTO_LIS(void);
void subMenu_edit_Auto_LIS_item(uint8_t item);

int main(void)
{
	setUpSystem();
	IN_BUFF_ON();
	OUT_BUFF_ON();

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
				ALARMSTATE = ARMED;
				break;
			case ARMING:
				armingDelay();
				ALARMSTATE = ARM;
				break;
			case ARMED:
				sensorsActive = pollAlarmSensors();
				pollAutomation();
				if (sensorsActive) entryDelay(sensorsActive);
				if (updateTimeFlag) flashARMLED();
				break;
			case DISARM:
				setIOpin(&alarm_system_O[SIREN], DISABLE);
				RESET_PIN_ATTEMPTS();
				DISABLE_ON_PWR();
				dispMainDARD(c_user->name);
				displayNormal();
				displayON();
				readyToArm = 2;  //SO IT DISPLAYS AFTER WRONG KEYPRESS
				ALARMSTATE = DISARMED;
				break;
			case DISARMED:
				if (updateTimeFlag) updateDisplayTime();
				if (!dispDimmed && CHECK_DIM_TIMER()) displayDim();
				if (dispDimmed == 1 && CHECK_DISPLAY_OFF_TIMER()) displayOFF();
				if (checkReadyToArm() != readyToArm) displayReadyToArm();
				checkMenu();
				pollAutomation();
				break;
			case ACTIVATE_ALARM:
				setIOpin(&alarm_system_O[SIREN], ENABLE);
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
		if (TIME_UP(on_pressed_time, ON_PRESSED_TIMEOUT))
		{
			ALARMSTATE = ACTIVATE_ALARM;
			return 0;
		}
	}
	onPressedFlag = 0;
	DISABLE_ON_PWR();
	for (uint8_t keys = 0; keys < 4; keys++)
	{
		kpData[keys] = getKP(KP_TIMEOUT_DEFAULT_MS + 1000);
		on_pressed_time = systemTick;
		while (getKP(150))
		{
			if (TIME_UP(on_pressed_time, KP_TIMEOUT_DEFAULT_MS))
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
	debouncer();

	if (selection)
	{
		if (dispDimmed) displayNormal();
		ALARMSTATE = DISARM;
		switch (selection)
		{
		case KP_Mplus:
			if (!readyToArm || !OE_INPUT_ON())
				return;
			menu_Arm();
			break;
		case KP_MRC:
			menu_Lights_Int();
			break;
		case KP_Mminus:
			menu_Lights_Ext();
			break;
		case KP_root:
			menu_changeTime();
			break;
		case KP_equal:
			menu_inputStatus();
			break;
		case KP_pcnt:
			menu_edit_IObuffers();
			break;
		case KP_div:
			menu_edit_DarkTH();
			break;
		case KP_plus:
			menu_edit_ArmDelay();
			break;
		case KP_minus:
			menu_edit_EntryDelay();
			break;
		case KP_dec:
			menu_ExtMotionSensorStatus();
			break;
		case KP_times:
			menu_outputStatus();
			break;
		case KP_0:
			showUpTime();
			break;
		case KP_1:
			subMenu_edit_AUTO_LIS();
			break;
		}
	}
}

void menu_Arm(void)
{
	uint32_t selection = 0;

	dispArmedType();
	selection = getKP(KP_TIMEOUT_SUBMENU_MS);
	debouncer();
	if (selection)
	{
		switch (selection)
		{
		case KP_Mplus:
			ARMEDSTATE = AWAY;
			break;
		case KP_root:
			ARMEDSTATE = NO_ALARM;
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

void menu_Lights_Int(void)
{
	uint32_t selection = 0;

	dispIntLight();
	uint32_t menuTimer = systemTick;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
		if (selection)
		{
			switch (selection)
			{
			case KP_1: //MAIN LIGHTS TOGGLE
				setIOpin(&alarm_system_O[L_I_M], (getIOpin(&alarm_system_I[LIM]) ^ 1));
				break;
			case KP_2: //SUPP LIGHTS TOGGLE
				setIOpin(&alarm_system_O[L_I_S], (getIOpin(&alarm_system_I[LIS]) ^ 1));
				break;
			case KP_3: //FAN TOGGLE
				setIOpin(&alarm_system_O[FAN], (getIOpin(&alarm_system_I[I_FAN]) ^ 1));
				break;
			}
			dispIntLight();
		}
	}
}

void menu_Lights_Ext(void)
{
	uint32_t selection = 0;

	dispExtLight();
	uint32_t menuTimer = systemTick;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();

		if (selection)
		{
			switch (selection)
			{
			case KP_1: //SOUTH LIGHT TOGGLE
				setIOpin(&alarm_system_O[L_X_S], (getIOpin(&alarm_system_O[L_X_S]) ^ 1));
				break;
			case KP_2: //NORTH LIGHT TOGGLE
				setIOpin(&alarm_system_O[L_X_N], (getIOpin(&alarm_system_O[L_X_N]) ^ 1));
			case KP_3: //EAST LIGHT TOGGLE
				setIOpin(&alarm_system_O[L_X_E], (getIOpin(&alarm_system_O[L_X_E]) ^ 1));
				break;
			case KP_4: //WEST LIGHT TOGGLE
				setIOpin(&alarm_system_O[L_X_W], (getIOpin(&alarm_system_O[L_X_W]) ^ 1));
				break;
			}
			dispExtLight();
		}
	}
}

void menu_changeTime(void)
{
	uint32_t selection[2] = { 0, 0 };
	uint32_t value = 0;
	RTC_TIME_T tempTime;

	dispTimeChange(HR);
	if (!getKPInput(selection, 2)) return;
	value = (selection[0] * 10) + selection[1];
	if (value > 23) return;
	tempTime.time[RTC_TIMETYPE_HOUR] = value;
	pause(500);

	dispTimeChange(MINIT);
	if (!getKPInput(selection, 2)) return;
	value = (selection[0] * 10) + selection[1];
	if (value > 59) return;
	tempTime.time[RTC_TIMETYPE_MINUTE] = value;
	pause(500);

	dispTimeChange(MOS);
	if (!getKPInput(selection, 2)) return;
	value = (selection[0] * 10) + selection[1];
	if (value > 12) return;
	tempTime.time[RTC_TIMETYPE_MONTH] = value;
	pause(500);

	dispTimeChange(DAY);
	if (!getKPInput(selection, 2)) return;
	value = (selection[0] * 10) + selection[1];
	if (value > 31) return;
	tempTime.time[RTC_TIMETYPE_DAYOFMONTH] = value;
	pause(500);

	dispTimeChange(YR);
	if (!getKPInput(selection, 2)) return;
	value = (selection[0] * 10) + selection[1];
	if (value > 50) return;
	tempTime.time[RTC_TIMETYPE_YEAR] = (2000 + value);
	pause(500);

	tempTime.time[RTC_TIMETYPE_SECOND] = 0;
	Chip_RTC_SetFullTime(LPC_RTC, &tempTime);
}

void menu_inputStatus(void)
{
	uint32_t selection[2] = { 0, 0 };

	uint32_t value = 0;

	dispInputStrings(0);
	setCursor(0, 9);
	if (!getKPInput(selection, 2))
	{
		subMenu_inputStatusAll();
		return;
	}
	if (selection[0] == 255) return;
	value = (selection[0] * 10) + selection[1];
	if (value > NUM_OF_SYSTEMS - 1) return;

	uint32_t menuTimer = systemTick;

	dispInput(value);

	selection[0] = 0;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
		if (selection[0]) break;
	}

	if (selection[0] == KP_equal)
	{
		subMenu_edit_Inputs(value);
	}

	menuTimer = systemTick;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		__NOP();
	}
}

void subMenu_edit_Inputs(uint8_t sensorid)
{
	uint32_t menuTimer = systemTick;
	uint32_t selection[3] = { 0, 0, 0 };
	uint32_t value;
	uint8_t byteStorage[4];

	dispEditInput(sensorid);

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
		switch (selection[0])
		{
		case KP_1:
			alarm_system_I[sensorid].active = (alarm_system_I[sensorid].active ? 0 : 1);
			saveByte((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 1, alarm_system_I[sensorid].active);
			break;
		case KP_2:
			alarm_system_I[sensorid].req_to_arm = (alarm_system_I[sensorid].req_to_arm ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 2), alarm_system_I[sensorid].req_to_arm);
			break;
		case KP_3:
			alarm_system_I[sensorid].armedstate = (alarm_system_I[sensorid].armedstate == 0 ? 4 : 0);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 3), alarm_system_I[sensorid].armedstate);
			break;
		case KP_4:
			alarm_system_I[sensorid].sig_active_level = (alarm_system_I[sensorid].sig_active_level ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 4), alarm_system_I[sensorid].sig_active_level);
			break;
		case KP_5:
			setCursor(0, 16);
			selection[0] = 0;
			if (!getKPInput(selection, 3)) break;
			if (selection[0] == 255) break;
			value = (selection[0] * 100) + (selection[1] * 10) + selection[2];
			if (value <= 999) alarm_system_I[sensorid].delay = value;
			else break;
			intTobytes(byteStorage, value);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 5), byteStorage[0]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 6), byteStorage[1]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 7), byteStorage[2]);
			saveByte(((EPROM_PAGE_SZ * sensorid) + ASI_OFFSET + 8), byteStorage[3]);
			break;
		}
		dispEditInput(sensorid);
	}
}

void subMenu_edit_Outputs(uint8_t sensorid)
{
	uint32_t menuTimer = systemTick;
	uint32_t selection[1] = { 0 };

	dispEditOutput(sensorid);

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
		switch (selection[0])
		{
		case KP_1:
			alarm_system_O[sensorid].active = (alarm_system_O[sensorid].active ? 0 : 1);
			saveByte((EPROM_PAGE_SZ * sensorid) + OUTPUT_OFFSET + 1, alarm_system_O[sensorid].active);
			break;
		case KP_2:
			alarm_system_O[sensorid].sig_active_level = (alarm_system_O[sensorid].sig_active_level ? 0 : 1);
			saveByte(((EPROM_PAGE_SZ * sensorid) + OUTPUT_OFFSET + 2), alarm_system_O[sensorid].sig_active_level);
			break;
		}
		dispEditOutput(sensorid);
	}
}

void menu_outputStatus(void)
{
	uint32_t selection[2] = { 0, 0 };

	uint32_t value = 0;

	dispOutputStrings(0);
	setCursor(0, 9);
	if (!getKPInput(selection, 2))
	{
		subMenu_outputStatusAll();
		return;
	}
	if (selection[0] == 255)
		return;
	value = (selection[0] * 10) + selection[1];
	if (value > NUM_OF_AUTO_O - 1)
		return;

	uint32_t menuTimer = systemTick;

	dispOutput(value);

	selection[0] = 0;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();

		if (selection[0])
			break;
	}

	if (selection[0] == KP_times)
	{
		subMenu_edit_Outputs(value);
	}

	menuTimer = systemTick;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		__NOP();
	}
}

void menu_ExtMotionSensorStatus(void)
{
	uint32_t selection[2] = { 0, 0 };

	uint32_t value = 0;

	dispInputStrings(0);
	setCursor(0, 9);
	if (!getKPInput(selection, 1))
	{
		subMenu_ExtMotionSensorAll();
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

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
		if (selection[0])
			break;
	}

	if (selection[0] == KP_dec)
	{
		subMenu_edit_ExtMotionSensor(value);
	}

	menuTimer = systemTick;

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		__NOP();
	}
}

void subMenu_edit_ExtMotionSensor(uint8_t sensorid)
{
	uint32_t menuTimer = systemTick;
	uint32_t selection[3] = { 0, 0, 0 };
	uint32_t value;
	uint8_t byteStorage[4];

	dispEditMotionSensor(sensorid);

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();
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
		dispEditMotionSensor(sensorid);
	}
}

void menu_edit_DarkTH(void)
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

void menu_edit_ArmDelay(void)
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

void menu_edit_EntryDelay(void)
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

void menu_edit_IObuffers(void)
{
	uint32_t selection[1];
	selection[0] = 0;
	dispBuffers();
	if (!getKPInput(selection, 1))
		return;
	if (selection[0] > 3)
		return;

	switch (selection[0])
	{
	case 0:
		IN_BUFF_OFF();
		break;
	case 1:
		IN_BUFF_ON();
		break;
	case 2:
		OUT_BUFF_OFF();
		break;
	case 3:
		OUT_BUFF_ON();
		break;
	}
}

uint8_t pollAlarmSensors(void)
{
	uint8_t sensor = 0;
	uint8_t numActiveSensors = 0;
	if (OE_INPUT_ON())
	{
		for (sensor = 0; sensor < NUM_OF_SYSTEMS; sensor++)
		{
			if (alarm_system_I[sensor].active && (alarm_system_I[sensor].armedstate != A_S_NO_ALARM))
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
	uint8_t error_led = DISABLE;

	if (!OE_INPUT_ON())
	{
		error_led = ENABLE;
	}

	if (!OE_OUTPUT_ON())
	{
		error_led = ENABLE;
	}

	for (uint8_t t_s = 0; t_s < trippedSensors; t_s++)
	{
		if (alarm_system_I[activeSensors[t_s]].req_to_arm)
		{
			error_led = ENABLE;
		}
	}

	if (error_led)
	{
		setIOpin(&alarm_system_O[ERROR_O], ENABLE);
		return 0;
	}
	else
	{
		setIOpin(&alarm_system_O[ERROR_O], DISABLE);
		return 1;
	}
}

void armingDelay(void)
{
	setCountDown(ARM_DELAY);
	displayArming();  //DIDN'T SEND TO PAUSE IN CASE WANTED IT TO DO SOMETHING
	while (timer_timeOutFlag)
	{
		__NOP();
	}
}

uint8_t entryDelay(uint8_t active)
{
	if (activeSensors[0] == DOOR_MAIN)
	{
		setCountDown(ENTRY_DELAY);
		ENABLE_ON_PWR();
		while (timer_timeOutFlag)
		{
			if (onPressedFlag)
			{
				displayPIN();
				if (getPIN() == 2)
				{
					ALARMSTATE = DISARM;
					disableCountDown();
					return 1;
				}
				else
				{
					dispClear();
					if (PIN_TRIES_EXCEEDED())
					{
						ALARMSTATE = ACTIVATE_ALARM;
						return 0;
					}
				}
			}
		}
	}
	ALARMSTATE = ACTIVATE_ALARM;
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
					setIOpin(&alarm_system_O[motion_lights[sensor].device], ENABLE);
					motion_lights[sensor].timestamp = systemTick;
				}
			}
			//check if time to turn off
			if (motion_lights[sensor].timestamp > 0)
			{
				if (systemTick - motion_lights[sensor].timestamp > (motion_lights[sensor].delay * (1000 * 60)))
				{
					setIOpin(&alarm_system_O[motion_lights[sensor].device], DISABLE);
					motion_lights[sensor].timestamp = 0;
				}
			}
		}

		Chip_RTC_GetFullTime(LPC_RTC, &checkTime);
		//check if time to turn on interior lights
		for (uint8_t iLights = 0; iLights < no_of_turnon_times; iLights++)
		{
			if (!light_auto[iLights].active)
			{
				if ((uint8_t)checkTime.time[RTC_TIMETYPE_HOUR] == light_auto[iLights].hour)
					if ((uint8_t)checkTime.time[RTC_TIMETYPE_MINUTE] == light_auto[iLights].min)
						if ((uint8_t)checkTime.time[RTC_TIMETYPE_SECOND] == 0)
						{
							light_auto[iLights].active = ENABLE;
							setIOpin(&alarm_system_O[L_I_S], ENABLE);
							alarm_system_O[L_I_S].timestamp = systemTick;
							//setIOpin(&automation_O[ARM_I], ENABLE); FOR TESTING
						}
			}
			else //check to turn off
			{
				if (TIME_UP(alarm_system_O[L_I_S].timestamp, (light_auto[iLights].duration * (1000 * 60))))
				{
					light_auto[iLights].active = DISABLE;
					setIOpin(&alarm_system_O[L_I_S], DISABLE);
					//setIOpin(&automation_O[ARM_I], DISABLE);
					alarm_system_O[L_I_S].timestamp = 0;
				}
			}
		}

		//check if time to strobe exterior lights
		for (uint8_t xstrobe = 0; xstrobe < no_of_x_flashes; xstrobe++)
		{
			if ((uint8_t)checkTime.time[RTC_TIMETYPE_HOUR] == x_light_auto[xstrobe].hour)
					if ((uint8_t)checkTime.time[RTC_TIMETYPE_MINUTE] == x_light_auto[xstrobe].min)
						if ((uint8_t)checkTime.time[RTC_TIMETYPE_SECOND] == 0)
						{
							setIOpin(&alarm_system_O[L_X_N], ENABLE);
							pause(250);
							setIOpin(&alarm_system_O[L_X_N], DISABLE);
							setIOpin(&alarm_system_O[L_X_E], ENABLE);
							pause(250);
							setIOpin(&alarm_system_O[L_X_E], DISABLE);
							setIOpin(&alarm_system_O[L_X_S], ENABLE);
							pause(250);
							setIOpin(&alarm_system_O[L_X_S], DISABLE);
							setIOpin(&alarm_system_O[L_X_W], ENABLE);
							pause(250);
							setIOpin(&alarm_system_O[L_X_W], DISABLE);
						}
		}
	}
	return numActiveSensors;
}

void subMenu_inputStatusAll(void)
{
	uint8_t sensorStatus[NUM_OF_SYSTEMS] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2};
	uint8_t sensorValue = 0;

	dispInputAll();

	do
	{
		for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
		{
			sensorValue = getIOpin(&alarm_system_I[sensorid]);
			if (sensorValue != sensorStatus[sensorid])
			{
				setCursor(0,sensorid+2);
				sensorStatus[sensorid] = sensorValue;
				sendChar(sensorValue + 48);
			}
		}
	} while (!getKP(200));

	debouncer();
}

void subMenu_outputStatusAll(void)
{
	//uint8_t sensorStatus[NUM_OF_AUTO_O] = {2,2,2,2,2,2,2,2,2,2,2};
	uint8_t sensorValue = 0;

	dispOutputAll();
	setCursor(0,4);

	for (uint8_t sensorid = 0; sensorid < NUM_OF_AUTO_O; sensorid++)
	{
		sensorValue = getIOpin(&alarm_system_O[sensorid]);
		sendChar(sensorValue + 48);
	}

	while (!getKP(200))
	{
		__NOP();
	}

	debouncer();
}

void subMenu_ExtMotionSensorAll(void)
{
	dispMotionSensorAll();

	do
	{
		setCursor(1, 1);

		for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
		{
			sendChar(getIOpin(&motion_lights[sensorid]) + 48);
			sendChar('-');
			sendChar(getIOpin(&alarm_system_O[motion_lights[sensorid].device]) + 48);
			if (sensorid < 3)
			{
				sendChar('|');
				sendChar(' ');
			}
		}
	} while (!getKP(200));

	debouncer();
}

void checkPIN(void)
{
	if (onPressedFlag)
	{
		displayPIN();
		if (getPIN() == 2) ALARMSTATE = DISARM;
		else
		{
			dispClear();
			displayOFF();
		}
	}
}

void showUpTime(void)
{
	dispUpTime();

	pause(KP_TIMEOUT_SUBMENU_MS);
}

void subMenu_edit_AUTO_LIS(void)
{
	uint8_t menuItem = 0;
	uint32_t selection = 0;
	uint32_t menuTimer = systemTick;

	dispAutomateLIS(menuItem);

	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection = getKP(KP_TIMEOUT_SUBMENU_MS);
		debouncer();

		switch (selection)
		{
		case KP_plus:
			if (menuItem < no_of_turnon_times - 1) menuItem++;
			menuTimer = systemTick;
			break;
		case KP_minus:
			if (menuItem) menuItem--;
			menuTimer = systemTick;
			break;
		case KP_equal:
			subMenu_edit_Auto_LIS_item(menuItem);
			dispAutomateLIS(menuItem);
			menuTimer = systemTick;
			break;
		}
		dispAutomateLIS(menuItem);
	}
}

void subMenu_edit_Auto_LIS_item(uint8_t item)
{
	uint32_t selection[2] = { 0, 0 };
	uint32_t menuTimer = systemTick;
	uint8_t value = 0;

	setCursor(1, 1);
	sendCMD(14);
	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		if (!getKPInput(selection, 2))
		{
			sendCMD(12);
			return;
		}
		value = (selection[0] * 10) + selection[1];
		if (value > 23)
		{
			sendCMD(12);
			return;
		}
		light_auto[item].hour = value;
	}

	menuTimer = systemTick;
	setCursor(1, 4);
	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		if (!getKPInput(selection, 2))
		{
			sendCMD(12);
			return;
		}
		value = (selection[0] * 10) + selection[1];
		if (value > 59)
		{
			sendCMD(12);
			return;
		}
		light_auto[item].min = value;
	}

	menuTimer = systemTick;
	setCursor(1, 9);
	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		if (!getKPInput(selection, 2))
		{
			sendCMD(12);
			return;
		}
		value = (selection[0] * 10) + selection[1];
		if (value > 99)
		{
			sendCMD(12);
			return;
		}
		light_auto[item].duration = value;
	}

	menuTimer = systemTick;
//	setCursor(1, 15);
	while (TIME_WAIT(menuTimer, KP_TIMEOUT_SUBMENU_MS))
	{
		selection[0] = getKP(KP_TIMEOUT_SUBMENU_MS);
		if (!selection[0])
		{
			sendCMD(12);
			return;
		}
		light_auto[item].active ^= 1;
		if (light_auto[item].active)
		{
			sendChar('O');
			sendChar('N');
			sendChar(' ');
		}
		else
		{
			sendChar('O');
			sendChar('F');
			sendChar('F');
		}
	}
	sendCMD(12);
}
