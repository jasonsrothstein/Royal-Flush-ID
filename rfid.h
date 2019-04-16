/*
 * rfid.h
 *
 *  Created on: Mar 16, 2019
 *      Author: James
 */

#ifndef RFID_H_
#define RFID_H_

#endif /* RFID_H_ */

void initSPI(void);
void initRFID(void);
void sendSignal(char address, char signal);
char receiveSignal(char address);
void selfTest(void);
void FIFOcheck(void);
char scan4tag(void);
char getCard(void);
char UIDtoCard(char * uid);
char hexToChar(char hex);
void nano_wait(int t);
