/*
 * rfid.h
 *
 *  Created on: Mar 16, 2019
 *      Author: James
 */

#ifndef RFID_H_
#define RFID_H_



#endif /* RFID_H_ */

typedef struct {
	char size;			// Number of bytes in the UID. 4, 7 or 10.
	char uidByte[10];
	char sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
} Uid;

typedef struct {
	char keyByte[6];
} MIFARE_Key;

void writeRegister(char reg, char count, char * values);
void readRegister(char reg, char count, char * values, char rxAlign);
void transceiveData(char * sendData, char sendLen, char * backData, char * backLen, char * validBits, char rxAlign);
void communicateWithPICC(char command, char waitIRq, char * sendData, char sendLen, char * backData, char * backLen, char * validBits, char rxAlign);
void PICC_RequestA(char * bufferATQA, char * bufferSize);
void PICC_WakeupA(char * bufferATQA, char * bufferSize);
void PICC_REQA_WUPA(char command, char * bufferATQA, char * bufferSize);
void PICC_Select(Uid * uid, char validBits);
void PICC_HaltA(void);
void PCD_Authenticate(char command, char blockAddr, MIFARE_Key * key, Uid * uid);
char isNewCardPresent();
char readCardSerial();

void initSPI(void);
void sendSignal(char address, char signal);
void nano_wait(int t);
void initLoadCell(void);
int convert2s(int count);
int weigh(void);
void initRFID(void);
char receiveSignal(char address);
void writeDMA(char address, char signal);
void selfTest(void);
int RxRFID(void);
char Rx(void);
char scan4tag(void);
char * getUID(void);
void FIFOcheck(void);

Uid uid;
