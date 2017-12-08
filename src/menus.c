/*
 * menus.c
 *
 *  Created on: Jan 8, 2017
 *      Author: dave
 */
#include "chip.h"
#include "menus.h"
#include "sys_config.h"
#include "stdio.h"

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
extern RTC_TIME_T cTime;
extern uint8_t readyToArm;

//uint8_t DISP_BOOT0[] = {0,0,'S','n','a','p','p','e','r','T','r','o','n',' ','v',VERSION_MAJOR,VERSION_MINOR,'\0'};
struct MSG_S DISP_BOOT0 = {0,0,"SnapperTron v1.0"};
//uint8_t DISP_BOOT1[] = {1,2,'.','.','i','n','i','t','i','a','l','i','z','i','n','g','.','.','\0'};
struct MSG_S DISP_BOOT1 = {1,2,"..initializing.."};
//uint8_t DISP_DARMD[] = {1,0,'D','i','s','a','r','m','e','d',' ','b','y',' ','\0'};
//uint8_t DISP_SPACE[] = "                    ";
struct MSG_S DISP_SPACE = {0,0,"                    "};
//uint8_t DISP_ARMED_TYPE1[] = {0,0,'A','W','1',',','A','W','2',',','A','W','3',',','A','W','4',',','S','T','Y','\0'};
struct MSG_S DISP_ARMED_TYPE1 = {0,0,"AW1,AW2,AW3,AW4,STY"};
//uint8_t DISP_ARMED_TYPE2[] = {1,0,'M','+',' ',' ','S','Q','R',' ',' ','/',' ',' ',' ','-',' ',' ',' ','=','\0'};
struct MSG_S DISP_ARMED_TYPE2 = {1,0,"M+  SQR  /   -   ="};
//uint8_t DISP_INT_LIGHT1[] = {0,0,'P','R','I',' ',' ','A','L','T',' ',' ','A','L','L',' ',' ','F','A','N','\0'};
struct MSG_S DISP_INT_LIGHT1 = {0,0,"PRI  ALT  ALL  FAN"};
//uint8_t DISP_INT_LIGHT2[] = {1,0,'7','/','4',' ',' ','8','/','5',' ',' ','1','/','0',' ',' ','%','/','*','\0'};
struct MSG_S DISP_INT_LIGHT2 = {1,0,"7/4  8/5  1/0  %/*"};
//uint8_t DISP_EXT_LIGHT1[] = {0,1,'E',' ',' ',' ','S',' ',' ',' ','N',' ',' ',' ','W',' ',' ','A','L','L','\0'};
struct MSG_S DISP_EXT_LIGHT1 = {0,1,"E   S   N   W  ALL"};
//uint8_t DISP_EXT_LIGHT2[] = {1,0,'7','/','4',' ','8','/','5',' ','9','/','6',' ','%','/','*',' ','1','/','0','\0'};
struct MSG_S DISP_EXT_LIGHT2 = {1,0,"7/4 8/5 9/6 $/* 1/0"};
//uint8_t DISP_CHANGE_TIME[] = {0,0,'T','i','m','e','/','D','a','t','e','\0'};
struct MSG_S DISP_CHANGE_TIME = {0,0,"Time/Date"};
//uint8_t DISP_HOUR[] = {1,0,'H','o','u','r',':',' ','\0'};
struct MSG_S DISP_HOUR = {1,0,"Hour: "};
//uint8_t DISP_MINUTE[] = {1,0,'M','i','n','u','t','e','s',':',' ','\0'};
struct MSG_S DISP_MINUTE = {1,0,"Minute: "};
//uint8_t DISP_MONTH[] = {1,0,'M','o','n','t','h',':',' ','\0'};
struct MSG_S DISP_MONTH = {1,0,"Month: "};
//uint8_t DISP_DAY[] = {1,0,'D','a','y',':',' ','\0'};
struct MSG_S DISP_DAY = {1,0,"Day: "};
//uint8_t DISP_YEAR[] = {1,0,'Y','e','a','r',':',' ','\0'};
struct MSG_S DISP_YEAR = {1,0,"Year: "};
//uint8_t DISP_STATUS[] = {0,0,'S','e','n','s','o','r',':',' ','\0'};
struct MSG_S DISP_STATUS = {0,0,"Sensor: "};
//uint8_t DISP_STATUS2[] = {1,0,'A','c','t','i','v','e','\0'};
struct MSG_S DISP_STATUS2 = {1,0,"Active"};
//uint8_t DISP_STATUS3[] = {1,0,'I','n','a','c','t','i','v','e','\0'};
struct MSG_S DISP_STATUS3 = {1,0,"Inactive"};
//uint8_t DISP_STATUS_STAY[] = "STAY";
struct MSG_S DISP_STATUS_STAY = {0,0,"STAY"};
//uint8_t DISP_STATUS_AWAY[] = "AWAY";
struct MSG_S DISP_STATUS_AWAY = {0,0,"AWAY"};
//uint8_t INPUT_BUFF[] = {0,0,'I','N','P','U','T',' ','0',':','O','F','F','/','1',':','O','N','\0'};
struct MSG_S INPUT_BUFF = {0,0,"INPUT 0:OFF/1:ON"};
//uint8_t DISP_TEMP_CONST[] = {1,12,'0','0','0','F','/','0','0','%','\0'};
struct MSG_S DISP_TEMP_CONST = {1,12,"000F/00%"};
//uint8_t DISP_RDY_ARM[] = {1,9,42,'\0'};
struct MSG_S DISP_RDY_ARM = {1,9,{42,'\0'}};
//uint8_t DISP_NOTRDY_ARM[] = {1,9,219,'\0'};
struct MSG_S DISP_NOTRDY_ARM = {1,9,{219,'\0'}};

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
	*time_tmp.msg = *timeStr;
	struct MSG_S date_tmp = {0,10, ""};
	*date_tmp.msg = *dateStr;
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
	*user.msg = *c_user->name;
	sendDisplay(1, &user);
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
