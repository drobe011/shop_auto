#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

#define EEPROM_SIG1 141
#define EEPROM_SIG2 201

I2C_XFER_T EEPROMxfer;
uint8_t eepromTXbuffer[11];
uint8_t eepromRXbuffer[11];

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD;
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S motion_lights[];
extern struct ALARM_SYSTEM_S automation_O[];

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
			automation_O[sensorid].active = rbuffer[1];
			automation_O[sensorid].sig_active_level = rbuffer[2];
		}
		else
		{
			return BAD;
		}
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
		tbuffer[3] = automation_O[sensorid].active;
		tbuffer[4] = automation_O[sensorid].sig_active_level;

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
