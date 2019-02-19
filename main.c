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
			
void initSPI(void);
void sendSignal(int address, int signal);
//void initDMA(void);
void nano_wait(int t);
void initPA1(void);
int convert2s(int count);
int weigh(void);
void initRFID(void);
int receiveSignal(int address);

int main(void)
{
	initSPI();
	//initDMA();
	initRFID();
}

void initRFID(void) {
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

int weigh(void) {
	initPA1();
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

void initPA1(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;

	GPIOA->MODER &= ~GPIO_MODER_MODER1;
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

void initSPI(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |= GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);


	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI1
	SPI1->CR2 |= SPI_CR2_DS;
	SPI1->CR2 &= ~SPI_CR2_DS_3; // 8-BIT TRANSFER RATE
	SPI1->CR1 &= ~SPI_CR1_CPHA; // FIRST DATA CAPTURE EDGE
	SPI1->CR1 &= ~SPI_CR1_CPOL; // 0 WHEN IDLE
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;
	SPI1->CR1 |= SPI_CR1_MSTR;
	//SPI1->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR; // 1-LINE BIDERECTIONAL OUTPUT ENABLED MASTER CONFIGURATION
	SPI1->CR1 |= SPI_CR1_BR;
	SPI1->CR1 &= ~SPI_CR1_BR_0; // BAUD RATE 375 KHZ
	SPI1->CR2 |= SPI_CR2_SSOE; // SELECT SLAVE OUTPUT ENABLE
	SPI1->CR2 |= SPI_CR2_NSSP; // NSS PULSE
	SPI1->CR2 |= SPI_CR2_FRXTH; // RXNE EVENT TRIGGERED IF FIFO LEVEL >= 8 BITS
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST; // MOST SIGNIFICANT BIT TO BE SENT FIRST
	SPI1->CR1 |= SPI_CR1_SPE; // ENABLE SPI1

}

/*void initDMA(void) {
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel5->CCR &= ~(DMA_CCR_EN);
    DMA1_Channel5->CMAR = (uint32_t) SPI1->DR;
    DMA1_Channel5->CPAR = (uint32_t) &(SPI1->DR);
    DMA1_Channel5->CNDTR = 1;
    DMA1_Channel5->CCR &= ~(DMA_CCR_PINC | DMA_CCR_PL | DMA_CCR_MSIZE_1 | DMA_CCR_TEIE | DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_MEM2MEM);
    DMA1_Channel5->CCR |= DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE | DMA_CCR_CIRC;

    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    SPI1->CR2 |= SPI_CR2_RXDMAEN;

    DMA1_Channel5->CCR |= DMA_CCR_EN;
} */

void sendSignal(int address, int signal) {
	address = address << 1;
	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = address; // SEND ADDRESS

	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = signal; // SEND SIGNAL
}

int receiveSignal(int address) {
	int i;
	address = address << 1;
	while((SPI1->SR & SPI_SR_RXNE) == SPI_SR_RXNE) {
		i = SPI1->DR;
	}
	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = (0x80 | address); // SEND ADDRESS

	while((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
	return (SPI1->DR); // READ CONTENTS
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
