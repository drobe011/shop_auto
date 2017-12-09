/*
 * constants.h
 *
 *  Created on: Jan 7, 2017
 *      Author: dave
 */

#ifndef SYS_CONFIG_H_
#define SYS_CONFIG_H_

#define VERSION_MAJOR "1"
#define VERSION_MINOR "a"

// ALARM SYSTEMS GLOBALS AND DEFINES
//
#define A_S_INACTIVE 0
#define A_S_ACTIVE 1
#define A_S_REQ_TO_ARM 1
#define A_S_NOT_REQ_TO_ARM 0
#define A_S_SIG_LEVEL_LOW 0
#define A_S_SIG_LEVEL_HIGH 1
#define A_S_ARM_ST_AWAY 0
#define A_S_ARM_ST_STAY 4

struct ALARM_SYSTEM_S {
	uint8_t name[6];
	uint8_t port;
	uint8_t pin;
	uint8_t active;
	uint8_t req_to_arm;
	uint8_t armedstate;
	uint8_t sig_active_level;
};

//
// END ALARM SYSTEMS GLOBALS AND DEFINES

// KEYPAD IO DEFINES
//
#define KP_GPIO_PORT 0
#define KP_IN_port 0
#define KP_OUT_port 0
#define K2 3
#define K3 4
#define K4 5
#define K5 6
#define K6 7
#define A2X 8
#define A3X 9
#define A4X 10
#define A5X 11
#define P2 15
#define ON_ 16
#define KP_SCAN_RATE_MS 15
#define KP_TIMEOUT_DEFAULT_MS 1000
#define KP_TIMEOUT_SUBMENU_MS 2500
#define KP_MASK_IN ((1 << A2X) | (1 << A3X) | (1 << A4X) | (1 << A5X) | (1 << P2) | (1 << ON_))

#define KP_MRC 516
#define KP_Mminus 2053
#define KP_Mplus 32773
#define KP_CE 2055
#define KP_7 1028
#define KP_8 1029
#define KP_9 1031
#define KP_4 517
#define KP_5 519
#define KP_6 518
#define KP_1 261
#define KP_2 263
#define KP_3 262
#define KP_0 260
#define KP_dec 515
#define KP_p_m 32771
#define KP_plus 2054
#define KP_pcnt 2052
#define KP_times 32774
#define KP_root 259
#define KP_div 32775
#define KP_minus 1030
#define KP_equal 32772
//
// END KEYPAD IO DEFINES

// SYSTEM I/O DEFINES
//
//#define TXD3_port 0
//#define TXD3_pin 0
//#define TXD3_func IOCON_FUNC2
//#define TXD3_mode IOCON_MODE_INACT
//#define RXD3_port 0
//#define RXD3_pin 1
//#define RXD3_func IOCON_FUNC2
//#define RXD3_mode IOCON_MODE_INACT
#define LIGHT_SENSE_port 0
#define LIGHT_SENSE_pin 2
#define LIGHT_SENSE_func IOCON_FUNC2
#define LIGHT_SENSE_mode IOCON_MODE_INACT
#define SIREN_DAC_port 0
#define SIREN_DAC_pin 26
#define SIREN_DAC_func IOCON_FUNC2
#define SIREN_DAC_mode IOCON_MODE_INACT
#define SDA0_port 0
#define SDA0_pin 27
#define SDA0_func IOCON_FUNC1
#define SDA0_mode IOCON_MODE_INACT
#define SCL0_port 0
#define SCL0_pin 28
#define SCL0_func IOCON_FUNC1
#define SCL0_mode IOCON_MODE_INACT
#define XB_TX1_port 2
#define XB_TX1_pin 0
#define XB_TX1_func IOCON_FUNC2
#define XB_TX1_mode IOCON_MODE_INACT
#define XB_RX1_port 2
#define XB_RX1_pin 1
#define XB_RX1_func IOCON_FUNC2
#define XB_RX1_mode IOCON_MODE_INACT
#define XB_CTS1_port 2
#define	XB_CTS1_pin 2
#define XB_CTS1_func IOCON_FUNC2
#define XB_CTS1_mode IOCON_MODE_INACT
#define XB_RTS1_port 2
#define XB_RTS1_pin 7
#define XB_RTS1_func IOCON_FUNC2
#define XB_RTS1_mode IOCON_MODE_INACT
#define EEPROM_SDA1_port 0
#define EEPROM_SDA1_pin 19
#define EEPROM_SDA1_func IOCON_FUNC3
#define EEPROM_SDA1_mode IOCON_MODE_INACT
#define EEPROM_SCL1_port 0
#define EEPROM_SCL1_pin 20
#define EEPROM_SCL1_func IOCON_FUNC3
#define EEPROM_SCL1_mode IOCON_MODE_INACT
//
// END SYSTEM I/O DEFINES

// GPIO DEFINES
//
////ALAARM INPUTS
#define SYSCK 0
#define VIB1 1
#define VIB2 2
#define MTN_EXT_S 3
#define MTN_INT 4
#define MTN_EXT_N 5
#define MTN_EXT_E 6
#define LT_MN 7
#define DOOR_ENTRY 8
#define MTN_EXT_W 9
#define WDW_E 10
#define WDW_S 11
#define DOOR_N 12
#define DOOR_E 13
#define LT_SP 14
#define FAN_I 15
#define NUM_OF_SYSTEMS 16

////OUTPUTS
#define INDCT 0
#define L_X_N 1
#define L_I_M 2
#define L_X_S 3
#define L_X_E 4
#define L_X_W 5
#define FAN 6
#define L_I_S 7
#define NUM_OF_AUTO_O 8


#define TACH1_p2_I 12
#define TACH2_p4_I 29

////TEMP IO
#define TMP_ALM_LO_p2_I 13
#define TMP_ALM_HI_p0_I 21
#define TMP_RDY_p0_I 22
#define VCC_p1_I 28

////DISPLAY IO
#define DSP_RST_p0_O 23


#define ERR1_p0_O 1
// optional, pin not broke out  #define ALARM_STATE_IND_p0_O 29

#define IN_BUFF_OE_p3_O 26
#define OUT_BUFF_OE_p4_O 28
#define IN_BUFF_ON() Chip_GPIO_SetPinOutLow(LPC_GPIO, 3, 26)
#define IN_BUFF_OFF() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 3, 26)
#define OUT_BUFF_ON() Chip_GPIO_SetPinOutLow(LPC_GPIO, 4, 28)
#define OUT_BUFF_OFF() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 4, 28)

//
// END GPIO DEFINES

// DISPLAY DEFINES AND GLOBALS
//
#define DISPLAY_DEV I2C0
#define DISPLAY_ADDRESS 0B0111100
#define DISPLAY_PACKET_SZ 2
#define DISPLAY_RESET_ACTIVE() Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, DSP_RST_p0_O)
#define DISPLAY_RESET_INACTIVE() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, DSP_RST_p0_O)
#define DISPLAY_BUFF_RST() DISPLAYxfer.txBuff = DISPLAY_txbuffer
#define PAUSE_AFTER_RESET 250
#define DISPLAY_TXBUFFER_SZ 23
#define CMD (1 << 7)
#define DATA (1 << 6)
//
// END DISPLAY DEFINES AND GLOBALS

// USER DATA
//
struct users_S
{
	uint8_t id;
	uint8_t name[8];
	uint32_t pin[4];
	uint8_t level;
};
//
// END USER DATA

struct MSG_S {
	uint8_t row;
	uint8_t column;
	uint8_t msg[21];
};

// RTC DEFINES
//
#define RTC_OSCF 4
#define HR 0
#define MINIT 1
#define MOS 2
#define DAY 3
#define YR 4
//
// END RTC DEFINES

// SERIAL OUT DEFINES
//
#define SERIAL_DEBUG LPC_UART3
#define SERIAL_BAUD 115200
//
// END SERIAL OUT DEFINES

// INT LIGHTS/FAN DEFINES
//
#define INT_MAIN_LT 1
#define INT_SUPP_LT 2
#define INT_FAN 3
//
// END INT LIGHTS/FAN DEFINES

// EXT LIGHTS DEFINES
//
#define EXT_EAST_LT 1
#define EXT_SOUTH_LT 2
#define EXT_NORTH_LT 3
#define EXT_WEST_LT 4
//
// END EXT LIGHTS DEFINES

// PROGRAM DEFINES
//
#define MAIN_STATE_LOOP_FX 25
#define DIM_OLED_TIME 10000
#define OFF_OLED_TIME (1000 * 20)
#define ARM_DELAY (1000 * 10)
#define CHECK_STATE_TIMER() systemTick > (stateTimer + MAIN_STATE_LOOP_FX)
#define CHECK_DIM_TIMER() systemTick > (dimTimer + DIM_OLED_TIME)
#define CHECK_OFF_TIMER() systemTick > (dimTimer + OFF_OLED_TIME)
#define RESET_STATE_TIMER() stateTimer = systemTick
#define RESET_DIM_TIMER() dimTimer = systemTick
#define RESET_PIN_ATTEMPTS() pinAttempts = 0
#define ENABLE_ON_PWR() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, K5)
#define DISABLE_ON_PWR() Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, K5)
#define ON_PRESSED_TIMEOUT 1000
#define ON_PRESSED() Chip_GPIO_GetPinState(LPC_GPIO, 0, ON_)
#define MAX_PIN_TRIES 3
#define ENABLE_ERR_LED() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, ERR1_p0_O)
#define DISABLE_ERR_LED() Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, ERR1_p0_O)
#define PIN_TRIES_EXCEEDED() pinAttempts > MAX_PIN_TRIES
#define OE_INPUT_ON() (Chip_GPIO_GetPinState(LPC_GPIO, 3, IN_BUFF_OE_p3_O) ^ 1)
#define SYSCK_GOOD() (Chip_GPIO_GetPinState(LPC_GPIO, 2, 12) ^ 1)
//
// END PROGRAM DEFINES

// FUNCTION DECLARATIONS
//

void setUpSystem(void);
void sendDisplay(uint8_t startAtCursor, struct MSG_S*);
void sendCMD(uint8_t cmdByte);
void sendData(uint8_t dataByte);
void sendChar(uint8_t data);
void pause(uint32_t ps);
void setCursor(uint8_t row, uint8_t column);
void dispClear(void);
uint32_t getKP(uint32_t timeout);
uint8_t getDigit(uint32_t key);
uint8_t getKPInput(uint32_t *p_values, uint8_t length);
void displayON(void);
void displayOFF(void);
void displayDim(void);
void displayNormal(void);
void setIOpin(struct ALARM_SYSTEM_S *sys, uint8_t level);
uint8_t getIOpin(struct ALARM_SYSTEM_S *sys);
//
// END FUNCTION DECLARATIONS


#endif /* SYS_CONFIG_H_ */
