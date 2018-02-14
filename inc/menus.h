/*
 * menu.h
 *
 *  Created on: Jan 8, 2017
 *  Updated on: Dec 7, 2017
 *      Author: dave
 */

#ifndef MENUS_H_
#define MENUS_H_

#define ARROW_DOWN 6
#define ARROW_UP 5
#define CURSOR_ON() sendCMD(14)
#define CURSOR_OFF() sendCMD(12)

void clearLine(uint8_t row);
void dispBoot(void);
void dispMainDARD(uint8_t* value);
void dispDateTime(void);
void dispArmedType(void);
void dispIntLight(void);
void dispExtLight(void);
void dispTimeChange(uint8_t);
void dispInput(uint8_t item);
void dispOutput(uint8_t item);
void dispInputStrings(uint8_t item);
void dispOutputStrings(uint8_t item);
void dispEditInput(uint8_t sensorid);
void dispEditOutput(uint8_t sensorid);
void dispMotionSensor(uint8_t item);
void dispEditMotionSensor(uint8_t sensorid);
void dispBuffers(void);
void dispDarkTH(void);
void dispArmDelay(void);
void dispEntryDelay(void);
void displayReadyToArm(void);
void displayArming(void);
void displayPIN(void);
void dispMotionSensorAll(void);
void dispInputAll(void);
void dispOutputAll(void);
void dispUpTime(void);
void dispAutomateLIS(uint8_t LIS_item);
void dispMainMenu(uint8_t position);
void dispInputsMenu(uint8_t position);
void dispOutputsMenu(uint8_t position);
void dispDelaysMenu(uint8_t position);
void dispSystemMenu(uint8_t position);
void dispAdminMenu(uint8_t position);
void dispResetDialog(void);
void dispNewPin(uint8_t round);
void dispNewUser(void);

#endif /* MENUS_H_ */
