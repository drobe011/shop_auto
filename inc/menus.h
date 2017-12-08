/*
 * menu.h
 *
 *  Created on: Jan 8, 2017
 *  Updated on: Dec 7, 2017
 *      Author: dave
 */

#ifndef MENUS_H_
#define MENUS_H_

#include <stdint.h>

//extern const uint8_t DISP_VERSION[];
//extern const uint8_t OOPS[];

//struct MSG_S {
//	uint8_t row;
//	uint8_t column;
//	uint8_t msg[];
//};

void clearLine(uint8_t row);
void dispBoot(uint8_t stage);
void dispMainDARD(uint8_t* value);
void dispDateTime(void);
void dispArmedType(void);
void dispIntLight(void);
void dispExtLight(void);
void dispTimeChange(uint8_t);
void dispStatus(uint8_t item);
void dispInputBuffers(void);
void displayReadyToArm(void);
#endif /* MENUS_H_ */
