/*
 * menus.c
 *
 *  Created on: Jan 8, 2017
 *      Author: dave
 */
#include "chip.h"
#include "menus.h"
#include "sys_config.h"
#include <stdio.h>
#include <string.h>

extern struct users_S *c_user;
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S automation_O[];
extern struct ALARM_SYSTEM_S motion_lights[];
extern RTC_TIME_T cTime;
extern uint8_t readyToArm;
extern uint8_t DARK_THRESHOLD;
extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;

struct MSG_S DISP_BOOT0 = { 0, 0,
		"SnapperTron v" VERSION_MAJOR "." VERSION_MINOR }; //1.0"};
struct MSG_S DISP_BOOT1 = { 1, 2, "..initializing.." };
struct MSG_S DISP_SPACE = { 0, 0, "                    " };
struct MSG_S DISP_ARMED_TYPE1 = { 0, 0, "AW1,AW2,AW3,AW4,STY" };
struct MSG_S DISP_ARMED_TYPE2 = { 1, 0, "M+  SQR  /   -   =" };
struct MSG_S DISP_INT_LIGHT1 = { 0, 0, "PRI  ALT  ALL  FAN" };
struct MSG_S DISP_INT_LIGHT2 = { 1, 0, "7/4  8/5  1/0  %/*" };
struct MSG_S DISP_EXT_LIGHT1 = { 0, 1, "E   S   N   W  ALL" };
struct MSG_S DISP_EXT_LIGHT2 = { 1, 0, "7/4 8/5 9/6 $/* 1/0" };
struct MSG_S DISP_CHANGE_TIME = { 0, 0, "Time/Date" };
struct MSG_S DISP_HOUR = { 1, 0, "Hour: " };
struct MSG_S DISP_MINUTE = { 1, 0, "Minute: " };
struct MSG_S DISP_MONTH = { 1, 0, "Month: " };
struct MSG_S DISP_DAY = { 1, 0, "Day: " };
struct MSG_S DISP_YEAR = { 1, 0, "Year: " };
struct MSG_S DISP_STATUS = { 0, 0, "Sensor: " };
struct MSG_S DISP_STATUS2 = { 1, 0, "Active" };
struct MSG_S DISP_STATUS3 = { 1, 0, "Inactive" };
struct MSG_S DISP_STATUS_STAY = { 0, 0, "STAY" };
struct MSG_S DISP_STATUS_AWAY = { 0, 0, "AWAY" };
struct MSG_S DISP_MOTION_DUR = { 0, 8, "DUR:" };
struct MSG_S INPUT_BUFF = { 0, 0, "INPUT 0:OFF/1:ON" };
struct MSG_S DISP_TEMP_CONST = { 1, 12, "000F/00%" };
struct MSG_S DISP_RDY_ARM = { 1, 9, { 42, '\0' } };
struct MSG_S DISP_NOTRDY_ARM = { 1, 9, { 219, '\0' } };
struct MSG_S DISP_ARMING = { 0, 0, { "ARMING...." } };
struct MSG_S DISP_PIN = { 0, 0, "ENTER PIN: " };
struct MSG_S DISP_DARK1 = { 0, 0, "[0-255]" };
struct MSG_S DISP_DARK2 = { 1, 0, "Dark TH [   ]:" };
struct MSG_S DISP_ARM_DELAY = { 1, 0, "Arm Delay [   ]:" };
struct MSG_S DISP_SENS_EDIT = { 1, 0, "[1] [2] [3] [4] [5]" };
struct MSG_S DISP_ENTRY_DELAY = { 1, 0, "Ent Delay [   ]:" };
struct MSG_S DISP_MOTN_EDIT = { 1, 0, "[1] [2] [3]" };

void clearLine(uint8_t row)
{
	setCursor(row, 0);
	sendDisplay(1, &DISP_SPACE);
	setCursor(row, 0);
}
void dispDateTime(void)
{
	char timeStr[5];
	char dateStr[9];

	Chip_RTC_GetFullTime(LPC_RTC, &cTime);

	sprintf(timeStr, "%02d:%02d", cTime.time[RTC_TIMETYPE_HOUR],
			cTime.time[RTC_TIMETYPE_MINUTE]);
	sprintf(dateStr, "%02d/%02d/%04d", cTime.time[RTC_TIMETYPE_MONTH],
			cTime.time[RTC_TIMETYPE_DAYOFMONTH], cTime.time[RTC_TIMETYPE_YEAR]);
	struct MSG_S time_tmp = { 0, 0, "" };
	strcpy((char*) time_tmp.msg, (char*) timeStr);
	struct MSG_S date_tmp = { 0, 10, "" };
	strcpy((char*) date_tmp.msg, (char*) dateStr);
	sendDisplay(0, &time_tmp);
	sendDisplay(0, &date_tmp);
}

void dispBoot(void)
{
	sendDisplay(0, &DISP_BOOT0);
	sendDisplay(0, &DISP_BOOT1);
}

void dispMainDARD(uint8_t* value)
{
	dispClear();
	dispDateTime();
	struct MSG_S user = { 1, 0, "" };
	strcpy((char*) user.msg, (char*) c_user->name);
	sendDisplay(0, &user);
	sendDisplay(0, &DISP_TEMP_CONST);
}

void dispArmedType(void)
{
	dispClear();
	sendDisplay(0, &DISP_ARMED_TYPE1);
	sendDisplay(0, &DISP_ARMED_TYPE2);
}

void dispIntLight(void)
{
	dispClear();
	sendDisplay(0, &DISP_INT_LIGHT1);
	sendDisplay(0, &DISP_INT_LIGHT2);
}

void dispExtLight(void)
{
	dispClear();
	sendDisplay(0, &DISP_EXT_LIGHT1);
	sendDisplay(0, &DISP_EXT_LIGHT2);
}

void dispTimeChange(uint8_t mode)
{
	switch (mode)
	{
	case HR:
		dispClear();
		sendDisplay(0, &DISP_CHANGE_TIME);
		sendDisplay(0, &DISP_HOUR);
		break;
	case MINIT:
		clearLine(1);
		sendDisplay(0, &DISP_MINUTE);
		break;
	case MOS:
		clearLine(1);
		sendDisplay(0, &DISP_MONTH);
		break;
	case DAY:
		clearLine(1);
		sendDisplay(0, &DISP_DAY);
		break;
	case YR:
		clearLine(1);
		sendDisplay(0, &DISP_YEAR);
		break;
	}
}

void dispSensor(uint8_t item)
{
	struct MSG_S sensor = { 0, 15, "" };
	strcpy((char*) sensor.msg, (char*) alarm_system_I[item].name);
	sendDisplay(0, &sensor);
	clearLine(1);
	setCursor(1, 0);
	dispSensorStatus((alarm_system_I[item].active ? 1 : 2));
	setCursor(1, 10);
	dispSensorStatus((alarm_system_I[item].armedstate ? 3 : 4));
	setCursor(1, 19);
	sendChar(getIOpin(&alarm_system_I[item]) + 48);
}

void dispMotionSensor(uint8_t item)
{
	char dur[4];

	struct MSG_S sensor = { 0, 0, "" };
	strcpy((char*) sensor.msg, (char*) motion_lights[item].name);
	dispClear();
	sendDisplay(0, &sensor);
	dispSensorStatus((motion_lights[item].active ? 1 : 2));
	setCursor(1, 19);
	sendChar(getIOpin(&automation_O[motion_lights[item].device]) + 48);
	sendDisplay(0, &DISP_MOTION_DUR);
	sprintf(dur, "%03d", motion_lights[item].delay);
	struct MSG_S dur_tmp = { 0, 13, "" };
	strcpy((char*) dur_tmp.msg, (char*) dur);
	sendDisplay(0, &dur_tmp);
}

void dispSensorStatus(uint8_t item)
{
	switch (item)
	{
	case 0:
		dispClear();
		sendDisplay(0, &DISP_STATUS);
		break;
	case 1:
		sendDisplay(0, &DISP_STATUS2);
		break;
	case 2:
		sendDisplay(0, &DISP_STATUS3);
		break;
	case 3:
		sendDisplay(1, &DISP_STATUS_STAY);
		break;
	case 4:
		sendDisplay(1, &DISP_STATUS_AWAY);
		break;
	}
}

void dispInputBuffers(void)
{
	dispClear();
	sendDisplay(0, &INPUT_BUFF);
}

void displayReadyToArm(void)
{
	if (readyToArm)
	{
		readyToArm = 0;
		sendDisplay(0, &DISP_NOTRDY_ARM);
	}
	else
	{
		readyToArm = 1;
		sendDisplay(0, &DISP_RDY_ARM);
	}
}

void displayArming(void)
{
	dispClear();
	sendDisplay(0, &DISP_ARMING);
}

void displayPIN(void)
{
	sendDisplay(0, &DISP_PIN);
}

void dispDarkTH(void)
{
	char THStr[4];
	char currentADCvalStr[3];
	uint8_t lightVal = isDark(0);

	dispClear();
	sendDisplay(0, &DISP_DARK1);
	sendDisplay(0, &DISP_DARK2);
	sprintf(THStr, "%03d", DARK_THRESHOLD);
	struct MSG_S darkth_tmp = { 1, 9, "" };
	strcpy((char*) darkth_tmp.msg, (char*) THStr);
	sendDisplay(0, &darkth_tmp);

	struct MSG_S adcval_tmp = { 0, 17, "" };
	sprintf(currentADCvalStr, "%03d", (char) lightVal);
	strcpy((char*) adcval_tmp.msg, (char*) currentADCvalStr);
	sendDisplay(0, &adcval_tmp);
	setCursor(1, 15);
}

void dispSensorEdit(uint8_t sensorid)
{
	dispClear();
	setCursor(0, 1);
	sendChar(alarm_system_I[sensorid].active ? 'Y' : 'N');
	setCursor(0, 5);
	sendChar(alarm_system_I[sensorid].req_to_arm ? 'Y' : 'N');
	setCursor(0, 9);
	sendChar((alarm_system_I[sensorid].armedstate == 0 ? 'A' : 'S'));
	setCursor(0, 13);
	sendChar(alarm_system_I[sensorid].sig_active_level ? 'H' : 'L');

	char delay_tmp[4];
	sprintf(delay_tmp, "%03d", alarm_system_I[sensorid].delay);
	struct MSG_S delay_msg = { 0, 16, "" };
	strcpy((char*) delay_msg.msg, (char*) delay_tmp);
	sendDisplay(0, &delay_msg);
	sendDisplay(0, &DISP_SENS_EDIT);
}

void dispMotionSensorEdit(uint8_t sensorid)
{
	dispClear();
	setCursor(0, 1);
	sendChar(motion_lights[sensorid].active ? 'Y' : 'N');
	setCursor(0, 5);
	sendChar(alarm_system_I[sensorid].sig_active_level ? 'H' : 'L');

	char delay_tmp[4];
	sprintf(delay_tmp, "%03d", motion_lights[sensorid].delay);
	struct MSG_S delay_msg = { 0, 8, "" };
	strcpy((char*) delay_msg.msg, (char*) delay_tmp);
	sendDisplay(0, &delay_msg);
	sendDisplay(0, &DISP_MOTN_EDIT);
}

void dispArmDelay(void)
{
	char ADStr[4];

	dispClear();
	sendDisplay(0, &DISP_DARK1);
	sendDisplay(0, &DISP_ARM_DELAY);
	sprintf(ADStr, "%03d", ARM_DELAY);
	struct MSG_S armdelay_tmp = { 1, 11, "" };
	strcpy((char*) armdelay_tmp.msg, (char*) ADStr);
	sendDisplay(0, &armdelay_tmp);
	setCursor(1, 17);
}

void dispEntryDelay(void)
{
	char EDStr[4];

	dispClear();
	sendDisplay(0, &DISP_DARK1);
	sendDisplay(0, &DISP_ENTRY_DELAY);
	sprintf(EDStr, "%03d", ENTRY_DELAY);
	struct MSG_S entrydelay_tmp = { 1, 11, "" };
	strcpy((char*) entrydelay_tmp.msg, (char*) EDStr);
	sendDisplay(0, &entrydelay_tmp);
	setCursor(1, 17);
}
