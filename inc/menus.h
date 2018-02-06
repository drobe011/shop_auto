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
void dispSensor(uint8_t item);
void dispAuto_O(uint8_t item);
void dispSensorStatus(uint8_t item);
void dispAuto_O_Status(uint8_t item);
void dispSensorEdit(uint8_t sensorid);
void dispAuto_O_Edit(uint8_t sensorid);
void dispMotionSensor(uint8_t item);
void dispMotionSensorEdit(uint8_t sensorid);
void dispBuffers(void);
void dispDarkTH(void);
void dispArmDelay(void);
void dispEntryDelay(void);
void displayReadyToArm(void);
void displayArming(void);
void displayPIN(void);
void dispAllXMSStat(void);
void dispSensAll(void);
void dispAuto_O_All(void);

#endif /* MENUS_H_ */
