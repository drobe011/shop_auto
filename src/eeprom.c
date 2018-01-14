#include <chip.h>
#include "eeprom.h"

const uint8_t EEPROM_SIG1 = 27;
const uint8_t EEPROM_SIG2 = 201;

extern uint8_t ARM_DELAY;
extern uint8_t ENTRY_DELAY;
extern uint8_t DARK_THRESHOLD_DEFAULT;

EEPROM_STATUS initEEPROM(void)
{
	I2C_XFER_T EEPROMxfer;
	EEPROMxfer.slaveAddr = 0b1010000;
	EEPROMxfer.slaveAddr &= 0xFF;
	uint8_t eeAddress[2] = {0,0};
	uint8_t sigs[2];
	EEPROMxfer.rxSz = 2;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = sigs;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(I2C1, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		if ((sigs[0] == 27) && (sigs[1] == 201)) return GOOD;
		else return UNSET;
	}
	else return BAD;
}

EEPROM_STATUS getEEPROMdata(void)
{
	I2C_XFER_T EEPROMxfer;
	EEPROMxfer.slaveAddr = 0b1010000;
	EEPROMxfer.slaveAddr &= 0xFF;
	uint8_t eeAddress[2] = {0,2};
	uint8_t rcvdata[3];
	EEPROMxfer.rxSz = 3;
	EEPROMxfer.txSz = 2;
	EEPROMxfer.rxBuff = rcvdata;
	EEPROMxfer.txBuff = eeAddress;

	if (Chip_I2C_MasterTransfer(I2C1, &EEPROMxfer) == I2C_STATUS_DONE)
	{
		ARM_DELAY = rcvdata[0];
		ENTRY_DELAY = rcvdata[1];
		DARK_THRESHOLD_DEFAULT = rcvdata[2];

		return GOOD;
	}
	else return BAD;
}

EEPROM_STATUS setEEPROMdefaults(void)
{


}
