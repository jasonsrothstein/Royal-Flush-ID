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

// Scale is 0, 1, or 2 and represents the physical load cell you wish to access
int weigh(int scale) {
	unsigned int count;
	count = 0;
	for (int n = 0; n < 24; n++) {
		//while((GPIOB->IDR & (1 << (scale * 2))) == 1);
		GPIOB->ODR |= (2 << (scale * 2));
		count = count << 1;
		nano_wait(40000);
		GPIOB->ODR &= ~(2 << (scale * 2));
		if (GPIOB->IDR & (1 << (scale * 2))) {
			count++;
		}
		nano_wait(40000);
	}
	GPIOB->ODR |= (2 << (scale * 2));
	nano_wait(40000);
	GPIOB->ODR &= ~(2 << (scale * 2));
	nano_wait(40000);

	nano_wait(100000000);

	return(convert2s(count));
}


int weigh3(void) {
	initLoadCell();
	unsigned int count;
	count = 0;
	for (int n = 0; n < 24; n++) {
		while((GPIOB->IDR & GPIO_IDR_4) == 1);
		GPIOB->ODR |= GPIO_ODR_5;
		count = count << 1;
		nano_wait(40000);
		GPIOB->ODR &= ~GPIO_ODR_5;
		if (GPIOB->IDR & GPIO_IDR_4) {
			count++;
		}
		nano_wait(40000);

	}
	GPIOB->ODR |= GPIO_ODR_5;
	nano_wait(40000);
	GPIOB->ODR &= ~GPIO_ODR_5;
	nano_wait(40000);

	nano_wait(100000000);
	return(convert2s(count));
}


void initLoadCell(void) {
	/*
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER |= GPIO_MODER_MODER0_0;	// OUTPUT MODE
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;	// PULL DOWN

	GPIOA->MODER &= ~GPIO_MODER_MODER1;		// INPUT MODE
	*/

	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER3 | GPIO_MODER_MODER5);
	GPIOB->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER5_0;	// OUTPUT MODE
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR3 | GPIO_PUPDR_PUPDR5);
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR1_1 | GPIO_PUPDR_PUPDR3_1 | GPIO_PUPDR_PUPDR5_1;	// PULL DOWN

	GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER2 | GPIO_MODER_MODER4);		// INPUT MODE
}

int convert2s(int count) {
	int dec = ~count;
	dec &= (0xFFFFFF);
	dec++;
	if (dec > 500000) {
		return(count * -1);
	}
	return(dec);
}

void zero(int scale) {
	int sum = 0;
	for (int i = 0; i < 5; i++) {
		weigh(scale);
	}
	nano_wait(100000000);
	for (int i = 0; i < 5; i++) {
		sum += weigh(scale);
	}
	offset[scale] = sum / 5;
}

int getChips(int scale) {
	int total = findAvg(scale) + (chipWeight[scale] / 2);
	return (total / chipWeight[scale]);
	// Else if for weight then chips
}

void getChipWeight(int scale) {
	chipWeight[scale] = findAvg(scale) / 10;
}

int findAvg(int scale) {
	int average;
	int sum = 0;
	for (int i = 0; i < 5; i++) {
		sum += weigh(scale) - offset[scale];
	}
	average = sum / 5;
	return(average);
}
