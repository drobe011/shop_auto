#include <chip.h>
#include "sys_config.h"
#include "alarm_settings.h"
#include <time.h>
#include "eeprom.h"

#define EEPROM_SIG1 131
#define EEPROM_SIG2 201

I2C_XFER_T EEPROMxfer;
uint8_t eepromTXbuffer[20];
uint8_t eepromRXbuffer[20];

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD;
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S motion_lights[];
extern struct ALARM_SYSTEM_S alarm_system_O[];
extern struct LIGHT_AUTO_S light_auto[];
extern struct users_S users[];


EEPROM_STATUS initEEPROM(void)
{
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SDA1_port, EEPROM_SDA1_pin);
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SCL1_port, EEPROM_SCL1_pin);
	Chip_I2C_Init(EEPROM_DEV);
	Chip_I2C_SetClockRate(EEPROM_DEV, 100000);
	Chip_I2C_SetMasterEventHandler(EEPROM_DEV, Chip_I2C_EventHandlerPolling);

	EEPROMxfer.slaveAddr = EEPROM_ADDRESS;
	uint8_t *rbuffer = eepromRXbuffer;
	uint8_t *tbuffer = eepromTXbuffer;

	EEPROMxfer.rxBuff = rbuffer;;
	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxSz = 2;
	EEPROMxfer.txSz = 2;

	tbuffer[0] = 0;
	tbuffer[1] = 0;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((rbuffer[0] == EEPROM_SIG1)
				&& (rbuffer[1] == EEPROM_SIG2))
		{
			return getEEPROMdata();
		}
		else
			return setEEPROMdefaults();
	}
	else
		return BAD;
}

EEPROM_STATUS getEEPROMdata(void)
{
	uint8_t *rbuffer = eepromRXbuffer;
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = 0;
	uint8_t bytes[4];

	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxBuff = rbuffer;
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = 2;
	tbuffer[0] = 0;
	tbuffer[1] = ARM_DELAY_OFFSET;

	EPROM_DELAY();

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rbuffer[0];
		ENTRY_DELAY = rbuffer[1];
		DARK_THRESHOLD = rbuffer[2];
	}
	else
	{
		return BAD;
	}

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		rbuffer = eepromRXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + ASI_OFFSET;
		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxBuff = rbuffer;
		EEPROMxfer.rxSz = SENSOR_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		EPROM_DELAY();

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rbuffer[0] == sensorid)
		{
			alarm_system_I[sensorid].active = rbuffer[1];
			alarm_system_I[sensorid].req_to_arm = rbuffer[2];
			alarm_system_I[sensorid].armedstate = rbuffer[3];
			alarm_system_I[sensorid].sig_active_level = rbuffer[4];
			bytes[0] = rbuffer[5];
			bytes[1] = rbuffer[6];
			bytes[2] = rbuffer[7];
			bytes[3] = rbuffer[8];
			alarm_system_I[sensorid].delay = bytesToint(bytes);
		}
		else
		{
			return BAD;
		}
	}

	for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		rbuffer = eepromRXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + MS_OFFSET;
		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxBuff = rbuffer;
		EEPROMxfer.rxSz = X_MOTION_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		EPROM_DELAY();

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rbuffer[0] == sensorid)
		{
			motion_lights[sensorid].active = rbuffer[1];
			motion_lights[sensorid].sig_active_level = rbuffer[2];
			bytes[0] = rbuffer[3];
			bytes[1] = rbuffer[4];
			bytes[2] = rbuffer[5];
			bytes[3] = rbuffer[6];
			motion_lights[sensorid].delay = bytesToint(bytes);
		}
		else
		{
			return BAD;
		}
	}

	for (uint8_t sensorid = 0; sensorid < NUM_OF_AUTO_O; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		rbuffer = eepromRXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + OUTPUT_OFFSET;
		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxBuff = rbuffer;
		EEPROMxfer.rxSz = OUTPUT_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		EPROM_DELAY();

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rbuffer[0] == sensorid)
		{
			alarm_system_O[sensorid].active = rbuffer[1];
			alarm_system_O[sensorid].sig_active_level = rbuffer[2];
		}
		else
		{
			return BAD;
		}
	}

	for (uint8_t sensorid = 0; sensorid < NUM_OF_AUTO_LIS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		rbuffer = eepromRXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + AUTO_LIS_OFFSET;
		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxBuff = rbuffer;
		EEPROMxfer.rxSz = AUTO_LIS_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		EPROM_DELAY();

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		light_auto[sensorid].hour = rbuffer[0];
		light_auto[sensorid].min = rbuffer[1];
		light_auto[sensorid].duration = rbuffer[2];
		light_auto[sensorid].active = rbuffer[3];
	}

	return GOOD;
}

EEPROM_STATUS setEEPROMdefaults(void)
{
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = 0;
	uint8_t byteStorage[4];

	tbuffer[0] = 0;
	tbuffer[1] = 0;
	tbuffer[2] = EEPROM_SIG1;
	tbuffer[3] = EEPROM_SIG2;
	tbuffer[4] = ARM_DELAY;
	tbuffer[5] = ENTRY_DELAY;
	tbuffer[6] = DARK_THRESHOLD;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 7;
	EEPROMxfer.txBuff = tbuffer;

	EPROM_DELAY();

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		return BAD;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + ASI_OFFSET;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = SENSOR_PACKET_SIZE + 2;
		EEPROMxfer.txBuff = tbuffer;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		tbuffer[2] = sensorid;
		tbuffer[3] = alarm_system_I[sensorid].active;
		tbuffer[4] = alarm_system_I[sensorid].req_to_arm;
		tbuffer[5] = alarm_system_I[sensorid].armedstate;
		tbuffer[6] = alarm_system_I[sensorid].sig_active_level;
		intTobytes(byteStorage, alarm_system_I[sensorid].delay);
		tbuffer[7] = byteStorage[0];
		tbuffer[8] = byteStorage[1];
		tbuffer[9] = byteStorage[2];
		tbuffer[10] = byteStorage[3];

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}

	for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + MS_OFFSET;

		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = X_MOTION_PACKET_SIZE + 2;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		tbuffer[2] = sensorid;
		tbuffer[3] = motion_lights[sensorid].active;
		tbuffer[4] = motion_lights[sensorid].sig_active_level;
		intTobytes(byteStorage, motion_lights[sensorid].delay);
		tbuffer[5] = byteStorage[0];
		tbuffer[6] = byteStorage[1];
		tbuffer[7] = byteStorage[2];
		tbuffer[8] = byteStorage[3];

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}
	for (uint8_t sensorid = 0; sensorid < NUM_OF_AUTO_O; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + OUTPUT_OFFSET;

		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = OUTPUT_PACKET_SIZE + 2;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		tbuffer[2] = sensorid;
		tbuffer[3] = alarm_system_O[sensorid].active;
		tbuffer[4] = alarm_system_O[sensorid].sig_active_level;

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}
	for (uint8_t sensorid = 0; sensorid < NUM_OF_AUTO_LIS; sensorid++)
	{
		tbuffer = eepromTXbuffer;
		address = (EPROM_PAGE_SZ * sensorid) + AUTO_LIS_OFFSET;

		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = AUTO_LIS_PACKET_SIZE + 2;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		tbuffer[2] = light_auto[sensorid].hour;
		tbuffer[3] = light_auto[sensorid].min;
		tbuffer[4] = light_auto[sensorid].duration;
		tbuffer[5] = light_auto[sensorid].active;

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}

	for (uint8_t userid = 0; userid < MAX_USERS; userid++)
	{
		tbuffer = eepromTXbuffer;
		address = (EPROM_PAGE_SZ * userid) + USERDATA_OFFSET;

		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = USERDATA_PACKET_SIZE + 2;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;
		tbuffer[2] = users[userid].id;
		for (uint8_t name = 0; name < 8; name++)
		{
			tbuffer[name+3] = users[userid].name[name];
		}
		tbuffer[11] = (uint8_t)getDigit(users[userid].pin[0]);
		tbuffer[12] = (uint8_t)getDigit(users[userid].pin[1]);
		tbuffer[13] = (uint8_t)getDigit(users[userid].pin[2]);
		tbuffer[14] = (uint8_t)getDigit(users[userid].pin[3]);
		tbuffer[15] = users[userid].level;

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}
	return GOOD;
}

EEPROM_STATUS setEEPROMbyte(uint32_t offset, uint8_t ebyte)
{
	uint8_t *tbuffer = eepromTXbuffer;
	tbuffer[0] = (offset >> 8) & 0xFF;
	tbuffer[1] =  offset & 0xFF;;
	tbuffer[2] = ebyte;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 3;
	EEPROMxfer.txBuff = tbuffer;

	EPROM_DELAY();

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		return GOOD;
	}
	else
	{
		return BAD;
	}
}

EEPROM_STATUS setBootStamp(void)
{
	RTC_TIME_T bootTime;
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = BOOTTIME_OFFSET;

	Chip_RTC_GetFullTime(LPC_RTC, &bootTime);

	tbuffer[0] = (address >> 8) & 0xFF;
	tbuffer[1] = address & 0xFF;
	tbuffer[2] = bootTime.time[RTC_TIMETYPE_SECOND];
	tbuffer[3] = bootTime.time[RTC_TIMETYPE_MINUTE];
	tbuffer[4] = bootTime.time[RTC_TIMETYPE_HOUR];
	tbuffer[5] = bootTime.time[RTC_TIMETYPE_DAYOFMONTH];
	tbuffer[6] = bootTime.time[RTC_TIMETYPE_MONTH];
	tbuffer[7] = (bootTime.time[RTC_TIMETYPE_YEAR] >> 8) & 0xFF;
	tbuffer[8] = bootTime.time[RTC_TIMETYPE_YEAR] & 0xFF;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 9;
	EEPROMxfer.txBuff = tbuffer;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
	{
		return BAD;
	}
	return GOOD;
}

EEPROM_STATUS getBootStamp(RTC_TIME_T* boottime)
{
	uint32_t address = BOOTTIME_OFFSET;
	uint8_t *tbuffer = eepromTXbuffer;
	uint8_t *rbuffer = eepromRXbuffer;
	uint8_t year0 = 0;
	uint8_t year1 = 0;
	uint32_t newYear = 0;

	tbuffer[0] = (address >> 8) & 0xFF;
	tbuffer[1] = address & 0xFF;

	EEPROMxfer.rxSz = 7;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxBuff = rbuffer;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
	{
		return BAD;
	}

	boottime->time[RTC_TIMETYPE_MONTH] = rbuffer[4];
	boottime->time[RTC_TIMETYPE_DAYOFMONTH] = rbuffer[3];
	boottime->time[RTC_TIMETYPE_HOUR] = rbuffer[2];
	boottime->time[RTC_TIMETYPE_MINUTE] = rbuffer[1];
	boottime->time[RTC_TIMETYPE_SECOND] = rbuffer[0];
	year0 = rbuffer[5];
	year1 = rbuffer[6];
	newYear = ((uint32_t) year0 << 8) + (uint32_t) year1;
	boottime->time[RTC_TIMETYPE_YEAR] = newYear;

	return GOOD;
}

EEPROM_STATUS getUserData(uint8_t userid, struct users_S* userdata)
{
	if (userid == 0) return BAD;

	uint8_t *rbuffer = eepromRXbuffer;
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = 0;

	address = (EPROM_PAGE_SZ * userid) + USERDATA_OFFSET;

	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxBuff = rbuffer;
	EEPROMxfer.rxSz = USERDATA_PACKET_SIZE;
	EEPROMxfer.txSz = 2;

	tbuffer[0] = (address >> 8) & 0xFF;
	tbuffer[1] = address & 0xFF;

	EPROM_DELAY();

	Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);

	if (EEPROMxfer.rxSz > 0)
	{
		return BAD;
	}
	userdata->id = rbuffer[0];
	for (uint8_t name = 0; name < 8; name++)
	{
		userdata->name[name] = rbuffer[name+1];
	}
	userdata->pin[0] = rbuffer[9];
	userdata->pin[1] = rbuffer[10];
	userdata->pin[2] = rbuffer[11];
	userdata->pin[3] = rbuffer[12];
	userdata->level = rbuffer[13];
	return GOOD;
}

EEPROM_STATUS addUser(struct users_S *userdata)
{
	struct users_S tempUser;

	for (uint8_t userid = 1; userid < MAX_USERS; userid++)
	{
		getUserData(userid, &tempUser);
		if (tempUser.level == 0)
		{
			if (saveNewUser(userid, userdata)) return GOOD;
			else return BAD;
		}
	}
	return BAD;
}

EEPROM_STATUS saveNewUser(uint8_t userid, struct users_S *newuser)
{
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = BOOTTIME_OFFSET;

	address = (EPROM_PAGE_SZ * userid) + USERDATA_OFFSET;

	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = USERDATA_PACKET_SIZE + 2;

	tbuffer[0] = (address >> 8) & 0xFF;
	tbuffer[1] = address & 0xFF;
	tbuffer[2] = userid;
	for (uint8_t name = 0; name < 8; name++)
	{
		tbuffer[name+3] = newuser->name[name];
	}
	tbuffer[11] = (uint8_t)getDigit(newuser->pin[0]);
	tbuffer[12] = (uint8_t)getDigit(newuser->pin[1]);
	tbuffer[13] = (uint8_t)getDigit(newuser->pin[2]);
	tbuffer[14] = (uint8_t)getDigit(newuser->pin[3]);
	tbuffer[15] = newuser->level;

	EPROM_DELAY();

	Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
	if (EEPROMxfer.txSz > 0)
	{
		return BAD;
	}
	return GOOD;
}

EEPROM_STATUS changePIN(uint8_t userid, uint32_t *newpin)
{
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = BOOTTIME_OFFSET;

	tbuffer = eepromTXbuffer;
	address = (EPROM_PAGE_SZ * userid) + USERDATA_OFFSET + 9;

	EEPROMxfer.txBuff = tbuffer;
	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 4 + 2; //PIN + ADDRESS

	tbuffer[0] = (address >> 8) & 0xFF;
	tbuffer[1] = address & 0xFF;
	tbuffer[2] = (uint8_t)getDigit(newpin[0]);
	tbuffer[3] = (uint8_t)getDigit(newpin[1]);
	tbuffer[4] = (uint8_t)getDigit(newpin[2]);
	tbuffer[5] = (uint8_t)getDigit(newpin[3]);

	EPROM_DELAY();

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
	{
		return BAD;
	}
	return GOOD;
}

uint8_t getNumOfUsers(void)
{
	uint8_t *rbuffer = eepromRXbuffer;
	uint8_t *tbuffer = eepromTXbuffer;
	uint32_t address = 0;
	uint8_t numOfUsers = 0;

	for (uint8_t userid = 1; userid < MAX_USERS; userid++)
	{
		address = (EPROM_PAGE_SZ * userid) + USERDATA_OFFSET + 13;

		EEPROMxfer.txBuff = tbuffer;
		EEPROMxfer.rxBuff = rbuffer;
		EEPROMxfer.rxSz = 1;
		EEPROMxfer.txSz = 2;

		tbuffer[0] = (address >> 8) & 0xFF;
		tbuffer[1] = address & 0xFF;

		EPROM_DELAY();

		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);

		if (EEPROMxfer.rxSz > 0)
		{
			return 255;
		}
		if (rbuffer[0] > 0) numOfUsers++;
	}
	return numOfUsers;
}
