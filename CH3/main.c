#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <string.h>
#include "display.h"
#include "xbee.h"
#include "rfid.h"
#include "LoadCell.h"
#include "home.h"
#include "poker.h"




//debouncing a push button

int main(void) {

	lcd_init();
	xbee_init();
	init_led();
	init_btns();
	initRFID();
	initLoadCell();


	led(2,1);

	state_update();

	while(1) {
		//just waiting to do stuff
	}
}
