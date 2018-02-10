/*
 * menu.h
 *
 *  Created on: Jan 8, 2017
 *  Updated on: Dec 7, 2017
 *      Author: dave
 */

#ifndef MENUS_H_
#define MENUS_H_

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

#endif /* MENUS_H_ */
