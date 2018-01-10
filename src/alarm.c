/*
 ===============================================================================
 Name        : alarm_12_2017.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
 ===============================================================================
 */

//TODO: SAVE ALARM STATE TO EEPROM IN EVENT OF RESET
//TODO: SAVE USER DATA EEPROM
//TODO: SAVE GLOBAL PARAMETERS TO EEPROM
#include "chip.h"
#include <cr_section_macros.h>
#include <sys_config.h>
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

enum ALARMSTATE_T
{
	ARM, ARMING, ARMED, DISARM, DISARMED, ACTIVATE_SIREN
};
enum ARMEDSTATE_T
{
	AWAY, AWAY_NO_INT_AUTO, AWAY_NO_EXT_AUTO, AWAY_NO_AUTO, STAY
};

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

uint8_t getPIN(void);
void checkMenu(void);
void checkAlarmSubMenu(void);
void checkIntLightSubMenu(void);
void checkExtLightSubMenu(void);
void changeTimeMenu(void);
void checkStatus(void);
void inputBuffers(void);
void armingDelay(void);
uint8_t pollAlarmSensors(void);
uint8_t checkReadyToArm(void);
uint8_t entryDelay(uint8_t active);

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
				//ENABLE_ERR_LED();
				break;
			case ARMING:
				armingDelay();
				ALARMSTATE = ARM;
				break;
			case ARMED:
				//checkSensors(ARMEDSTATE);
				//checkAutomation(ARMEDSTATE)
				sensorsActive = pollAlarmSensors();

				if (sensorsActive)
					if (!entryDelay(sensorsActive))
						ALARMSTATE = ACTIVATE_SIREN;
				break;
			case DISARM:
				RESET_PIN_ATTEMPTS();
				DISABLE_ON_PWR();
				DISABLE_ERR_LED();
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
				break;
			case ACTIVATE_SIREN:
				ENABLE_ERR_LED();
				pause(2000);
				DISABLE_ERR_LED();
				ALARMSTATE = DISARM;
				break;
			}
			RESET_STATE_TIMER();
		}
	}
}

uint8_t getPIN(void)
{
	uint32_t on_pressed_time = systemTick;
	uint32_t kpData[4] =
	{ 0, 0, 0, 0 };
	uint8_t match = 0;

	while (ON_PRESSED())
	{
		if (systemTick > (on_pressed_time + ON_PRESSED_TIMEOUT))
		{
			ALARMSTATE = ACTIVATE_SIREN;
			return 0;
		}
	}
	onPressed = 0;
	DISABLE_ON_PWR();
	for (uint8_t keys = 0; keys < 4; keys++)
	{

		kpData[keys] = getKP(KP_TIMEOUT_DEFAULT_MS);
		on_pressed_time = systemTick;
		while (getKP(200))
		{
			if (systemTick > (on_pressed_time + KP_TIMEOUT_DEFAULT_MS))
			{
				ENABLE_ON_PWR();
				pinAttempts++;
				return 0;
			}
		}
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
	} //TODO: check time holding button

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
	}
	if (selection)
	{
		switch (selection)
		{
		case KP_Mplus:
			//ALARMSTATE = ARM;
			ARMEDSTATE = AWAY;
			break;
		case KP_root:
			//ALARMSTATE = ARM;
			ARMEDSTATE = AWAY_NO_INT_AUTO;
			break;
		case KP_div:
			//ALARMSTATE = ARM;
			ARMEDSTATE = AWAY_NO_EXT_AUTO;
			break;
		case KP_minus:
			//ALARMSTATE = ARM;
			ARMEDSTATE = AWAY_NO_AUTO;
			break;
		case KP_equal:
			//ALARMSTATE = ARM;
			ARMEDSTATE = STAY;
			break;
		}
		ALARMSTATE = ARMING;
	}
}

void checkIntLightSubMenu(void)
{
	//0 = Main Lights
	//1 = Supp Lights
	//2 = All Lights
	//3 = Fan

	uint32_t selection = 0;
	uint32_t lightBit = 0;
	uint8_t lightState = 0;

	dispIntLight();
	selection = getKP(KP_TIMEOUT_SUBMENU_MS);
	while (getKP(100))
	{
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
	uint32_t selection[2] =
	{ 0, 0 };
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
	uint32_t selection[2] =
	{ 0, 0 };
	uint32_t value = 0;

	dispStatus(0);
	while (selection[0] != 255) //TODO: CHANGE TO TIMEOUT
	{
		setCursor(0, 9);
		if (!getKPInput(selection, 2))
			return;
		if (selection[0] == 255)
			return;
		value = (selection[0] * 10) + selection[1];
		if (value > 15)
			return;

		//TODO: MAKE INTO FUNCTION
		//setCursor(0,15);
		struct MSG_S sensor =
		{ 0, 15, "" };
		strcpy((char*) sensor.msg, (char*) alarm_system_I[value].name);
		sendDisplay(0, &sensor);
		clearLine(1);
		setCursor(1, 0);
		dispStatus((alarm_system_I[value].active ? 1 : 2));
		setCursor(1, 10);
		dispStatus((alarm_system_I[value].armedstate ? 3 : 4));
		setCursor(1, 19);
		sendChar(getIOpin(&alarm_system_I[value]) + 48);
	}

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
		ENABLE_ERR_LED();
		return 0;
	}
	else
		DISABLE_ERR_LED();
	/*
	 if (!SYSCK_GOOD())
	 {
	 ENABLE_ERR_LED();
	 return 0;
	 }
	 else
	 DISABLE_ERR_LED();*/

	for (uint8_t t_s = 0; t_s < trippedSensors; t_s++)
	{
		if (alarm_system_I[activeSensors[t_s]].req_to_arm)
		{
			ENABLE_ERR_LED();
			return 0;
		}
	}
	DISABLE_ERR_LED();
	return 1;
}

void armingDelay(void)
{
	uint32_t armingTimer = systemTick;

	displayArming();
	while (systemTick < armingTimer + ARM_DELAY)
	{
		//TODO:MAYBE BLINK SOMETHING WHILE ARM DELAY
	}
}

uint8_t entryDelay(uint8_t active)
{
	if (activeSensors[0] == DOOR_MAIN)
	{
		uint32_t entrytime = systemTick;
		ENABLE_ERR_LED();
		while (systemTick < entrytime + ENTRY_DELAY)
		{
			if (onPressed)
			{
				if (getPIN() == 2)
					return 1;
				if (PIN_TRIES_EXCEEDED())
					return 0;
			}
		}

	}
	return 0;
}
