#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

const uint8_t EEPROM_SIG1 = 17;
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
		if ((sigs[0] == EEPROM_SIG1) && (sigs[1] == EEPROM_SIG1))
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
	uint8_t *addr_ptr = eeAddress;
	uint8_t *rdata_ptr = rcvdata;
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = ARRAY_LEN(eeAddress);
	EEPROMxfer.rxBuff = rcvdata;
	EEPROMxfer.txBuff = eeAddress;



	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rdata_ptr[0];
		ENTRY_DELAY = rdata_ptr[1];
		DARK_THRESHOLD = rdata_ptr[2];
	}
	else
		return BAD;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		addr_ptr = eeAddress;
		rdata_ptr = rcvdata;
		addr_ptr[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		EEPROMxfer.rxSz = SENSOR_PACKET_SIZE;
		EEPROMxfer.txSz = 2;
		EEPROMxfer.rxBuff = rdata_ptr;
		EEPROMxfer.txBuff = addr_ptr;
		uint8_t bytes[4];
		uint32_t *lval; // = (unsigned long *)in;

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
			return BAD;

		if (rdata_ptr[0] == sensorid)
		{
			alarm_system_I[sensorid].active = rdata_ptr[1];
			alarm_system_I[sensorid].req_to_arm = rdata_ptr[2];
			alarm_system_I[sensorid].armedstate = rdata_ptr[3];
			alarm_system_I[sensorid].sig_active_level = rdata_ptr[4];
			bytes[0] = rdata_ptr[5];
			bytes[1] = rdata_ptr[6];
			bytes[2] = rdata_ptr[7];
			bytes[3] = rdata_ptr[8];
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
	uint8_t *tdata_ptr = sensordata;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		tdata_ptr = sensordata;
		tdata_ptr[0] = 0;
		tdata_ptr[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		tdata_ptr[2] = sensorid;
		tdata_ptr[3] = alarm_system_I[sensorid].active;
		tdata_ptr[4] = alarm_system_I[sensorid].req_to_arm;
		tdata_ptr[5] = alarm_system_I[sensorid].armedstate;
		tdata_ptr[6] = alarm_system_I[sensorid].sig_active_level;
		tdata_ptr[7] = (alarm_system_I[sensorid].delay >> 24) & 0xFF;
		tdata_ptr[8] = (alarm_system_I[sensorid].delay >> 16) & 0xFF;
		tdata_ptr[9] = (alarm_system_I[sensorid].delay >> 8) & 0xFF;
		tdata_ptr[10] = alarm_system_I[sensorid].delay & 0xFF;

		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = SENSOR_PACKET_SIZE + 2;
		EEPROMxfer.txBuff = tdata_ptr;

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
