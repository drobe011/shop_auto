#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

#define EEPROM_SIG1 187
#define EEPROM_SIG2 201

I2C_XFER_T EEPROMxfer;
uint8_t eepromTXbuffer[11];
uint8_t eepromRXbuffer[11];

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
	EEPROMxfer.rxSz = 2;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = eepromRXbuffer;
	EEPROMxfer.txBuff = eepromTXbuffer;
	uint8_t *eeAddress = eepromTXbuffer;//[2] = { 0, 0 };
	eeAddress[0] = 0;
	eeAddress[1] = 0;
	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((eepromRXbuffer[0] == EEPROM_SIG1) && (eepromRXbuffer[1] == EEPROM_SIG2))
		{
			ENABLE_ERR_LED();
			pause(1250);
			DISABLE_ERR_LED();
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
	uint8_t *tdata_ptr = eepromTXbuffer;
	tdata_ptr[0] = 0;
	tdata_ptr[1] = ARM_DELAY_OFFSET;
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.txBuff = tdata_ptr;
	uint8_t *rcvbuff = eepromRXbuffer;
	EEPROMxfer.rxBuff = rcvbuff;


	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = eepromRXbuffer[0];
		ENTRY_DELAY = eepromRXbuffer[1];
		DARK_THRESHOLD = eepromRXbuffer[2];
	}
	else
	{
		return BAD;
	}

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		rcvbuff = eepromRXbuffer;
		tdata_ptr = eepromTXbuffer;
		EEPROMxfer.txBuff = tdata_ptr;
		EEPROMxfer.rxBuff = rcvbuff;
		//addr_ptr = eeAddress;
		//rdata_ptr = rcvdata;
		tdata_ptr[0] = 0;
		tdata_ptr[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		EEPROMxfer.rxSz = SENSOR_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		uint8_t bytes[4];
		uint32_t *lval; // = (unsigned long *)in;

		pause(50);

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rcvbuff[0] == sensorid)
		{
			alarm_system_I[sensorid].active = rcvbuff[1];
			alarm_system_I[sensorid].req_to_arm = rcvbuff[2];
			alarm_system_I[sensorid].armedstate = rcvbuff[3];
			alarm_system_I[sensorid].sig_active_level = rcvbuff[4];
			bytes[0] = rcvbuff[5];
			bytes[1] = rcvbuff[6];
			bytes[2] = rcvbuff[7];
			bytes[3] = rcvbuff[8];
			lval = (uint32_t *) bytes;
			alarm_system_I[sensorid].delay = *lval;
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
	uint8_t *tdata_ptr = eepromTXbuffer;
	tdata_ptr[0] = 0;
	tdata_ptr[1] = 0;
	tdata_ptr[2] = EEPROM_SIG1;
	tdata_ptr[3] = EEPROM_SIG2;
	tdata_ptr[4] = ARM_DELAY;
	tdata_ptr[5] = ENTRY_DELAY;
	tdata_ptr[6] = DARK_THRESHOLD;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 7;
	EEPROMxfer.txBuff = tdata_ptr;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE) return BAD;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		tdata_ptr = eepromTXbuffer;
		EEPROMxfer.txBuff = tdata_ptr;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = SENSOR_PACKET_SIZE + 2;

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

		pause(50);
		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			ENABLE_ERR_LED();
			pause(1000);
			DISABLE_ERR_LED();
			pause(1000);
			ENABLE_ERR_LED();
			pause(1000);
			DISABLE_ERR_LED();
			return BAD;
			}
		//&sensordata[0] = holdptr;

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
