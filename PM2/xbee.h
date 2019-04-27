
#ifndef XBEE_H_
#define XBEE_H_

void xbee_init();
void xbee_send(char, char, int*, int);
void xbee_sendX(char);
char xbee_read();
char xbee_readH(char);


#endif /* DISPLAY_H_ */
