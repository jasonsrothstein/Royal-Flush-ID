#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include "display.h"
#include "xbee.h"

static char MY_ADDR = 0b0010;
int SCREEN = 10;
int NUM_SCREENS = 2;
int BANK = 0;
int BET = 0;
int POT = 0;
int CALL = 0;
int COMBO[6] = {5,0,0,0,0,0};
int STATUS = 0;
int STR = 0;
int HAND[7] = {5,0,0,0,0,0,0};
int H_INDEX = 0;


void nano_wait(int t) {
    asm("       mov r0,%0\n"
        "repeat:\n"
        "       sub r0,#83\n"
        "       bgt repeat\n"
        : : "r"(t) : "r0", "cc");
}

//debouncing a push button



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


void TIM2_IRQHandler() {
	//check TIM2->SR CCXIF flag to determine source

    if(TIM2->SR & TIM_SR_CC1IF) {
    	//BTN 2
    	//fold
    	if(STATUS == 6) {
    		STATUS = 0;
    		xbee_send(0, 4, &STATUS, 1);
    		update_display();
    		led(1,0);
    		led(3,0);
    	}

    }
    else if(TIM2->SR & TIM_SR_CC2IF) {
    	//BTN 1
    	//pass
    	if(STATUS == 6) {
    		STATUS = 1;
    		xbee_send(0, 4, &STATUS, 1);
    		led(3,0);
    		led(1,1);
    	}
    }
    int fake;
    fake = TIM2->CCR1;
    fake = TIM2->CCR2;
    TIM2->SR &= ~TIM_SR_UIF;
}
void TIM1_CC_IRQHandler() {
if(STATUS != 0) {
	SCREEN += 1;
	if(SCREEN > 1) {
		SCREEN = 0;
	}
	clear_display();
	update_display();

	int fake;
	fake = TIM1->CCR4;
	TIM1->SR &= ~TIM_SR_UIF;
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

int update_display() {
	if(SCREEN == 0) {
		display(1,"Pot: ", 'l');
		display(2,"Call: ", 'l');
		display(3, "Bet: ", 'l');
		display(4, "Bank: ", 'l');

		char num[7];
		if(STATUS == 4) {
			clear_line(1);
			display(1,"LITTLE BLIND", 'l');
		} else if(STATUS == 5) {
			clear_line(1);
			display(1,"BIG BLIND", 'l');
		} else {
			sprintf(num, "%d", POT);
			displayX(1, 5, num);
		}
		if(STATUS == 7) {
			display(2, "--- WINNER ---", 'c');
		} else if(STATUS == 8) {
			display(2, "*** SPLIT ***", 'c');
		} else {
			sprintf(num, "%d", CALL);
			displayX(2, 6, num);
		}
		if(STATUS == 0) {
			displayX(3,5,"FOLD");
		} else {
			sprintf(num, "%d", BET);
			displayX(3, 5, num);
		}
		if(STATUS == -1) {
			displayX(4, 6, "BUST    ");
		} else {
		sprintf(num, "%d", BANK);
		displayX(4, 6, num);
		}
	}
	else if(SCREEN == 1) {
		display(1, "HAND: ", 'l');
		switch(COMBO[0]) {
		case 1:
			displayX(1, 6, "HIGH CARD");
			break;
		case 2:
			displayX(1, 6, "1 PAIR");
			break;
		case 3:
			displayX(1, 6, "2 PAIR");
			break;
		case 4:
			displayX(1, 6, "3 OF A KIND");
			break;
		case 5:
			displayX(1, 6, "STRAIGHT");
			break;
		case 6:
			displayX(1, 6, "FLUSH");
			break;
		case 7:
			displayX(1, 6, "FULL HOUSE");
			break;
		case 8:
			displayX(1, 6, "4 OF A KIND");
			break;
		case 9:
			displayX(1, 6, "STRAIGHT FLUSH");
			break;
		case 10:
			displayX(1, 6, "ROYAL FLUSH");
			break;
		default:
			displayX(1,6, "N/A");
			break;
		}



		display(2, "STRENGTH: ", 'l');
		int i;
		char block[2];
		block[0] = 0b00101010;
		block[1] = '\0';
		for(i = 0; i < STR; i++) {
			displayX(2, 10+i, &block);
		}
		block[0] = ' ';
		for(i = 0; i < (10-STR); i++) {
			displayX(2, 10+STR+i, &block);
		}
		display(3, "POT ODDS: ", 'l');
		//char num[7];
		//sprintf(num, "%.2f", (double) POT / (double) (CALL-BET));


		//ind. load cells screen?
		//table stats
	} else if(SCREEN == 10) {
		display(1, "Royal Flush ID", 'c');
		display(3, "Please wait", 'c');
		display(4, "for setup", 'c');
	}
}


int main(void) {

	   lcd_init();
       xbee_init();
       init_led();
       init_btns();

       led(2,1);



       STR = 7;

       STATUS = 0; //auto join game
       update_display();


   	while(1) {
   		char byte1 = xbee_read();
   		if(byte1&(MY_ADDR << 4)) { //used to match address bit (so that broadcasted messages work too)
   			char cmd = 0xF & byte1;
   			if(cmd == 0) { //stop

   			} else if(cmd == 1) { //confirm

   			} else if(cmd == 2) { //error

   			} else if(cmd == 3) { //new game
   				//import all game settings
   				BANK = read_int();
   				//only need bank
   				POT = 0;
   				CALL = 0;
   				BET = 0;

   			} else if(cmd == 4) { //status update
   				STATUS = read_int();
   				if(STATUS == 1) { //activated
   					led(3,1);
   					SCREEN = 0;
   					clear_display();
   				}
   				else if(STATUS == 3) {
   					//dealer
   					led(2,1);
   					//return to active status
   					STATUS = 1;
   				} else if((STATUS >= 4) & (STATUS <= 6)) {
   					led(1,0);
   					led(3,1);
   				}
   				update_display();

   			} else if(cmd == 5) { //new card
   				HAND[H_INDEX] = read_int();
   				H_INDEX++;
   				if(H_INDEX == 5) {
   					//STR = getHandStrength5Cards(hand);
   				} else if(H_INDEX ==6) {
   					//STR = getHandStrength6Cards(hand);
   				} else if(H_INDEX ==7) {
   					//STR = getHandStrength7Cards(hand);
   				}
   				update_display();
   			} else if(cmd == 6) { //pot update
   				POT = read_int();
   				update_display();
   			} else if(cmd == 7) { //call update
   				CALL = read_int();
   				update_display();
   			} else if(cmd == 8) { //bet update
   				BET = read_int();
   				update_display();
   			} else if(cmd == 9) { //bank update
   				BANK = read_int();
   				update_display();
   			} else if(cmd == 10) { //send hand
   				xbee_send(0, 10, COMBO, 6);
   			} else if(cmd == 11) { //winning hand
   				BANK = read_int();
   				STATUS = 7;
   				update_display();
   			} else if(cmd == 12) { //new round
   				BET = 0;
   				POT = 0;
   				CALL = 0;
   				for(int i = 0; i < 7; i++) {
   					HAND[i] = 0;
   				}
   				for(int i=0; i<6;i++) {
   					COMBO[i] = 0;
   				}
   				STR = 0;
   				H_INDEX = 0;
   				//turn off dealer LED
   				led(2,0);

   				update_display();
   			} else if(cmd == 13) { //tie
   				int split = read_int();
   				STATUS = 8;

   				update_display();

   			} else if(cmd == 14) { //close network

   			} else if(cmd == 15) { //shut down

   			}
   		}
   	}
}
