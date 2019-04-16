/*
 * LoadCell.c
 *
 *  Created on: Mar 30, 2019
 *      Author: James
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "LoadCell.h"
#include "rfid.h"

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
		nano_wait(500000);
		return(weight);
	}
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
	return(dec);
}

void zero(void) {
	offset = weigh();
}

int getChips(void) {
	int weight = weigh() - offset;
	// Else if for weight then chips
	return(weight);
}

int TESTfindAvg(void) {
	int average = 0;
	int sum = 0;
	while (1) {
		for (int i = 0; i < 10000; i++) {
			sum += getChips();
		}
		average = sum / 10000;
	}
}
