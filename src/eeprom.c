#include <chip.h>
#include "sys_config.h"
#include "eeprom.h"

const uint8_t EEPROM_SIG1 = 27;
const uint8_t EEPROM_SIG2 = 201;

I2C_XFER_T EEPROMxfer;

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD;

EEPROM_STATUS initEEPROM(void)
{
	I2C_XFER_T EEPROMxfer;
	EEPROMxfer.slaveAddr = EEPROM_ADDRESS;
	EEPROMxfer.slaveAddr &= 0xFF;
	uint8_t eeAddress[2] = {0,0};
	uint8_t sigs[2];
	EEPROMxfer.rxSz = 2;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = sigs;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((sigs[0] == 27) && (sigs[1] == 201)) return GOOD;
		else return UNSET;
	}
	else return BAD;
}

EEPROM_STATUS getEEPROMdata(void)
{
	uint8_t eeAddress[2] = {0,2};
	uint8_t rcvdata[3];
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = rcvdata;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rcvdata[0];
		ENTRY_DELAY = rcvdata[1];
		DARK_THRESHOLD = rcvdata[2];

		return GOOD;
	}
	else return BAD;
}

EEPROM_STATUS setEEPROMdefaults(void)
{

	return GOOD;
}

EEPROM_STATUS setEEPROMbyte(uint8_t offset, uint8_t *ebyte)
{
		//uint8_t eeAddress[2] = {0,offset};
		//uint8_t rcvdata[3];
		EEPROMxfer.rxSz = 0;
		EEPROMxfer.txSz = 3;
		EEPROMxfer.txBuff = ebyte;

		if (Chip_I2C_MasterTransfer(EEPROM_DEV, &EEPROMxfer) == I2C_STATUS_DONE)
		{
			return GOOD;
		}
		else
		{
			return BAD;
		}
}
