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

/*
 * SCREENS
 * 		BOOT SCREEN (DISP_BOOT)
 * 		********************
 * 		SnapperTron v1a
 * 		  ..initializing..
 *
 * 		DISARMED SCREEN (DISP_DARMD)
 * 		********************
 * 		Date          Time
 * 		Disarmed by
 */
extern struct users_S *c_user;
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern RTC_TIME_T cTime;
extern uint8_t readyToArm;
extern uint8_t DARK_THRESHOLD;

//uint8_t DISP_BOOT0[] = {0,0,'S','n','a','p','p','e','r','T','r','o','n',' ','v',VERSION_MAJOR,VERSION_MINOR,'\0'};
struct MSG_S DISP_BOOT0 = {0,0,"SnapperTron v" VERSION_MAJOR "." VERSION_MINOR}; //1.0"};
struct MSG_S DISP_BOOT1 = {1,2,"..initializing.."};
//uint8_t DISP_DARMD[] = {1,0,'D','i','s','a','r','m','e','d',' ','b','y',' ','\0'};
struct MSG_S DISP_SPACE = {0,0,"                    "};
struct MSG_S DISP_ARMED_TYPE1 = {0,0,"AW1,AW2,AW3,AW4,STY"};
struct MSG_S DISP_ARMED_TYPE2 = {1,0,"M+  SQR  /   -   ="};
struct MSG_S DISP_INT_LIGHT1 = {0,0,"PRI  ALT  ALL  FAN"};
struct MSG_S DISP_INT_LIGHT2 = {1,0,"7/4  8/5  1/0  %/*"};
struct MSG_S DISP_EXT_LIGHT1 = {0,1,"E   S   N   W  ALL"};
struct MSG_S DISP_EXT_LIGHT2 = {1,0,"7/4 8/5 9/6 $/* 1/0"};
struct MSG_S DISP_CHANGE_TIME = {0,0,"Time/Date"};
struct MSG_S DISP_HOUR = {1,0,"Hour: "};
struct MSG_S DISP_MINUTE = {1,0,"Minute: "};
struct MSG_S DISP_MONTH = {1,0,"Month: "};
struct MSG_S DISP_DAY = {1,0,"Day: "};
struct MSG_S DISP_YEAR = {1,0,"Year: "};
struct MSG_S DISP_STATUS = {0,0,"Sensor: "};
struct MSG_S DISP_STATUS2 = {1,0,"Active"};
struct MSG_S DISP_STATUS3 = {1,0,"Inactive"};
struct MSG_S DISP_STATUS_STAY = {0,0,"STAY"};
struct MSG_S DISP_STATUS_AWAY = {0,0,"AWAY"};
struct MSG_S INPUT_BUFF = {0,0,"INPUT 0:OFF/1:ON"};
struct MSG_S DISP_TEMP_CONST = {1,12,"000F/00%"};
struct MSG_S DISP_RDY_ARM = {1,9,{42,'\0'}};
struct MSG_S DISP_NOTRDY_ARM = {1,9,{219,'\0'}};
struct MSG_S DISP_ARMING = {0,0,{"ARMING...."}};
struct MSG_S DISP_PIN = {0,0,"ENTER PIN: "};
struct MSG_S DISP_DARK1 = {0,0,"[0-255]"};
struct MSG_S DISP_DARK2 = {1,0,"Dark TH [   ]:"};
struct MSG_S DISP_SENS_EDIT = {1,0,"[1] [2] [3] [4] [5]"};

void clearLine(uint8_t row)
{
	setCursor(row, 0);
	sendDisplay(1,&DISP_SPACE);
	setCursor(row, 0);
}
void dispDateTime(void)
{
	char timeStr[5];
	char dateStr[9];

	Chip_RTC_GetFullTime(LPC_RTC, &cTime);

	//setCursor(0, 0);

	sprintf(timeStr, "%02d:%02d",cTime.time[RTC_TIMETYPE_HOUR], cTime.time[RTC_TIMETYPE_MINUTE]);
	sprintf(dateStr, "%02d/%02d/%04d", cTime.time[RTC_TIMETYPE_MONTH], cTime.time[RTC_TIMETYPE_DAYOFMONTH], cTime.time[RTC_TIMETYPE_YEAR]);
	struct MSG_S time_tmp = {0,0, ""};
	strcpy((char*)time_tmp.msg, (char*)timeStr);
	struct MSG_S date_tmp = {0,10, ""};
	strcpy((char*)date_tmp.msg, (char*)dateStr);
	sendDisplay(0, &time_tmp);
	//setCursor(0, 10);
	sendDisplay(0, &date_tmp);
}

void dispBoot(uint8_t stage)
{
	if (!stage)
	{
		sendDisplay(0, &DISP_BOOT0);
		sendDisplay(0, &DISP_BOOT1);
	}
	else
	{

	}
}

void dispMainDARD(uint8_t* value)
{
	dispClear();
	dispDateTime();
	//sendDisplay(0, DISP_DARMD);
	//setCursor(1,0);
	struct MSG_S user = {1,0, ""};
	strcpy ((char*)user.msg, (char*)c_user->name);
	//user.msg = c_user->name;
	sendDisplay(0, &user);
	sendDisplay(0, &DISP_TEMP_CONST);
	//SET NOT READY TO ARM
	//setCursor(1,9);
	//sendDisplay(0, DISP_NOTRDY_ARM);
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

void dispStatus(uint8_t item)
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

	//pause(5000);
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
	sprintf(THStr, "%03d",DARK_THRESHOLD);
	struct MSG_S darkth_tmp = {1,9, ""};
	strcpy((char*)darkth_tmp.msg, (char*)THStr);
	sendDisplay(0, &darkth_tmp);

	struct MSG_S adcval_tmp = {0,17,""};
	sprintf(currentADCvalStr, "%03d", (char)lightVal);
	strcpy((char*)adcval_tmp.msg, (char*)currentADCvalStr);
	sendDisplay(0, &adcval_tmp);
	setCursor(1,15);
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
	sprintf(delay_tmp,"%03d", alarm_system_I[sensorid].delay);
	struct MSG_S delay_msg = { 0, 16, "" };
	strcpy((char*) delay_msg.msg, (char*) delay_tmp);
	sendDisplay(0, &delay_msg);
	sendDisplay(0, &DISP_SENS_EDIT);
}
