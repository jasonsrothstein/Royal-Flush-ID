#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>


static void nano_wait(int t) {
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

	//Set up timer

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

}



void init_led() {
	//LED 1,2,3 on PA8,9,10
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= (0b101010) << (2*8); //set each to ALT
	GPIOA->AFR[1] |= (0x222); //set to AF2
	//GPIOA->PUPDR |= (0b101010) << (2*8); //pull down

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	TIM1-> PSC = 48000-1;
	TIM1->ARR = 2000;
	TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E; //set to output
	TIM1->CCMR1 |= TIM_CCMR1_OC1M | TIM_CCMR1_OC2M;
	TIM1->CCMR2 |= TIM_CCMR2_OC3M; //set to inverted PWM
	TIM1->CCR1 = 2001;
	TIM1->CCR2 = 2001;
	TIM1->CCR3 = 2001;
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 |= TIM_CR1_CEN;
}
void led(int pin, int mode) {
       //pin = which LED (1,2,3)

       //mode:
           //0 = off
           //1 = on
           //2 = heartbeat
	int ARR = 2000;
	if(mode == 2) {
		if(pin == 1) {
			TIM1->CCR1 = ARR/2;
		} else if(pin == 2) {
			TIM1->CCR2 = ARR/2;
		} else if(pin == 3) {
			TIM1->CCR3 = ARR/2;
		}
	} else if(mode==1) {
		if(pin == 1) {
			TIM1->CCR1 = 0;
		} else if(pin == 2) {
			TIM1->CCR2 = 0;
		} else if(pin == 3) {
			TIM1->CCR3 = 0;
		}
	} else {
		//off
		if(pin == 1) {
			TIM1->CCR1 = ARR;
		} else if(pin == 2) {
			TIM1->CCR2 = ARR;
		} else if(pin == 3) {
			TIM1->CCR3 = ARR;
		}
	}


}

void TIM2_IRQHandler() {
	//check TIM2->SR CCXIF flag to determine source

    if(TIM2->SR & TIM_SR_CC1IF) {
    	if(TIM1->CCR1 == 0) {
    		led(1,0);
    	} else {
    		led(1,1);
    	}
    }
    else if(TIM2->SR & TIM_SR_CC2IF) {
    	led(1,2);
    }
    int fake;
    fake = TIM2->CCR1;
    fake = TIM2->CCR2;

    //do function
    TIM2->SR &= ~TIM_SR_UIF;
}

int main(void) {

       //init_LCD();
       //init_loadcell();
       //init_rfid();
       //init_xbee();
       init_led();
       init_btns();
       led(2, 1); //to confirm micro is working
       for(;;) {
    	   asm("wfi");
       }
       //int i = 0;

      // display(2, "SYSTEMS TEST", 'c');
}

