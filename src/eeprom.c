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
	//Chip_IOCON_SetI2CPad(LPC_IOCON, I2CPADCFG_STD_MODE);
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SDA1_port, EEPROM_SDA1_pin);
	Chip_IOCON_EnableOD(LPC_IOCON, EEPROM_SCL1_port, EEPROM_SCL1_pin);
	Chip_I2C_Init(EEPROM_DEV);
	Chip_I2C_SetClockRate(EEPROM_DEV, 100000);
	Chip_I2C_SetMasterEventHandler(EEPROM_DEV, Chip_I2C_EventHandlerPolling);
	//I2C_XFER_T EEPROMxfer;
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
