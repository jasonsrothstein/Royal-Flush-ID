#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include <stdio.h>
#include "display.h"
#include "player.h"
#include "xbee2.h"

int SCREEN = 10;
int NUM_SCREENS = 2;
int EDIT = 0;

int BANK = 0;
int BET = 0;
int POT = 0;
int CALL = 0;
int COMBO[6] = {0,0,0,0,0,0};
int STATUS = 0;
int STR = 0;
int H_INDEX = 0;
int ROUND = 0;//kinda need that new round command to reset this
int WINNINGS = 0;
int WINNERS = 0;
int WIN_HAND = 0;
int OLD_BANK = 0;
int OLD_CALL = 0;
int OLD_BET = 0;
int RESULTS = 0; //0-N/A, 1-fold, 2-lose, 3-side pot, 4-split, 5-win

static char POKER_HANDS[11][14] = {
		"N/A",
		"HIGH CARD",
		"1 PAIR",
		"2 PAIR",
		"3 OF A KIND",
		"STRAIGHT",
		"FLUSH",
		"FULL HOUSE",
		"4 OF A KIND",
		"STRAIGHT FLUSH",
		"ROYAL FLUSH"
};


void init_btns() {
	//Btns 1,2,3 on PA0,1,11 on TIM2(1), (2), TIM1(4)
	//set up GPIOs

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (0b1010<<(2*0)) | (0b10 << (2*11)); //PA0,1 | PA11
	GPIOA->AFR[0] |= (0b0010 << (4*0)) | (0b0010 << (4*1));
	GPIOA->AFR[1] |=  (0b0010 << (4*(11-8)));
	GPIOA->PUPDR |= (0b0101 << (0*2)) | (0b01 << (2*11)); //pull up

	//Set up timer 2 (btn 1 and 2)

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 1-1;
	TIM2->ARR = ~(0x0);
	TIM2->CCMR1 |= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_1; //set CH1,2 to inputs
	TIM2->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E | TIM_CCER_CC2P | TIM_CCER_CC1P; //set ch to pin, active low parity
	TIM2->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC1IE;
	TIM2->CCMR1 |= TIM_CCMR1_IC2F_3 | TIM_CCMR1_IC2F_2 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_0 |
			TIM_CCMR1_IC1F_3 | TIM_CCMR1_IC1F_2 | TIM_CCMR1_IC1F_1 | TIM_CCMR1_IC1F_0; //set filtering?
	TIM2->CR1 |= TIM_CR1_CKD_1; //this is not right, set to CKD?
	TIM2->CR1 |= TIM_CR1_CEN;
	NVIC->ISER[0] = 1 <<TIM2_IRQn;

	//set up timer 1 (btn3)
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1->PSC = 1-1;
	TIM1->ARR = ~(0x0);
	TIM1->CCMR2 |= TIM_CCMR2_CC4S_0;
	TIM1->CCER |= TIM_CCER_CC4E | TIM_CCER_CC4P;
	TIM1->DIER |= TIM_DIER_CC4IE;
	TIM1->CCMR2 |= TIM_CCMR2_IC4F;
	TIM1->CR1 |= TIM_CR1_CKD_1;
	TIM1->CR1 |= TIM_CR1_CEN;
	NVIC->ISER[0] = 1 << TIM1_CC_IRQn;

}


void init_led() {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (0b010101) << (2*8); //set each to output (PA8,9,10)
	GPIOA->ODR &= ~(0b111 << (8)); //set to off
}

void led(int pin, int state) {
	pin += 7; //adjust
	if(state == 2) {
		GPIOA->ODR ^= (1<<pin); //xor = toggle
	} else if(state == 1) {
		GPIOA->ODR |= (1<<pin);
	} else { //state = 0, default
		GPIOA->ODR &= ~(1<<pin);
	}
}


int read_int(void) {
	int rv = 0;
	int j;
	int shift = 0;
	for(j=0; j<(2*4); j++) {
		int val = (int) xbee_read();
		rv += (val)<<shift;
		shift += 4;
	}
	return rv;
}


void TIM2_IRQHandler() {
	//check TIM2->SR CCXIF flag to determine source
	if(TIM2->SR & TIM_SR_CC1IF) {
		//BTN 2
		//fold
		if(STATUS == 6) {
			xbee_sendX(3); //request to stop
			xbee_read(); //wait for go ahead
			xbee_send(0, 4, &STATUS, 1); //send status
			STATUS = 0;
			led(1,0);
			led(3,0);
			EDIT = (0 << 4) | 6;
		}

	}
	else if(TIM2->SR & TIM_SR_CC2IF) {
		//BTN 1
		//pass
		if((STATUS <= 6) &&(STATUS >=4)) {

			xbee_sendX(3);
			//nano_wait(5*1000*1000);
			//char recieved = 0;
			//while(recieved != 1) {
			xbee_read();
			//}
			//nano_wait(5*1000*1000);
			//nano_wait(10*1000*1000);
			xbee_send(0, 4, &STATUS, 1);
			BANK -= BET - OLD_BET;
			OLD_BET = BET;
			if(STATUS == 6) {
				EDIT = (0<<4) | 6;
			}
			else {
				EDIT = (0<<4) | 6; //big/little blind needs reset
			}
			led(3,0);
			led(1,1);
			STATUS = 1;
		}

	}
	int fake;
	fake = TIM2->CCR1;
	fake = TIM2->CCR2;
	TIM2->SR &= ~TIM_SR_UIF;
}
void TIM1_CC_IRQHandler() {
	if(STATUS != 0) {
		int new_screen = SCREEN + 1;
		if((new_screen > 7) && (new_screen < 9)) { //screen is 6-8
			new_screen = 6;
		}
		else if((new_screen > 1) && (new_screen < 6)) { //screen is 0-2
			new_screen = 0;
		}

		EDIT = (new_screen<<4) | 7;

		int fake;
		fake = TIM1->CCR4;
		TIM1->SR &= ~TIM_SR_UIF;
	}
}


void update_display() {
	if(EDIT == 0) return; //nothing to do
	int new_screen = ((EDIT & 0xF0) >> 4);
	int lines = 0x0F & EDIT;

	if(new_screen != SCREEN) {
		if(lines == 7) { //force new screen
			//nano_wait(100*1000*1000);
			SCREEN = new_screen;
			//lines = 7;
		}
		else {
			return; //update does not pertain to current screen
		}
	}
	if(lines > 5) clear_display();

	EDIT = 0; 	//reset so if changed during execution, it will run again with new update
	char num[7]; //used for displaying numbers

	if(SCREEN == 0) {
		if(lines > 6) {
			display(1,"Pot: ", 'l');
			display(2,"Call: ", 'l');
			display(3, "Bet: ", 'l');
			display(4, "Bank: ", 'l');
		}

		if((lines == 1) || (lines >5)) {
			if(STATUS == 4) {
				//clear_line(1);
				display(1,"LITTLE BLIND    ", 'l');
			} else if(STATUS == 5) {
				//clear_line(1);
				display(1,"BIG BLIND      ", 'l');
			} else {
				display(1,"Pot: ", 'l');
				sprintf(num, "%6d", POT);
				displayX(1, 5, num);
			}
		}

		if((lines == 2) || (lines >5)) {
			sprintf(num, "%5d", CALL);
			displayX(2, 6, num);
		}
		if((lines == 3) || (lines >5)) {
			if(STATUS == 0) {
				displayX(3,5,"FOLD");
			} else {
				sprintf(num, "%6d", BET);
				displayX(3, 5, num);
			}
		}
		if((lines == 4) || (lines >5)) {
			if(STATUS == -1) {
				displayX(4, 6, "BUST    ");
			} else {
				sprintf(num, "%5d", BANK);
				displayX(4, 6, num);
			}
		}
	}
	else if(SCREEN == 1) {
		if (lines > 6) {
			display(1, "Hand: ", 'l');
			display(2, "Strength: ", 'l');
			//display(3, "Pot Odds: ", 'l');
		}
		if((lines == 1) || (lines >5)) {
			displayX(1, 6, POKER_HANDS[COMBO[0]]);
		}
		if((lines == 2) || (lines >5)) {
			int i;
			char block[2];
			block[0] = 0b00101010;
			block[1] = '\0';
			if(STR > 10) STR = 10;
			for(i = 0; i < STR; i++) {
				displayX(2, 10+i, block);
			}
			block[0] = ' ';
			for(i = 0; i < (10-STR); i++) {
				displayX(2, 10+STR+i, block);
			}
		}
		if((lines == 3) || (lines > 5)) {
			float pot_odds = 0;
			if((CALL-BET != 0)) {
				pot_odds = (float) POT / (float) (CALL-BET);
			}
			//sprintf(num, "%.2f", pot_odds);
		}

		//ind. load cells screen?
		//table stats
	}
	else if(SCREEN == 6) {
		//ind. results screen
		if(lines ==7) {
			display(1, "INDIVIDUAL", 'c');
			display(2, "Outcome: ", 'l');
			display(3, "Winnings: ", 'l');
			display(4, "Net: ", 'l');
		}
		if((lines == 2) || (lines>5)) {
			if(RESULTS == 5) {
				displayX(2, 16, "WIN");
			} else if(RESULTS == 4) {
				displayX(2, 14, "SPLIT");
			} else if(RESULTS == 3) {
				displayX(2, 11, "SIDE WIN");
			} else if(RESULTS == 2) {
				displayX(2, 15, "LOSE");
			} else if(RESULTS == 1) {
				displayX(2, 15, "FOLD");
			} else {
				displayX(2, 16, "N/A");
			}
		}
		if((lines == 3) || (lines>5)) {

			sprintf(num, "%6d", WINNINGS);
			displayX(3, 9, num);
		}
		if((lines == 4) || (lines>5)) {
			sprintf(num, "%6d", BANK - OLD_BANK);
			displayX(4, 9, num);
		}
	} else if(SCREEN == 7) {
		//table results
		if((lines==7)) {
			char title[20];
			sprintf(title, "ROUND %d RESULTS", ROUND);
			display(1, title, 'c');
			display(2, "Total Pot: ", 'l');
			display(3, "Winner: ", 'l');

		}
		if((lines == 2) || (lines>5)) {
			sprintf(num, "%6d", POT);
			displayX(2, 14, num);
		}
		if((lines == 3) || (lines>5)) {
			int col = 8;
			if(WINNERS & 0b0001) {
				displayX(3, col, "P1");
				col+= 2;
			}
			if(WINNERS & 0b0010) {
				if(col != 8) {
					displayX(3, col, "/");
					col+= 1;
				}
				displayX(3, col, "P2");
				col+= 2;
			}if(WINNERS & 0b0100) {
				if(col != 8) {
					displayX(3, col, "/");
					col+= 1;
				}
				displayX(3, col, "P3");
				col+= 2;
			}
			if(WINNERS & 0b1000) {
				if(col != 8) {
					displayX(3, col, "/");
					col+= 1;
				}
				displayX(3, col, "P4");
				col+= 2;
			}
		}
		if((lines == 4) || (lines>5)) {
			//display(4, "      ", 'l');
			displayX(4, 6, POKER_HANDS[WIN_HAND]);
		}
	}
	else if(SCREEN == 10) {
		//waiting screen
		if((lines==7)) {
			display(1, "Royal Flush ID", 'c');
			display(3, "Please wait", 'c');
			display(4, "for setup", 'c');
		}

	}
}

void USART2_IRQHandler() {
	char c = USART2->RDR; //address byte
	USART2->ICR |= USART_ICR_CMCF; //clear flag
	char cmd = xbee_read();
	if(cmd == 0) { //stop


	} else if(cmd == 1) { //confirm
		if(STATUS == 7) {
			STATUS = 8; //meaning, full send ahead
			EDIT = (6<<4 | 7);
			led(2,0); //do other LED stuff here

		}
	} else if(cmd == 2) { //interrupt

	} else if(cmd == 3) { //new game
		//import all game settings
		//BANK = read_int();
		//only need bank
		//POT = 0;
		//CALL = 0;
		//BET = 0;

	} else if(cmd == 4) { //status update
		STATUS = read_int();
		if(STATUS == 1) { //activated
			led(1,1);
			EDIT = (0<<4) | 7;
		}
		else if(STATUS == 3) {
			//dealer
			led(2,1);
			//return to active status
			STATUS = 1;
		} else if((STATUS >= 4) & (STATUS <= 6)) {
			led(1,0);
			led(3,1);
			EDIT = (0<<4 | 7);
		}

	} else if(cmd == 5) { //new card
		hand[H_INDEX] = 0xFF & read_int();
		H_INDEX++;
		//if(H_INDEX > 6) H_INDEX = 6; //overflow protection

		if(H_INDEX == 2) {
			STR = getHandStrength2Cards(hand);
		}
		else if(H_INDEX == 5) {
			STR = getHandStrength5Cards(hand);
		} else if(H_INDEX ==6) {
			STR = getHandStrength6Cards(hand);
		} else if(H_INDEX ==7) {
			STR = getHandStrength7Cards(hand,1);
			COMBO[1] = fiveCardHand[0];
			COMBO[2] = fiveCardHand[1];
			COMBO[3] = fiveCardHand[2];
			COMBO[4] = fiveCardHand[3];
			COMBO[5] = fiveCardHand[4];
		}
		OLD_CALL += CALL;
		CALL = 0;
		COMBO[0] = currentHand;
		BET = 0;
		OLD_BET = 0;
		EDIT = ((1<<4) | 6);

	} else if(cmd == 6) { //pot update
		POT = read_int();
		EDIT = ((0<<4) | 1);
	} else if(cmd == 7) { //call update
		CALL = read_int() - OLD_CALL;
		EDIT = ((0<<4) | 2);
	} else if(cmd == 8) { //bet update
		int diff = read_int();
		BET = diff + OLD_BET;
		//BANK -= diff;
		EDIT = ((0<<4) | 3);
	} else if(cmd == 9) { //bank update
		BANK = read_int();
		EDIT = ((0<<4) | 4);
	} else if(cmd == 10) { //send hand
		xbee_send(0, 10, COMBO, 6);
	} else if(cmd == 11) { //ind. results
		RESULTS = read_int(); //one char
		WINNINGS = read_int(); //how much to pick up off the table
		BANK += WINNINGS;
		STATUS = 7;

	} else if(cmd == 12) { //new round
		ROUND++;
		BET = 0;
		OLD_CALL = 0;
		POT = 0;
		CALL = 0;
		RESULTS = 0;
		WINNINGS = 0;
		WINNERS = 0;
		WIN_HAND = 0;
		OLD_BET = 0;
		OLD_BANK = BANK;

		for(int i = 0; i < 7; i++) {
			hand[i] = 99;
		}
		for(int i=0; i<6;i++) {
			COMBO[i] = 0;
		}
		STR = 0;
		H_INDEX = 0;
		//turn off dealer LED
		led(2,0);

		//SCREEN = 0;
		EDIT = ((0<<4) | 7);
	} else if(cmd == 13) { //table results
		WINNERS = read_int();
		WIN_HAND = read_int();

		if(STATUS == 0) {
			RESULTS = 1;
		} else if(STATUS == -1) {
			RESULTS = 0;
		}
		else if(STATUS != 7){
			RESULTS = 2;
		}
		STATUS = 7;

	} else if(cmd == 14) { //open network / reset
		OLD_BET = 0;
		ROUND = 0;
		BANK = 0;
		OLD_CALL = 0;
		BET = 0;
		POT = 0;
		CALL = 0;
		RESULTS = 0;
		WINNINGS = 0;
		WINNERS = 0;
		WIN_HAND = 0;
		OLD_BANK = 0;

		for(int i = 0; i < 7; i++) {
			hand[i] = 99;
		}
		for(int i=0; i<6;i++) {
			COMBO[i] = 0;
		}
		STR = 0;
		H_INDEX = 0;
		//turn on dealer LED
		led(2,1);
		STATUS = 0;
		EDIT = (10 << 4) | 7;
	} else if(cmd == 15) { //close network
		//MY_ADDR = read_int();
		//STATUS = 10;
	}
}

int main(void) {


	lcd_init();
	xbee_init();
	init_led();
	init_btns();

	led(2,1);
	EDIT = (10 << 4 | 7);
	while(1) {
		update_display();
	}
}
