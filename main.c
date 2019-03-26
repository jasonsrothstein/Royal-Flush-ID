/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "rfid.h"

/*	IMPORTANT RFID REGISTERS
 * CommandReg		0x01
 * ComIrqReg		0x04
 * Status2Reg		0x08					Look into the RxWait and see if it is necessary to change
 * FIFODataReg		0x09
 * FIFOLevelReg		0x0A
 * ControlReg		0x0C
 * BitFramingReg	0x0D
 * ModeReg			0x11					Location of the TxWaitRF bit
 * RxModeReg		0x13					Location of RxMultiple bit
 * TxControlReg		0x14
 * TxSelReg			0x16					DriverSel: selects the input of drivers TX1 and TX2
 * 											MFOutSel: selects the input for pin MFOUT
 * RxSelReg			0x17					RxWait
 * RxThresholdReg	0x18					MinLevel
 * ModWidthReg		0x24					Defines the width of the MIller modulation as multiples of the carrier frequency
 *
*/

/* int main(void) {
	initSPI();
	initRFID();
	while(1) {
		if (!isNewCardPresent() || !readCardSerial()) {
			return(1);
		}
	}
} */

int main(void) {
	initSPI();
	//selfTest();
	initRFID();
	char j[5];
	while(1) {
		getUID();
	}
}

char * getUID(void) {
	char size = 0;
	char data = 0;
	char uid[5];
	for (int i = 0; i < 5; i++) {
		uid[i] = 0;
	}
	while(scan4tag() != 0x4);
	sendSignal(0x01, 0x00);
	sendSignal(0x04, 0x7F);
	sendSignal(0x0A, 0x80);
	sendSignal(0x09, 0x93);
	sendSignal(0x09, 0x20);
	sendSignal(0x0D, 0x00);
	sendSignal(0x01, 0x0C);
	sendSignal(0x0D, 0x80);
	for (int i = 0; i < 3000; i++) {
		data = receiveSignal(0x04);
		if (data & 0x20) {
			nano_wait(1000000);
			size = receiveSignal(0x0A);
			break;
		}
	}
	for (int i = 0; i < size; i++) {
		uid[i] = receiveSignal(0x09);
	}
	return(uid);
}

void FIFOcheck(void) {
	char check;
	check = receiveSignal(0x0A);
	check = receiveSignal(0x09);
}

char scan4tag(void) {
	char size = 0;
	char data = 0;
	data = receiveSignal(0x0E);
	sendSignal(0x0E, data & (~0x80));
	//data = receiveSignal(0x18);
	sendSignal(0x18, ((data & 0x0F) | 0x40));
	sendSignal(0x14, 0x83);
	//sendSignal(0x26, 0x78);
	//data = receiveSignal(0x11);
	//sendSignal(0x11, data | 0x20);
	while (size != 2) {
		size = 0;
		sendSignal(0x01, 0x00);
		sendSignal(0x04, 0x7F);
		data = receiveSignal(0x04);
		sendSignal(0x0A, 0x80);
		sendSignal(0x09, 0x26);
		sendSignal(0x0D, 0x07);
		sendSignal(0x01, 0x0C);
		sendSignal(0x0D, 0x87);
		//data = receiveSignal(0x06);
		for (int i = 0; i < 3000; i++) {
			data = receiveSignal(0x04);
			if (data & 0x20) {
				nano_wait(1000000);
				size = receiveSignal(0x0A);
				break;
			}
		}
	}
	data = 0;
	for(int i = 0; i < size; i++) {
		data += receiveSignal(0x09);
	}
	if (data == 4) {
		return(data);
	}
	return(scan4tag());
}

char Rx(void) {
	unsigned char size = 0;
	unsigned char data = 0;
	sendSignal(0x01, 0x00);
	sendSignal(0x04, 0x7F);
	sendSignal(0x0A, 0x80);
	sendSignal(0x0D, 0x00);
	sendSignal(0x14, 0x83);
	sendSignal(0x01, 0x08);
	while (!(data & 0x20)) {
		data = receiveSignal(0x0A);
		data = receiveSignal(0x09);
		data = receiveSignal(0x04);
	}
	size = receiveSignal(0x0A);
	for(int i = 0; i < size; i++) {
		data = receiveSignal(0x0A);
		data = receiveSignal(0x09);
	}
	return(data);
}

int RxRFID(void) {
	int tag = 0;
	int size = 0;
	while (size == 0) {
		sendSignal(0x01, 0x00);
		sendSignal(0x04, 0x7F);
		sendSignal(0x0A, 0x80);
		sendSignal(0x09, 0x93);
		sendSignal(0x09, 0x20);
		sendSignal(0x0D, 0x00);
		sendSignal(0x01, 0x0C);
		sendSignal(0x0D, 0x80);
		for (int i = 0; i < 2000; i++) {
			tag = receiveSignal(0x04);
			if (tag & 0x30) {
				size = 1;
				break;
			}
		}
	}
	tag = 0;
	size = receiveSignal(0x0A);
	for (int i = 0; i < size; i++) {
		tag = tag << 8;
		tag += receiveSignal(0x09);
	}
	return(tag);
}

void selfTest(void) {
	int b;
	int c;
	sendSignal(0x01, 0x0F);
	for (int i = 0; i < 25; i++) {
		sendSignal(0x09, 0x00);
	}
	sendSignal(0x01, 0x01);
	sendSignal(0x36, 0x09);
	sendSignal(0x09, 0x00);
	sendSignal(0x01, 0x03);
	for (int n = 0; n < 64; n++) {
		b = receiveSignal(0x0A);
		c = receiveSignal(0x09);
	}
}

void initSPI(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |= GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI1
	SPI1->CR1 &= ~(SPI_CR1_CRCEN | SPI_CR1_CRCL | SPI_CR1_CRCNEXT | SPI_CR1_RXONLY | SPI_CR1_RXONLY | SPI_CR1_SSM | SPI_CR1_LSBFIRST | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR);
	SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR;
	SPI1->CR1 &= ~SPI_CR1_BR_0; // BAUD RATE 375 KHZ
	SPI1->CR2 |= SPI_CR2_DS | SPI_CR2_NSSP | SPI_CR2_SSOE;
	SPI1->CR2 &= ~(SPI_CR2_FRXTH | SPI_CR2_FRF | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
	SPI1->CR1 |= SPI_CR1_SPE; // ENABLE SPI1
}

void initRFID(void) {
	sendSignal(0x01, 0x0F);
	nano_wait(100000000);
	sendSignal(0x12, 0x00);
	sendSignal(0x13, 0x00);
	sendSignal(0x24, 0x26);
	sendSignal(0x2A, 0x80);
	sendSignal(0x2B, 0xA9);
	sendSignal(0x2C, 0x03);
	sendSignal(0x2D, 0xE8);
	sendSignal(0x15, 0x40);
	sendSignal(0x11, 0x3D);
	int value = receiveSignal(0x14);
	if ((value & 0x03) != 0x03) {
		sendSignal(0x14, value | 0x03);
	}
}

void sendSignal(char address, char signal) {

	nano_wait(10000);

	int transmission = address << 9;
	transmission |= signal;

	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = transmission; // SEND ADDRESS
}

char receiveSignal(char address) {

	//nano_wait(1000000);

	char data;
	int transmission = address << 9;
	transmission |= 0x8000;

	while((SPI1->SR & SPI_SR_RXNE) == SPI_SR_RXNE) {
		data = SPI1->DR;
	}

	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = transmission; // SEND ADDRESS

	while((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
	data = SPI1->DR;

	return(data);
}

int weigh(void) {
	initLoadCell();
	unsigned int count;
	unsigned int complement;
	while(1) {
		count = 0;
		complement = 0;
		for (int n = 0; n < 24; n++) {
			while((GPIOA->IDR & GPIO_IDR_1) == 1);
			GPIOA->ODR |= GPIO_ODR_0;
			count = count << 1;
			complement = complement << 1;
			nano_wait(40000);
			GPIOA->ODR &= ~GPIO_ODR_0;
			if (GPIOA->IDR & GPIO_IDR_1) {
				count++;
			} else {
				complement++;
			}
			nano_wait(40000);

		}
		GPIOA->ODR |= GPIO_ODR_0;
		nano_wait(40000);
		GPIOA->ODR &= ~GPIO_ODR_0;
		nano_wait(40000);

		int weight;
		weight = complement + 1;
		weight = convert2s(count);
		printf("%d\n", weight);
		nano_wait(500000);
		return(weight);
	}
}

void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

void initLoadCell(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER |= GPIO_MODER_MODER0_0;	// OUTPUT MODE
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;	// PULL DOWN

	GPIOA->MODER &= ~GPIO_MODER_MODER1;		// INPUT MODE
}

int convert2s(int count) {
	int dec = ~count;
	for(int n = 0; n < 8; n++) {
		dec = dec << 1;
	}
	for(int n = 0; n < 8; n++) {
		dec = dec >> 1;
	}
	dec++;
	int result = -1*dec;
	result += 118000;
	return(dec);
}

void i2c_init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    GPIOB->MODER |= GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
    GPIOB->AFR[0] |= 1<<(4*6) | 1<<(4*7);

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    //RCC->CFGR3 |= RCC_CFGR3_I2C1SW;  // set for 48MHz sysclk

    // I2C CR1 Config
    I2C1->CR1 &= ~I2C_CR1_PE;           // Disable to perform reset.
    I2C1->CR1 &= ~I2C_CR1_ANFOFF;       // 0: Analog noise filter enabled.
    I2C1->CR1 &= ~I2C_CR1_ERRIE;        // Errors interrupt disable
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;    // Enable clock stretching

    // From table 83. p642 of FRM.  Set for 400 kHz with 8MHz clock.
    I2C1->TIMINGR = 0;
    I2C1->TIMINGR &= ~I2C_TIMINGR_PRESC;// Clear prescaler
    I2C1->TIMINGR |= 0 << 28;           // Set prescaler to 0
    I2C1->TIMINGR |= 3 << 20;           // SCLDEL
    I2C1->TIMINGR |= 1 << 16;           // SDADEL
    I2C1->TIMINGR |= 3 << 8;            // SCLH
    I2C1->TIMINGR |= 9 << 0;            // SCLL

    // I2C Own address 1 register (I2C_OAR1)
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR1 =   I2C_OAR1_OA1EN | (0x1<<1);// Set 7-bit own address 1

    I2C1->CR2 &= ~I2C_CR2_ADD10;        // 0 = 7-bit mode; 1 = 10-bit mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;       // Enable the auto end
    I2C1->CR2 |= I2C_CR2_NACK;          // For slave mode: set NACK

    I2C1->CR1 |= I2C_CR1_PE;            // Enable I2C1
}
