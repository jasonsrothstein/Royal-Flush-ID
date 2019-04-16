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



/* int main(void) {
	char card;
	char suit;
	char value;
	initSPI();
	initRFID();
	while(1) {
		card = getCard();
		suit = card % 4;
		value = card / 4;
	}
}
*/

///*
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
	int result = getHandStrengthC(cards);
	return(0);
}
//*/
/*
int main(void) {
	zero();
	TESTfindAvg();
}
*/
