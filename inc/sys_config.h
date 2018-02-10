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
// SHORTCUTS FOR DEFINING STRUCT MEMBERS
// SHOULD MERGE AND USE ALREADY DEFINED ENUM

#define A_S_INACTIVE 0
#define A_S_ACTIVE 1
#define A_S_REQ_TO_ARM 1
#define A_S_NOT_REQ_TO_ARM 0
#define A_S_SIG_LEVEL_LOW 0
#define A_S_SIG_LEVEL_HIGH 1
#define A_S_ARM_ST_AWAY 0
#define A_S_NO_ALARM 1
#define A_S_ARM_ST_STAY 4
#define NONE 0

struct ALARM_SYSTEM_S
{
	uint8_t name[6];
	uint8_t port;
	uint8_t pin;
	uint8_t active;
	uint8_t req_to_arm;
	uint8_t armedstate;
	uint8_t sig_active_level;
	uint32_t delay;
	uint32_t timestamp;
	uint8_t device;
};

struct LIGHT_AUTO_S
{
	uint8_t hour;
	uint8_t min;
	uint8_t duration;
	uint8_t active;
};

struct X_LIGHT_AUTO_S
{
	uint8_t hour;
	uint8_t min;
	uint8_t active;
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
#define KP_TIMEOUT_DEFAULT_MS 2000
#define KP_TIMEOUT_SUBMENU_MS 4000
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
#define LIGHT_SENSE_port 0
#define LIGHT_SENSE_pin 2
#define LIGHT_SENSE_func IOCON_FUNC2
#define LIGHT_SENSE_mode IOCON_MODE_INACT
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

////ALARM INPUTS
#define PWR_SENSE 0
#define VIB1 1
#define MTN_INT1 2
#define LIM 3
#define DOOR_MAIN 4
#define WDW_E 5
#define LIS 6
#define WDW_S 7
#define DOOR_N 8
#define DOOR_E 9
#define I_FAN 10
#define SPAR4 11
#define MTN_INT2 12
#define VIB2 13
#define NUM_OF_SYSTEMS 14


////AUTOMATION INPUTS
#define MTN_EXT_S 0
#define MTN_EXT_N 1
#define MTN_EXT_E 2
#define MTN_EXT_W 3
#define X_MOTION_DETECTORS 4

////OUTPUTS
#define L_X_S 0
#define L_X_N 1
#define L_X_E 2
#define L_X_W 3
#define BUZZR 4
#define FAN 5
#define L_I_M 6
#define L_I_S 7
#define SIREN 8
#define ARM_I 9
#define ERROR_O 10
#define NUM_OF_AUTO_O 11

////TEMP IO
#define TMP_ALM_LO_p2_I 13
#define TMP_ALM_HI_p0_I 21
#define TMP_RDY_p0_I 22
#define VCC_p1_I 28

////DISPLAY IO
#define DSP_RST_p0_O 23

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
#define EEPROM_DEV I2C1
#define DISPLAY_ADDRESS 0B0111100
#define EEPROM_ADDRESS 0B1010000
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

struct MSG_S
{
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

// EEPROM DEFINES
//
#define EPROM_PAGE_SZ 32
#define EPROM_DELAY() pause(10)
#define ARM_DELAY_OFFSET 2
#define ENTRY_DELAY_OFFSET 3
#define DARK_THRESHOLD_OFFSET 4
#define ASI_OFFSET (1 * EPROM_PAGE_SZ)
#define MS_OFFSET (15 * EPROM_PAGE_SZ)
#define BOOTTIME_OFFSET (19 * EPROM_PAGE_SZ)
#define BOOTTIME_SIZE 7
#define SENSOR_PACKET_SIZE 9
#define X_MOTION_PACKET_SIZE 7
#define OUTPUT_OFFSET (BOOTTIME_OFFSET + BOOTTIME_SIZE)
#define OUTPUT_PACKET_SIZE 3
//
// END EEPROM DEFINES

//PROGRAM DEFINES
//
#define CHECK_STATE_TIMER() systemTick > (stateTimer + MAIN_STATE_LOOP_FX)
#define CHECK_DIM_TIMER() systemTick > (dimTimer + DIM_OLED_TIME)
#define CHECK_DISPLAY_OFF_TIMER() systemTick > (dimTimer + OFF_OLED_TIME)
#define RESET_STATE_TIMER() stateTimer = systemTick
#define RESET_DIM_TIMER() dimTimer = systemTick
#define RESET_PIN_ATTEMPTS() pinAttempts = 0
#define ENABLE_ON_PWR() Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, K5)
#define DISABLE_ON_PWR() Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, K5)
#define ON_PRESSED_TIMEOUT 1000
#define ON_PRESSED() Chip_GPIO_GetPinState(LPC_GPIO, 0, ON_)
#define PIN_TRIES_EXCEEDED() pinAttempts > MAX_PIN_TRIES
#define OE_INPUT_ON() (Chip_GPIO_GetPinState(LPC_GPIO, 3, IN_BUFF_OE_p3_O) ^ 1)
#define OE_OUTPUT_ON() (Chip_GPIO_GetPinState(LPC_GPIO, 4, OUT_BUFF_OE_p4_O) ^ 1)
#define TIME_UP(x,y) (systemTick - x) > y
#define TIME_WAIT(x,y) (systemTick - x) < y
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
uint8_t isDark(uint8_t mode);
void saveByte(uint32_t offset, uint8_t ebyte);
uint32_t bytesToint(uint8_t *bytes);
void intTobytes(uint8_t *bytes, uint32_t intVal);

//
// END FUNCTION DECLARATIONS

#endif /* SYS_CONFIG_H_ */
