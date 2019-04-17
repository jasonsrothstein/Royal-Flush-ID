/*
 * display.c
 *
 *  Created on: Apr 17, 2019
 *      Author: cunnin66
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode (ENTRYMODE)
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control (DISP CONT)
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit



static uint8_t TARGET_ADDR = 0x27;
static uint8_t _DISP_FUNC = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
static uint8_t _DISP_CONT = LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON;
static uint8_t _DISP_MODE = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
static uint8_t _BACKLIGHT = LCD_BACKLIGHT;

static void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

//===========================================================================
// Initialize I2C1 to 100 kHz
void i2c_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER |= 2<<(2*6) | 2<<(2*7);
    GPIOB->AFR[0] |= 1<<(4*6) | 1<<(4*7); //set to af1

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    //RCC->CFGR3 |= RCC_CFGR3_I2C1SW;  // set for 48MHz sysclk

    // I2C CR1 Config
    I2C1->CR1 &= ~I2C_CR1_PE;           // Disable to perform reset.
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;       // 0: Analog noise filter enabled.
    I2C1->CR1 &= ~I2C_CR1_ERRIE;        // Errors interrupt disable
   // I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;    // Enable clock stretching

    // From table 83. p642 of FRM.  Set for 100 kHz with 8MHz clock.
    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;// Clear prescaler
    I2C1->TIMINGR |= 1 << 28;           // Set prescaler to 0
    I2C1->TIMINGR |= 4 << 20;           // SCLDEL
    I2C1->TIMINGR |= 2 << 16;           // SDADEL
    I2C1->TIMINGR |= 0xF << 8;            // SCLH
    I2C1->TIMINGR |= 0x13 << 0;            // SCLL

    // I2C Own address 1 register (I2C_OAR1)
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 =   I2C_OAR1_OA1EN | (0x1<<1);// Set 7-bit own address 1

    I2C1->CR2 &= ~I2C_CR2_ADD10;        // 0 = 7-bit mode; 1 = 10-bit mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;       // Enable the auto end
    I2C1->CR2 |= I2C_CR2_NACK;          // For slave mode: set NACK

    I2C1->CR1 |= I2C_CR1_PE;            // Enable I2C1
}


//===========================================================================
// Clear NACK condition.
int i2c_checknack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        return 1;
    return 0;
}

//===========================================================================
// Clear NACK condition.
void i2c_clearnack(void) {
    if (I2C1->ISR & I2C_ISR_NACKF)
        I2C1->ICR |= I2C_ICR_NACKCF;
}

//===========================================================================
// Clear STOP condition.
int i2c_checkstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return 1;
    return 0;
}

//===========================================================================
// Clear STOP condition.
void i2c_clearstop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        I2C1->ICR |= I2C_ICR_STOPCF;
}

//===========================================================================
// Look at the ISR and display the error.
/*void display_i2c_error(void) {
    if (I2C1->ISR & I2C_ISR_BERR)
        display1("Bus error");
    else if (I2C1->ISR & I2C_ISR_ARLO)
        display1("Arbitration Lost");
    else if (I2C1->ISR & I2C_ISR_TIMEOUT)
        display1("I2C TIMEOUT");
    else if (I2C1->ISR & I2C_ISR_BUSY)
        display1("I2C BUSY");
    else if (I2C1->ISR & I2C_ISR_PECERR)
        display1("I2C PEC Error");
    else if (I2C1->ISR & I2C_ISR_STOPF)
        display1("I2C Stopped");
    else if (I2C1->ISR & I2C_ISR_NACKF)
        display1("I2C NACK");
    else
        display1("???");
}
*/
//===========================================================================
// Generate a start bit.
void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir) {
    // dir: 0 = master requests a write transfer
    // dir: 1 = master requests a read transfer
    uint32_t tmpreg = I2C1->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES |
                I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);
    if (dir == 1)
        tmpreg |= I2C_CR2_RD_WRN;  // Read from slave
    else
        tmpreg &= I2C_CR2_RD_WRN;  // Write to slave
    tmpreg |= ((devaddr<<1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;
    I2C1->CR2 = tmpreg;
}

//===========================================================================
// Generate a stop bit.
void i2c_stop(void) {
    if (I2C1->ISR & I2C_ISR_STOPF)
        return;
    // Master: Generate STOP bit after current byte has been transferred.
    I2C1->CR2 |= I2C_CR2_STOP;
    // Wait until STOPF flag is reset
    while( (I2C1->ISR & I2C_ISR_STOPF) == 0);
    I2C1->ICR |= I2C_ICR_STOPCF; // Write  to clear STOPF flag
}

//===========================================================================
// Check wait for the bus to be idle.
void i2c_waitidle(void) {
    while ( (I2C1->ISR ^ I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

//===========================================================================
// Send multiple bytes to a target slave.
// - Wait for idle.
// - Send data, byte by byte.
// - After all bytes sent, hardware sets the TC flag.
//   Software waits until TC is set.
//   Hardware automatically clears TC flag when START bit or STOP bit in
//   CR2 is set.
int8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size) {
    int i;
    if (size <= 0 || pdata == 0) return -1;
    uint8_t *udata = (uint8_t*)pdata;
    i2c_waitidle();
    // Last argument is dir: 0 = sending data to the slave.
    i2c_start(devaddr, size, 0);

    for(i=0; i<size; i++) {
        // TXIS bit is set by hardware when the TXDR register is empty and the
        // data to be transmitted must be written in the TXDR register.  It is
        // cleared when the next data to be sent is written in the TXDR reg.
        // The TXIS flag is not set when a NACK is received.
        int count = 0;
        while( (I2C1->ISR & I2C_ISR_TXIS) == 0) {
            count += 1;
            if (count > 1000000)
                return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                i2c_stop();
                return -1;
            }
        }

        // TXIS is cleared by writing to the TXDR register.
        I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
    }

    // Wait until TC flag is set or the NACK flag is set.
    while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

    if ( (I2C1->ISR & I2C_ISR_NACKF) != 0)
        return -1;
    i2c_stop();
    return 0;
}


void send(char c, char mode) {
	//send high 4 bits
	uint8_t upper4 = (c&0xF0)| _BACKLIGHT | mode;
	i2c_senddata(TARGET_ADDR, &upper4, 1);
	nano_wait(1000*1000);
	//enable pulse?
	upper4 |= En;
	i2c_senddata(TARGET_ADDR, &upper4, 1);
	nano_wait(1*1000*1000);
	upper4 &= ~En;
	i2c_senddata(TARGET_ADDR, &upper4, 1);
	nano_wait(1*1000*1000);
	//send low 4 bits
	uint8_t lower4 = ((c&0x0F) << 4)| _BACKLIGHT | mode;
	i2c_senddata(TARGET_ADDR, &lower4, 1);
	lower4 |= En;
	i2c_senddata(TARGET_ADDR, &lower4, 1);
	nano_wait(1*1000*1000);
	lower4 &= ~En;
	i2c_senddata(TARGET_ADDR, &lower4, 1);
	nano_wait(1*1000*1000);

	/*char _4bit[2];
	_4bit[0] = (cmd&0xF0) | _BACKLIGHT;
	_4bit[1] = ((cmd&0x0F) << 4) | _BACKLIGHT;
	i2c_senddata(TARGET_ADDR, _4bit, 2);
	*/

}
void command(char cmd) {
	send(cmd, 0);
}

void displayX( int line, int col, char* s) {
	//set cursor
	line -= 1; //set to 1st entry at 0
	uint8_t ROW_OFFSET[] = {0x00,0x40,0x14,0x54};
	if(line > 3) line = 3;
	if(line < 0) line = 0;
	command(LCD_SETDDRAMADDR | (col + ROW_OFFSET[line]));

	//send data
	int len;
	for(len = 0; s[len] != '\0'; ++len); //set length
	//uint8_t _4bit[2*len];
	if(len > 20) len = 20;
	for(int i = 0; i < len; i++) {
		//here, Rs denotes "register select"
		send(s[i], Rs);
	}
}

void display(int line, char* s, char mode) {
	//mode can be center 'c', left 'l', right 'r'

	//clear entire line
	//display(line, "                    ", 'l');

	int size;
	for(size = 0; s[size] != '\0'; size++);
	if(size > 20) {
		size = 20;
	}

	int col;
	switch(mode) {
		case 'c':
			col = ((20 - size)/2);
			break;
		case 'r':
			col = (20-size);
			break;
		default:
			col = 0; //left default
		break;
	}
	//set cursor
	line -= 1; //set to 1st entry at 0
	uint8_t ROW_OFFSET[] = {0x00,0x40,0x14,0x54};
	if(line > 3) line = 3;
	if(line < 0) line = 0;
	command(LCD_SETDDRAMADDR | (col + ROW_OFFSET[line]));

	//send data
	int len;
	for(len = 0; s[len] != '\0'; ++len); //set length
	//uint8_t _4bit[2*len];
	if(len > 20) len = 20;
	for(int i = 0; i < len; i++) {
		//here, Rs denotes "register select"
		send(s[i], Rs);
	}
}

void clear_display() {
	command(LCD_CLEARDISPLAY);
	nano_wait(2000*1000);
}
void lcd_init() {
	i2c_init();
	nano_wait(50*1000*1000);
	//set to 4 bit mode
	for(int i = 0; i < 3; i++) {
		uint8_t cmd = 0x30;
		if(i==2) cmd = 0x20;

	i2c_senddata(TARGET_ADDR, &cmd, 1);
	nano_wait(1*1000*1000);
	//enable pulse?
	cmd |= En;
	i2c_senddata(TARGET_ADDR, &cmd, 1);
	nano_wait(1*1000*1000);
	cmd &= ~En;
	i2c_senddata(TARGET_ADDR, &cmd, 1);
	nano_wait(1*1000*1000);
	}
	command(LCD_FUNCTIONSET | _DISP_FUNC);
	command(LCD_DISPLAYCONTROL | _DISP_CONT);
	command(LCD_CLEARDISPLAY);
	nano_wait(2000*1000);
	//_BACKLIGHT = LCD_NOBACKLIGHT;
	command(LCD_ENTRYMODESET | _DISP_MODE);
	command(LCD_RETURNHOME);
	nano_wait(1*1000*1000);

}
