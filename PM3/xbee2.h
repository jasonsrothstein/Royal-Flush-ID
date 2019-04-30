
#ifndef XBEE2_H_
#define XBEE2_H_

void xbee_init();
void xbee_send(char, char, int*, int);
void xbee_sendX(char);
char xbee_read();
char xbee_sendH(char);
void nano_wait(int);


#endif
