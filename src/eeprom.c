#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

#define EEPROM_SIG1 163
#define EEPROM_SIG2 201

I2C_XFER_T EEPROMxfer;
uint8_t eepromTXbuffer[11];
uint8_t eepromRXbuffer[11];

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD;
extern struct ALARM_SYSTEM_S alarm_system_I[];
extern struct ALARM_SYSTEM_S motion_lights[];

EEPROM_STATUS initEEPROM(void)
{
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SDA1_port, EEPROM_SDA1_pin);
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SCL1_port, EEPROM_SCL1_pin);
	Chip_I2C_Init(EEPROM_DEV);
	Chip_I2C_SetClockRate(EEPROM_DEV, 40000);
	Chip_I2C_SetMasterEventHandler(EEPROM_DEV, Chip_I2C_EventHandlerPolling);

	EEPROMxfer.slaveAddr = EEPROM_ADDRESS;
	//EEPROMxfer.slaveAddr &= 0xFF;
	EEPROMxfer.rxSz = 2;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = eepromRXbuffer;
	EEPROMxfer.txBuff = eepromTXbuffer;
	uint8_t *eeAddress = eepromTXbuffer;
	eeAddress[0] = 0;
	eeAddress[1] = 0;
	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((eepromRXbuffer[0] == EEPROM_SIG1)
				&& (eepromRXbuffer[1] == EEPROM_SIG2))
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
	uint8_t bytes[4];
	uint8_t rcvbuff[3]; // = eepromTXbuffer;
	uint8_t tdata_ptr2[2];
	tdata_ptr2[0] = 0;
	tdata_ptr2[1] = ARM_DELAY_OFFSET;
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.txBuff = tdata_ptr2;

	EEPROMxfer.rxBuff = rcvbuff;

	pause(50);
	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rcvbuff[0];
		ENTRY_DELAY = rcvbuff[1];
		DARK_THRESHOLD = rcvbuff[2];
	}
	else
	{
		return BAD;
	}

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		uint8_t rcvbuff2[11]; // = eepromRXbuffer;
		uint8_t tdata_ptr3[2];
		//tdata_ptr = eepromTXbuffer;
		tdata_ptr3[0] = 0;
		tdata_ptr3[1] = SENSOR_OFFSET + (sensorid * SENSOR_PACKET_SIZE);
		EEPROMxfer.txBuff = tdata_ptr3;
		EEPROMxfer.rxBuff = rcvbuff2;
		EEPROMxfer.rxSz = SENSOR_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		pause(50);

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rcvbuff2[0] == sensorid)
		{
			alarm_system_I[sensorid].active = rcvbuff2[1];
			alarm_system_I[sensorid].req_to_arm = rcvbuff2[2];
			alarm_system_I[sensorid].armedstate = rcvbuff2[3];
			alarm_system_I[sensorid].sig_active_level = rcvbuff2[4];
			bytes[0] = rcvbuff2[5];
			bytes[1] = rcvbuff2[6];
			bytes[2] = rcvbuff2[7];
			bytes[3] = rcvbuff2[8];
			alarm_system_I[sensorid].delay = bytesToint(bytes);
		}
		else
		{
			//return BAD;
		}
	}
/*
	for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
	{
		rcvbuff = eepromRXbuffer;
		tdata_ptr = eepromTXbuffer;
		tdata_ptr[0] = 0;
		tdata_ptr[1] = X_MOTION_OFFSET + (sensorid * X_MOTION_PACKET_SIZE);
		EEPROMxfer.txBuff = tdata_ptr;
		EEPROMxfer.rxBuff = rcvbuff;
		EEPROMxfer.rxSz = X_MOTION_PACKET_SIZE;
		EEPROMxfer.txSz = 2;

		pause(50);

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		{
			return BAD;
		}

		if (rcvbuff[0] == sensorid)
		{
			motion_lights[sensorid].active = rcvbuff[1];
			motion_lights[sensorid].req_to_arm = rcvbuff[2];
			motion_lights[sensorid].armedstate = rcvbuff[3];
			motion_lights[sensorid].sig_active_level = rcvbuff[4];
			bytes[0] = rcvbuff[5];
			bytes[1] = rcvbuff[6];
			bytes[2] = rcvbuff[7];
			bytes[3] = rcvbuff[8];
			motion_lights[sensorid].delay = bytesToint(bytes);
		}
		else
		{
			return BAD;
		}
	}
	*/
	return GOOD;
}

EEPROM_STATUS setEEPROMdefaults(void)
{
	uint8_t tdata_ptr2[11];// = eepromTXbuffer;
	uint8_t byteStorage[4];

	tdata_ptr2[0] = 0;
	tdata_ptr2[1] = 0;
	tdata_ptr2[2] = EEPROM_SIG1;
	tdata_ptr2[3] = EEPROM_SIG2;
	tdata_ptr2[4] = ARM_DELAY;
	tdata_ptr2[5] = ENTRY_DELAY;
	tdata_ptr2[6] = DARK_THRESHOLD;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 7;
	EEPROMxfer.txBuff = tdata_ptr2;

	pause(50);
	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
		return BAD;

	for (uint8_t sensorid = 0; sensorid < NUM_OF_SYSTEMS; sensorid++)
	{
		uint8_t tdata_ptr[11];
		//tdata_ptr = eepromTXbuffer;
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
		intTobytes(byteStorage, alarm_system_I[sensorid].delay);
		tdata_ptr[7] = byteStorage[0];
		tdata_ptr[8] = byteStorage[1];
		tdata_ptr[9] = byteStorage[2];
		tdata_ptr[10] = byteStorage[3];

		pause(50);
		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}

/*
	for (uint8_t sensorid = 0; sensorid < X_MOTION_DETECTORS; sensorid++)
	{
		tdata_ptr = eepromTXbuffer;
		EEPROMxfer.txBuff = tdata_ptr;
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = X_MOTION_PACKET_SIZE + 2;

		tdata_ptr[0] = 0;
		tdata_ptr[1] = X_MOTION_OFFSET + (sensorid * X_MOTION_PACKET_SIZE);
		tdata_ptr[2] = sensorid;
		tdata_ptr[3] = motion_lights[sensorid].active;
		tdata_ptr[4] = motion_lights[sensorid].sig_active_level;
		intTobytes(byteStorage, motion_lights[sensorid].delay);
		tdata_ptr[5] = byteStorage[0];
		tdata_ptr[6] = byteStorage[1];
		tdata_ptr[7] = byteStorage[2];
		tdata_ptr[8] = byteStorage[3];

		pause(50);
		Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer);
		if (EEPROMxfer.txSz > 0)
		{
			return BAD;
		}
	}
*/
	return GOOD;
}

EEPROM_STATUS setEEPROMbyte(uint8_t offset, uint8_t ebyte)
{
	uint8_t *tdata_ptr = eepromTXbuffer;
	tdata_ptr[0] = 0;
	tdata_ptr[1] = offset;
	tdata_ptr[2] = ebyte;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 3;
	EEPROMxfer.txBuff = tdata_ptr;

	pause(50);

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
	uint8_t *tdata_ptr = eepromTXbuffer;

	Chip_RTC_GetFullTime(LPC_RTC, &bootTime);

	tdata_ptr[0] = 0;
	tdata_ptr[1] = BOOTTIME_OFFSET;
	tdata_ptr[2] = bootTime.time[RTC_TIMETYPE_SECOND];
	tdata_ptr[3] = bootTime.time[RTC_TIMETYPE_MINUTE];
	tdata_ptr[4] = bootTime.time[RTC_TIMETYPE_HOUR];
	tdata_ptr[5] = bootTime.time[RTC_TIMETYPE_DAYOFMONTH];
	tdata_ptr[6] = bootTime.time[RTC_TIMETYPE_MONTH];
	tdata_ptr[7] = (bootTime.time[RTC_TIMETYPE_YEAR] >> 8) & 0xFF;
	tdata_ptr[8] = bootTime.time[RTC_TIMETYPE_YEAR] & 0xFF;

	EEPROMxfer.rxSz = 0;
	EEPROMxfer.txSz = 9;
	EEPROMxfer.txBuff = tdata_ptr;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) != I2C_STATUS_DONE)
	{
		return BAD;
	}
	return GOOD;
}
