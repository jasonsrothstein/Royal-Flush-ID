/*
 * home.h
 *
 *  Created on: Apr 26, 2019
 *      Author: cunnin66
 */

#ifndef HOME_H_
#define HOME_H_

void init_btns(void);
void init_led(void);
void led(int, int);
int read_int(void);
void choice_update(int);
void state_update(void);
void TIM2_IRQHandler(void);
void TIM1_CC_IRQHandler(void);



//UNIVERSAL VARIABLES
static char MY_ADDR;
int SCREEN;
int STATE;
int CHOICE;
char OPTIONS[4][20];
int NUM_CHOICES;
int NUM_PLAYERS;
int TRAY_VALS[3];
int INIT_BANK;
int BLIND_INDEX;

#endif /* HOME_H_ */
