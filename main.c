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
#include "rfid.h"
#include "LoadCell.h"
#include "poker.h"
#include "player.h"


///*
int main(void) {
	char value;
	//initSPI();
	//initRFID();
	char val;
	char suit;
	for (int i = 0; i < 7; i++) {
		hand[i] = 69;
	}
	val = 7;
	suit = 1;
	hand[0] = val * 4 + suit;
	val = 14;
	suit = 0;
	hand[1] = val * 4 + suit;
	sortCardsLtoG(hand);
	value = getHandStrength2Cards(hand);
	val = 6;
	suit = 3;
	hand[2] = val * 4 + suit;
	val = 10;
	suit = 3;
	hand[3] = val * 4 + suit;
	val = 5;
	suit = 1;
	hand[4] = val * 4 + suit;
	sortCardsLtoG(hand);
	value = getHandStrength5Cards(hand);
	val = 9;
	suit = 0;
	hand[5] = val * 4 + suit;
	sortCardsLtoG(hand);
	value = getHandStrength6Cards(hand);
	val = 3;
	suit = 0;
	hand[6] = val * 4 + suit;
	sortCardsLtoG(hand);
	value = getHandStrength7Cards(hand,1);
	return(value);
	/*while(1) {
		for (int i = 0; i < 7; i++) {
			hand[i] = 99;
		}
		for (int i = 0; i < 2; i++) {
			hand[i] = getCard();
		}
		sortCardsLtoG(hand);
		value = getHandStrength2Cards(hand);
		for (int i = 2; i < 5; i++) {
			hand[i] = getCard();
		}
		sortCardsLtoG(hand);
		value = getHandStrength5Cards(hand);
		hand[5] = getCard();
		sortCardsLtoG(hand);
		value = getHandStrength6Cards(hand);
		hand[6] = getCard();
		sortCardsLtoG(hand);
		value = getHandStrength7Cards(hand,1);
	}
	return(value);
	*/
}
//*/

/*
int main(void) {
	char cards[7];
	cards[0] = 8;
	cards[1] = 12;
	cards[2] = 16;
	cards[3] = 28;
	cards[4] = 58;
	cards[5] = 99;
	cards[6] = 99;
	sortCards(cards);
	int result = getHandStrength7Cards(cards);
	return(0);
}
*/
/*
int main(void) {
	initLoadCell();
	for (int i = 0; i < 3; i++) {
		zero(i);
	}
	for (int i = 0; i < 3; i++) {
		getChipWeight(i);
	}
	int chips1;
	int chips2;
	int chips3;
	while (1) {
		chips1 = getChips(0);
		chips2 = getChips(1);
		chips3 = getChips(2);
	}
	chips1 = chips2;
	chips1 = chips3;
	return(chips1);
}
*/

/*
int main(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER8;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR8;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR8_1;

	GPIOA->MODER &= ~GPIO_MODER_MODER9;
	GPIOA->MODER |= GPIO_MODER_MODER9_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_1;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODER10;
	GPIOA->MODER |= GPIO_MODER_MODER10_0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_1;

	while(1) {
		GPIOA->ODR |= GPIO_ODR_8;
		for (int i = 0; i < 10; i++) {
			nano_wait(30000000);
		}
		GPIOA->ODR |= GPIO_ODR_9;
		for (int i = 0; i < 10; i++) {
			nano_wait(30000000);
		}
		GPIOA->ODR |= GPIO_ODR_10;
		for (int i = 0; i < 10; i++) {
			nano_wait(30000000);
		}
		GPIOA->ODR &= ~GPIO_ODR_8;
		for (int i = 0; i < 10; i++) {
			nano_wait(30000000);
		}
		GPIOA->ODR &= ~GPIO_ODR_9;
		for (int i = 0; i < 10; i++) {
			nano_wait(30000000);
		}
		GPIOA->ODR &= ~GPIO_ODR_10;
	}
}
*/
