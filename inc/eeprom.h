/*
 * eeprom.h
 *
 *  Created on: Jan 13, 2018
 *      Author: drob
 */

#ifndef EEPROM_H_
#define EEPROM_H_

typedef enum {BAD, GOOD, UNSET} EEPROM_STATUS;

EEPROM_STATUS initEEPROM(void);
EEPROM_STATUS setEEPROMdefaults(void);
EEPROM_STATUS getEEPROMdata(void);
EEPROM_STATUS setEEPROMbyte(uint32_t offset, uint8_t byte);
EEPROM_STATUS setBootStamp(void);

#endif /* EEPROM_H_ */
