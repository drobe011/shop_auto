#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

const uint8_t EEPROM_SIG1 = 27;
const uint8_t EEPROM_SIG2 = 201;

I2C_XFER_T EEPROMxfer;

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD;
extern struct ALARM_SYSTEM_S alarm_system_I[];

EEPROM_STATUS initEEPROM(void)
{
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SDA1_port, EEPROM_SDA1_pin);
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SCL1_port, EEPROM_SCL1_pin);
	Chip_I2C_Init(EEPROM_DEV);
	Chip_I2C_SetClockRate(EEPROM_DEV, 100000);
	Chip_I2C_SetMasterEventHandler(EEPROM_DEV, Chip_I2C_EventHandlerPolling);

	EEPROMxfer.slaveAddr = EEPROM_ADDRESS;
	EEPROMxfer.slaveAddr &= 0xFF;
	uint8_t eeAddress[2] = { 0, 0 };
	uint8_t sigs[2];
	EEPROMxfer.rxSz = ARRAY_LEN(eeAddress);
	EEPROMxfer.txSz = ARRAY_LEN(sigs);
	EEPROMxfer.rxBuff = sigs;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((sigs[0] == 27) && (sigs[1] == 201))
			return getEEPROMdata();
		else
			return setEEPROMdefaults();
	}
	else
		return BAD;
}

EEPROM_STATUS getEEPROMdata(void)
{
	uint8_t eeAddress[2] = { 0, ARM_DELAY_OFFSET };
	uint8_t rcvdata[SENSOR_PACKET_SIZE];
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = ARRAY_LEN(eeAddress);
	EEPROMxfer.rxBuff = rcvdata;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rcvdata[0];
		ENTRY_DELAY = rcvdata[1];
		DARK_THRESHOLD = rcvdata[2];
	}
	else
		return BAD;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		eeAddress[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		EEPROMxfer.rxSz = SENSOR_PACKET_SIZE;
		EEPROMxfer.txSz = 2;
		EEPROMxfer.rxBuff = rcvdata;
		EEPROMxfer.txBuff = eeAddress;
		uint8_t bytes[4];
		uint32_t *lval; // = (unsigned long *)in;

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
			return BAD;

		if (rcvdata[0] == sensorid)
		{
			alarm_system_I[sensorid].active = rcvdata[1];
			alarm_system_I[sensorid].req_to_arm = rcvdata[2];
			alarm_system_I[sensorid].armedstate = rcvdata[3];
			alarm_system_I[sensorid].sig_active_level = rcvdata[4];
			bytes[0] = rcvdata[5];
			bytes[1] = rcvdata[6];
			bytes[2] = rcvdata[7];
			bytes[3] = rcvdata[8];
			lval = (uint32_t *) bytes;
			alarm_system_I[sensorid].delay = *lval;
		}
		else
			return BAD;
	}
	return GOOD;
}

EEPROM_STATUS setEEPROMdefaults(void)
{
	uint8_t globals[] = { 0, 0, EEPROM_SIG1, EEPROM_SIG2, ARM_DELAY,
			ENTRY_DELAY, DARK_THRESHOLD };
	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = ARRAY_LEN(globals);
	EEPROMxfer.txBuff = globals;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		return BAD;

	uint8_t sensordata[SENSOR_PACKET_SIZE + 2];

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		sensordata[0] = 0;
		sensordata[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		sensordata[2] = sensorid;
		sensordata[3] = alarm_system_I[sensorid].active;
		sensordata[4] = alarm_system_I[sensorid].req_to_arm;
		sensordata[5] = alarm_system_I[sensorid].armedstate;
		sensordata[6] = alarm_system_I[sensorid].sig_active_level;
		sensordata[7] = (alarm_system_I[sensorid].delay >> 24) & 0xFF;
		sensordata[8] = (alarm_system_I[sensorid].delay >> 16) & 0xFF;
		sensordata[9] = (alarm_system_I[sensorid].delay >> 8) & 0xFF;
		sensordata[10] = alarm_system_I[sensorid].delay & 0xFF;

		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = SENSOR_PACKET_SIZE + 2;
		EEPROMxfer.txBuff = sensordata;

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
			return BAD;
	}
	return GOOD;
}

EEPROM_STATUS setEEPROMbyte(uint8_t offset, uint8_t ebyte)
{
	uint8_t eeAddress[] = { 0, offset, ebyte };
	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = ARRAY_LEN(eeAddress);
	EEPROMxfer.txBuff = eeAddress;

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

	Chip_RTC_GetFullTime(LPC_RTC, &bootTime);

	uint8_t eeAddress[] = { 0, BOOTTIME_OFFSET,
			bootTime.time[RTC_TIMETYPE_SECOND],
			bootTime.time[RTC_TIMETYPE_MINUTE],
			bootTime.time[RTC_TIMETYPE_HOUR],
			bootTime.time[RTC_TIMETYPE_DAYOFMONTH],
			bootTime.time[RTC_TIMETYPE_MONTH],
			(bootTime.time[RTC_TIMETYPE_YEAR] >> 8) & 0xFF,
			bootTime.time[RTC_TIMETYPE_YEAR] & 0xFF };

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = ARRAY_LEN(eeAddress);
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
	{
		return BAD;
	}

	return GOOD;
}
