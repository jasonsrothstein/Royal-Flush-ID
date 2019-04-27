/*
 * home.c
 *
 *  Created on: Apr 26, 2019
 *      Author: cunnin66
 */
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include "display.h"
#include "xbee.h"
#include "rfid.h"
#include "LoadCell.h"
#include "home.h"
#include "poker.h"

static char MY_ADDR = 0b0000;
int SCREEN = 0;
int STATE = 0;
int CHOICE = 0;
char OPTIONS[4][20];
int NUM_CHOICES = 4;
int NUM_PLAYERS = 0;
int TRAY_VALS[3] = {0,0,0};
int INIT_BANK = 0;

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
		int val = (int) xbee_readH(MY_ADDR);
		rv += (val)<<shift;
		shift += 4;
	}
	return rv;
}


void choice_update(int status) {
	//-1 is down list
	//0 is same
	//+1 is up list
	clear_line(3);
	if(status == 1) {
		CHOICE++;
		if(CHOICE >= NUM_CHOICES) CHOICE = 0;
	} else if(status == -1) {
		CHOICE--;
		if(CHOICE < 0) CHOICE = NUM_CHOICES-1;
	}
	display(3, OPTIONS[CHOICE], 'c');
}

void state_update(void) {
	if(STATE == 0) {
		//start up
		display(1, "Royal Flush ID",'c');
		display(4, "(BTN 2 to START)", 'c');

	} else if(STATE == 1) {
		//menu 1
		display(1, "Num. of Players?", 'c');
		strcpy(OPTIONS[0],"< 1 >");
		strcpy(OPTIONS[1],"< 2 >");
		strcpy(OPTIONS[2],"< 3 >");
		strcpy(OPTIONS[3], "< 4 >");
		NUM_CHOICES = 4;
		CHOICE = 1;
		choice_update(0);
	}
	else if(STATE == 2) {
		//menu 2
		display(1, "Chip Values?", 'c');
		strcpy(OPTIONS[0] , "< 1,1,1 >");
		strcpy(OPTIONS[1] , "< 1,5,10 >");
		strcpy(OPTIONS[2] , "< 10,25,50 >");
		NUM_CHOICES = 3;
		CHOICE = 0;
		choice_update(0);
	}
	else if(STATE == 3) {
		//menu 3
		display(1, "Starting Bank?", 'c');
		strcpy(OPTIONS[0],"< $100 >");
		strcpy(OPTIONS[1], "< $500 >");
		strcpy(OPTIONS[2] , "< $1000 >");
		strcpy(OPTIONS[3] , "< $5000 >");
		NUM_CHOICES = 4;
		CHOICE = 2;
		choice_update(0);
	}
	else if(STATE == 10) {
		//load cell zeroing
		display(1, "Tray Calibration", 'l');
		display(2, "Remove everything", 'c');
		display(3, "from trays", 'c');
		display(4, "BTN 2 when ready", 'l');
	}
	else if(STATE == 11) {
		//load cell calibration
		//zero(0);
		//zero(1);
		//zero(2);
		clear_display();
		display(1, "Tray Calibration", 'l');
		display(2, "Place 10 chips", 'c');
		display(3, "on each tray", 'c');
		display(4, "BTN 2 when ready", 'l');
	}
	else if(STATE == 12) {
		//getChipWeight(1);
		//getChipWeight(2);
		//getChipWeight(3);
		clear_display();
		display(1, "Done", 'l');
		display(2, "Remove chips", 'c');
		display(4, "BTN 2 when ready", 'l');
	}
	else if(STATE == 20) {
		//beginning of poker
		display(1, "POKER TIME", 'c');
		//set up w/ structures and PM
		initGame(NUM_PLAYERS, INIT_BANK);
		STATE = 21;
		state_update();
	} else if(STATE == 21) {

		//start scanning cards
		clear_display();
		display(1, "---- DEAL PHASE ----", 'c');
		display(2, "Scan card for: ", 'l');


		NUM_CHOICES = NUM_PLAYERS;
		strcpy(OPTIONS[0],"Player 1");
		strcpy(OPTIONS[1],"Player 2");
		strcpy(OPTIONS[2],"Player 3");
		strcpy(OPTIONS[3], "Player 4");
		CHOICE = (games.dealer + 1) % games.numPlayers;

		choice_update(0);
		dealHands();
		STATE = 22;
		state_update();

	} else if(STATE == 22) {
		clear_display();
		display(1, "ROUND 1 BETTING", 'c');
	}
}


void TIM2_IRQHandler() {
	//check TIM2->SR CCXIF flag to determine source

	if(TIM2->SR & TIM_SR_CC1IF) {
		//BTN 2

		if(STATE == 0) {
			clear_display();
			STATE = 1;
			state_update();
		}
		else if(STATE == 1) {
			//num players selected
			NUM_PLAYERS = CHOICE + 1;
			clear_display();
			STATE = 2;
			state_update();
		}
		else if(STATE == 2) {
			//chip values selected
			if(CHOICE == 1) {
				TRAY_VALS[0] = 1;
				TRAY_VALS[1] = 5;
				TRAY_VALS[2] = 10;
			}  else if(CHOICE == 2) {
				TRAY_VALS[0] = 10;
				TRAY_VALS[1] = 25;
				TRAY_VALS[2] = 50;
			}
			else {
				TRAY_VALS[0] = 1;
				TRAY_VALS[1] = 1;
				TRAY_VALS[2] = 1;
			}
			clear_display();
			STATE = 3;
			state_update();
		}
		else if(STATE == 3) {
			//initial bank selected
			if(CHOICE == 3) {
				INIT_BANK = 5000;
			}  else if(CHOICE == 2) {
				INIT_BANK = 1000;
			}
			else if(CHOICE == 2) {
				INIT_BANK = 500;
			}
			else {
				INIT_BANK = 100;
			}
			clear_display();
			STATE = 10;
			state_update();
		}
		else if(STATE == 10) {
			//clear_display();
			STATE = 11;
			state_update();
		}
		else if(STATE == 11) {
			//clear_display();
			STATE = 12;
			state_update();
		}
		else if(STATE == 12) {
			clear_display();
			STATE = 20;
			state_update();
		}

	}
	else if(TIM2->SR & TIM_SR_CC2IF) {
		//BTN 1
		//pass
		if((STATE >= 1) && (STATE <= 3)) {
			choice_update(-1);
		}

	}
	int fake;
	fake = TIM2->CCR1;
	fake = TIM2->CCR2;
	TIM2->SR &= ~TIM_SR_UIF;
}
void TIM1_CC_IRQHandler() {

	if((STATE >= 1) && (STATE <= 3)) {
		choice_update(1);
	}

	int fake;
	fake = TIM1->CCR4;
	TIM1->SR &= ~TIM_SR_UIF;
}
