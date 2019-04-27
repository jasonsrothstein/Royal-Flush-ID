/*
 * xbee.c
 *
 *  Created on: Apr 26, 2019
 *      Author: cunnin66
 */


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"





static void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}


void led_init() {
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	GPIOC->MODER |= 1<<(2*0);
	GPIOC->ODR &= ~(1<<0);
}

void led_output(char c) {
	if(c) {
		GPIOC->ODR |= 1<<0;
	} else {
		GPIOC->ODR &= ~(1<<0);
	}
}

void usart2_init(void) {
	//pins
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(3<<(2*2) | 3<<(2*3)); //clear
	GPIOA->MODER |= 2<<(2*2) | 2<<(2*3); //set alt.
	GPIOA->AFR[0] &= ~(0xF <<(4*2) | 0xF<<(4*3));
	GPIOA->AFR[0] |= (1 <<(4*2) | 1 <<(4*(3))); //set to tx/rx

	//set up USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	USART2->CR1 &= ~(1<<28 | USART_CR1_OVER8 | USART_CR1_PCE); //set to 8 bit, 16x samp., parity off
	USART2->CR2 &= ~(USART_CR2_STOP); //1 stop bit
	//USART1->BRR = 20000; //0x4e20
	USART2->BRR = 48000000/9600;

	//enables
	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE |USART_CR1_UE;
	while((USART2->ISR & USART_ISR_REACK) == 0);
	while((USART2->ISR & USART_ISR_TEACK) == 0);
}

void swrite(char c) {
	while((USART2->ISR & USART_ISR_TXE) != USART_ISR_TXE);
	USART2->TDR = c;
	//USART1->CR1
}

char sread(void) {
	while((USART2->ISR & USART_ISR_RXNE) != USART_ISR_RXNE); //wait until equal
	char c = USART2->RDR;
	return c;
}

void xbee_init(void) {
	usart2_init();
}

void xbee_sendX(char c) {
	swrite(c);
}

void xbee_send(char addr, char cmd, int* data, int size) {
	//address needs to be in binary
	//cmd in binary
	char ADDR = ((addr&0xF)<<4);
	swrite(ADDR | (cmd&0xF)); //command byte
	int i;
	for(i=0; i< size; i++) {
		int d = data[i];
		int j;
		for(j=0; j<(2*4); j++) {
			char c = 0xF & d; //low 4 bits
			swrite(ADDR | c);

			d = d >> 4;
		}
	}


}
char xbee_read(void) {
	return sread();
}
char xbee_readH(char ADDR) {
	char c = xbee_read();
	while(!(c & (ADDR << 4))) {
		c = xbee_read();
	}
	return (0xF)&c;
}
