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
EEPROM_STATUS getBootStamp(RTC_TIME_T* boottime);
EEPROM_STATUS getUserData(uint8_t userid, struct users_S *userdata);
EEPROM_STATUS addUser(struct users_S *userdata);
EEPROM_STATUS saveNewUser(uint8_t userid, struct users_S *newuser);
EEPROM_STATUS changePIN(uint8_t userid, uint32_t *newpin);
EEPROM_STATUS changeName(uint8_t userid, uint8_t *newname);
uint8_t getNumOfUsers(void);

#endif /* EEPROM_H_ */
