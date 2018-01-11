#include "chip.h"
#include "sys_config.h"
#include "menus.h"

extern uint32_t systemTick;
extern uint8_t onPressed;
extern uint8_t dispDimmed;
extern uint32_t dimTimer;

I2C_XFER_T DISPLAYxfer;
RTC_TIME_T cTime;
//volatile uint32_t kpMatrixTimer;

uint8_t DISPLAY_txbuffer[DISPLAY_TXBUFFER_SZ]; //[row][column][data]

struct users_S users[] = {{0,"System",{KP_0,KP_0,KP_0,KP_0},0},
						  {1,"David",{KP_1,KP_3,KP_1,KP_8},4},
						  {2,"Christa",{KP_0,KP_3,KP_1,KP_8},1},
						  {3,"Aaron",{KP_2,KP_3,KP_1,KP_8},4},
						  {4,"Donny",{KP_3,KP_3,KP_1,KP_8},4}};
struct users_S *c_user;
//{{"WAKE", 0, 25, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW},
struct ALARM_SYSTEM_S alarm_system_I[] = {
		{"PWR_S", 1, 18, A_S_ACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"VIB_1", 1, 19, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"MTN_1", 1, 21, A_S_ACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_STAY, A_S_SIG_LEVEL_LOW, ENTRY_DELAY},
		{"SPAR2", 1, 24, A_S_ACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"DOR_M", 1, 25, A_S_ACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, ENTRY_DELAY},
		{"WDW_E", 1, 27, A_S_ACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"SPAR1", 1, 28, A_S_ACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"WDW_S", 1, 29, A_S_ACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"DOR_N", 1, 30, A_S_INACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"DOR_E", 1, 31, A_S_ACTIVE, A_S_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"SPAR3", 2, 8, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"SPAR4", 2, 11, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"MTN_2", 2, 12, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, ENTRY_DELAY},
		{"VIB_2", 4, 29, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		};
struct ALARM_SYSTEM_S automation_I[] = {
		{"MTN_S", 1, 20, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"MTN_N", 1, 22, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"MTN_E", 1, 23, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
		{"MTN_W", 1, 26, A_S_INACTIVE, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_LOW, NONE},
};
struct ALARM_SYSTEM_S automation_O[] = {
		{"INDCT", 0, 17, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_X_N", 0, 18, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_I_M", 0, 24, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_X_S", 2, 3, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_X_E", 2, 4, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_X_W", 2, 5, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"FAN", 2, 6, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		{"L_I_S", 3, 25, A_S_NOT_REQ_TO_ARM, A_S_ARM_ST_AWAY, A_S_SIG_LEVEL_HIGH, NONE},
		};

static void setUpGPIO(void);
static void setUpRTC(void);
static uint8_t initDisplay(void);
static void setUpUsers(void);

void setCursor(uint8_t row, uint8_t column)
{
	if (row) column += 0x40;
	sendCMD(column | 0b10000000);
}

void dispClear(void)
{
	sendCMD(0x01);
	pause(2);
}
void setUpSystem(void)
{
	SystemCoreClockUpdate();
	setUpGPIO();
	SysTick_Config(SystemCoreClock / 1000);
	initDisplay();
	dispBoot(0);
	pause(1000);
	setUpRTC();
	setUpUsers();
	//dispMainDARD(c_user->name);
}
void setUpGPIO(void)
{
	//TURN ON IO CONTROLS
	Chip_GPIO_Init(LPC_GPIO);
	//Chip_IOCON_Init(LPC_IOCON);

	//SET FUNCTION|MODE SYSTEM IO
	//Chip_IOCON_PinMuxSet(LPC_IOCON, TXD3_port,  TXD3_pin,   TXD3_mode | TXD3_func);
	//Chip_IOCON_PinMuxSet(LPC_IOCON, RXD3_port,  RXD3_pin,   RXD3_mode | RXD3_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0,  ERR2_p0_O,   IOCON_MODE_INACT | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0,  ERR1_p0_O,   IOCON_MODE_INACT | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, LIGHT_SENSE_port, LIGHT_SENSE_pin, LIGHT_SENSE_mode | LIGHT_SENSE_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, SIREN_DAC_port, SIREN_DAC_pin, SIREN_DAC_mode | SIREN_DAC_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, SDA0_port, SDA0_pin, SDA0_mode | SDA0_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, SCL0_port, SCL0_pin, SCL0_mode | SCL0_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, XB_TX1_port, XB_TX1_pin, XB_TX1_mode | XB_TX1_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, XB_RX1_port, XB_RX1_pin, XB_RX1_mode | XB_RX1_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, XB_CTS1_port, XB_CTS1_pin, XB_CTS1_mode | XB_CTS1_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, XB_RTS1_port, XB_RTS1_pin, XB_RTS1_mode | XB_RTS1_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, A2X, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, A3X, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, A4X, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, A5X, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, P2, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, KP_IN_port, ON_, IOCON_MODE_PULLDOWN | IOCON_FUNC0);
	Chip_IOCON_PinMuxSet(LPC_IOCON, EEPROM_SDA1_port, EEPROM_SDA1_pin, EEPROM_SDA1_mode | EEPROM_SDA1_func);
	Chip_IOCON_PinMuxSet(LPC_IOCON, EEPROM_SCL1_port, EEPROM_SCL1_pin, EEPROM_SCL1_mode | EEPROM_SCL1_func);

	//SET OUTPUT GPIO
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, DSP_RST_p0_O, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, ERR2_p0_O, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, ERR1_p0_O, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 3, IN_BUFF_OE_p3_O, true);
	IN_BUFF_OFF();
	Chip_GPIO_SetPinDIR(LPC_GPIO, 4, OUT_BUFF_OE_p4_O, true);
	OUT_BUFF_OFF();
	for (uint8_t o_p = 0; o_p < NUM_OF_AUTO_O; o_p++)
	{
		Chip_GPIO_SetPinDIR(LPC_GPIO, automation_O[o_p].port, automation_O[o_p].pin, true);
	}

	//SET KP OUTPUTS
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, K2, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, K3, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, K4, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, K5, true);
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, K6, true);
	//Setup interrupt for ON/C
	Chip_GPIOINT_Init(LPC_GPIOINT);
	Chip_GPIOINT_SetIntRising(LPC_GPIOINT, GPIOINT_PORT0, LPC_GPIOINT->IO0.ENR |= (1 << ON_));
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

	//SET DEFAULT OUTPUT VALUE
	Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, K5);
	//TODO: set to actual pad values
	setIOpin(&automation_O[L_I_M], 0);
	setIOpin(&automation_O[L_I_S], 0);
	setIOpin(&automation_O[FAN], 0);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);
}

// DISPLAY FUNCTIONS
//*

void setUpRTC(void)
{
	if (LPC_RTC->RTC_AUX & _BIT(RTC_OSCF))
	{
		LPC_RTC->RTC_AUX |= _BIT(RTC_OSCF);
		Chip_RTC_Enable(LPC_RTC, 1);
	}
	LPC_RTC->CCR = 17;  //the new lpcxpresso1769 vD had bit 3 set

	Chip_RTC_CntIncrIntConfig(LPC_RTC, 2, ENABLE);
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);
//	cTime.time[RTC_TIMETYPE_HOUR] = 21;
//	cTime.time[RTC_TIMETYPE_MINUTE] = 23;
//	cTime.time[RTC_TIMETYPE_SECOND] = 0;
//	cTime.time[RTC_TIMETYPE_MONTH] = 1;
//	cTime.time[RTC_TIMETYPE_DAYOFMONTH] = 9;
//	cTime.time[RTC_TIMETYPE_YEAR] = 2017;
//	Chip_RTC_SetFullTime(LPC_RTC, &cTime);
}

void pause(uint32_t ps)
{
	uint32_t ticks = systemTick + ps;
	while (systemTick < ticks) {}
}

void sendCMD(uint8_t cmdByte)
{
	DISPLAY_BUFF_RST();
	DISPLAY_txbuffer[0] = CMD;
    DISPLAY_txbuffer[1] = cmdByte;
    DISPLAYxfer.txSz = DISPLAY_PACKET_SZ;
    Chip_I2C_MasterTransfer(DISPLAY_DEV, &DISPLAYxfer);
}

void sendData(uint8_t dataByte)
{
	DISPLAY_BUFF_RST();
	DISPLAY_txbuffer[0] = DATA;
    DISPLAY_txbuffer[1] = dataByte;
    DISPLAYxfer.txSz = DISPLAY_PACKET_SZ;
    Chip_I2C_MasterTransfer(DISPLAY_DEV, &DISPLAYxfer);
}

void sendChar(uint8_t data)
{
	sendData(data);
}
void sendDisplay(uint8_t startAtCursor, struct MSG_S *msg)
{
	uint8_t row = 0;
	uint8_t column = 0;
	uint8_t DDRAM_address = 0;
	uint8_t *msg_tmp = msg->msg;

	if(!startAtCursor)
	{
		row = msg->row;
		column = msg->column;
		DDRAM_address = column;
		if (row) DDRAM_address += 0x40;
		column = 0;  //reuse variable to ensure array stays in bounds
		sendCMD(DDRAM_address | 0b10000000);
	}
	while (*msg_tmp != '\0' && column < 20)
	{
		sendData(*msg_tmp++);
		column++;
	}
}

uint8_t initDisplay(void)
{
	DISPLAYxfer.slaveAddr = DISPLAY_ADDRESS;
	DISPLAYxfer.slaveAddr &= 0xFF;
	DISPLAYxfer.txBuff = DISPLAY_txbuffer;
	DISPLAYxfer.rxSz = 0;

	Chip_IOCON_SetI2CPad(LPC_IOCON, I2CPADCFG_STD_MODE);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 27);
	Chip_IOCON_EnableOD(LPC_IOCON, 0, 28);
	Chip_I2C_Init(I2C0);
	Chip_I2C_SetClockRate(I2C0, 100000);
	Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandlerPolling);

	DISPLAY_RESET_INACTIVE();

	pause(PAUSE_AFTER_RESET);

	sendCMD(0x22 | 0x08);//0x2A);				//function set (extended command set)
	sendCMD(0x71);				//disable internal VDD regulator
	sendData(0x00);
	sendCMD(0x20 | 0x08);//0x28);				//function set (fundamental command set)
	sendCMD(0x08);				//display off, cursor off, blink off
	sendCMD(0x22 | 0x08);//0x2A);				//function set (extended command set)
	sendCMD(0x79);				//OLED command set enabled
	sendCMD(0xD5);				//set display clock divide ratio/oscillator frequency
	sendCMD(0x70);
	sendCMD(0x78);				//OLED command set disabled
	sendCMD(0x08);				//extended function set (2-lines) ?????
	sendCMD(0x06);				//COM SEG direction
	sendCMD(0x72);				//function selection B, ROM CGRAM selection
	sendData(0x0a);//0x00);
	//sendCMD(0x2A);				//function set (extended command set)
	sendCMD(0x79);				//OLED command set enabled
	sendCMD(0xDA);				//set SEG pins hardware configuration
	sendCMD(0x10);
	sendCMD(0xDC);				//function selection C
	sendCMD(0x00);
	sendCMD(0x81);				//set contrast control
	sendCMD(0x7F);
	sendCMD(0xD9);				//set phase length
	sendCMD(0xF1);
	sendCMD(0xDB);				//set VCOMH deselect level
	sendCMD(0x40);
	sendCMD(0x78);				//OLED command set disabled
	sendCMD(0x20 | 0x08);//0x28);				//function set (fundamental command set)
	sendCMD(0x01);				//clear display
	pause(2);
	sendCMD(0x80);				//set DDRAM address to 0x00
	sendCMD(0x0C);				//display ON
	pause(PAUSE_AFTER_RESET);
	dispClear();				//clear screen, home cursor

	return 1;
}

void displayON(void)
{
	sendCMD(0x0C);
}

void displayOFF(void)
{
	sendCMD(0x08);
	dispDimmed = 2;
	pause(2);
}

void displayDim(void)
{
	sendCMD(0x22 | 0x08);
	sendCMD(0x79);
	sendCMD(0x81);
	sendCMD(1);
	sendCMD(0x78);
	sendCMD(0x20 | 0x08);
	dispDimmed = 1;
	pause(2);
}

void displayNormal(void)
{
	sendCMD(0x22 | 0x08);
	sendCMD(0x79);
	sendCMD(0x81);
	sendCMD(0x7F);
	sendCMD(0x78);
	sendCMD(0x20 | 0x08);
	if (dispDimmed == 2) displayON();
	dispDimmed = 0;
	dimTimer = systemTick;
	pause(2);
}
//
// END DISPLAY FUNCTIONS

void setUpUsers(void)
{
	c_user = &users[0];
}

uint32_t getKP(uint32_t timeout)
{
	uint32_t timeoutTickState = 0;
	uint32_t scanRateTickState = 0;
	uint32_t portValue = 0;
	uint8_t currentOutput = K2;
	uint8_t kpState = 0;

	timeoutTickState = systemTick;

	while (systemTick < timeoutTickState + timeout)
	{
		switch (kpState)
		{
		case 0:
			scanRateTickState = systemTick;
			Chip_GPIO_SetPinOutHigh(LPC_GPIO, 0, currentOutput);
			kpState = 1;
			break;
		case 1:
			portValue = Chip_GPIO_GetPortValue(LPC_GPIO, 0);
			if (portValue & KP_MASK_IN) kpState = 2;
			else kpState = 3;
			break;
		case 2:
			Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, currentOutput);
			return (currentOutput | (portValue & KP_MASK_IN));
			break;
		case 3:
			if (systemTick > scanRateTickState + KP_SCAN_RATE_MS)
			{
				Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, currentOutput);
				currentOutput = (currentOutput == K6) ? K2 : currentOutput + 1;
				kpState = 0;
			}
			else kpState = 1;
			break;
		}
	}
	return 0;
}

uint8_t getDigit(uint32_t key)
{
	switch (key)
	{
	case KP_1:
		return 1;
		break;
	case KP_2:
		return 2;
		break;
	case KP_3:
		return 3;
		break;
	case KP_4:
		return 4;
		break;
	case KP_5:
		return 5;
		break;
	case KP_6:
		return 6;
		break;
	case KP_7:
		return 7;
		break;
	case KP_8:
		return 8;
		break;
	case KP_9:
		return 9;
		break;
	case KP_0:
		return 0;
		break;
	default:
		return 10;
		break;
	}
}

uint8_t getKPInput(uint32_t *p_values, uint8_t length)
{
	uint32_t selection = 0;

	for (uint8_t posCounter = 0; posCounter < length; posCounter++)
	{
		selection = getKP(KP_TIMEOUT_SUBMENU_MS);
		while(getKP(100)) {}
		if (!selection) return 0;
		if (selection == KP_CE)
		{
			*p_values = 255;
			return 255;
		}
		*p_values = getDigit(selection);
		sendChar(*p_values+48);
		p_values++;
	}
	return 1;
}

void setIOpin(struct ALARM_SYSTEM_S *sys, uint8_t level)
{
	if (level)
	{
		if (sys->sig_active_level) Chip_GPIO_SetPinState(LPC_GPIO, sys->port, sys->pin, 1);
		else Chip_GPIO_SetPinState(LPC_GPIO, sys->port, sys->pin, 0);
	}
	else
	{
		if (sys->sig_active_level) Chip_GPIO_SetPinState(LPC_GPIO, sys->port, sys->pin, 0);
		else Chip_GPIO_SetPinState(LPC_GPIO, sys->port, sys->pin, 0);
	}
}

uint8_t getIOpin(struct ALARM_SYSTEM_S *sys)
{
	uint8_t pinValue = Chip_GPIO_GetPinState(LPC_GPIO, sys->port, sys->pin);
	if (sys->sig_active_level) return pinValue;
	else return (pinValue ^ 1);
	return 255;
}
void EINT3_IRQHandler(void)
{
	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, (1 << ON_));
	onPressed = 1;
}
