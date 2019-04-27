#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include "display.h"
#include "xbee.h"
#include "rfid.h"
#include "LoadCell.h"
#include "home.h"
#include "poker.h"



void nano_wait(int t) {
	asm("       mov r0,%0\n"
			"repeat:\n"
			"       sub r0,#83\n"
			"       bgt repeat\n"
			: : "r"(t) : "r0", "cc");
}

//debouncing a push button

int main(void) {

	lcd_init();
	xbee_init();
	init_led();
	init_btns();
	initRFID();
	//initLoadCells();


	led(1,1);
	state_update();

	while(1) {
		//just waiting to do stuff
	}
}
