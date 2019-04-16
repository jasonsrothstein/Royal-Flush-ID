#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "rfid.h"
#include <string.h>

char hexToChar(char hex) {
	if (hex < 10) {
		return(hex + 48);
	}
	else {
		return(hex + 55);
	}
}

char UIDtoCard(char * uid) {
	char id[15];
	for (int i = 0; i < 5; i++) {
		id[3*i] = hexToChar(uid[i] >> 4);
		id[3*i+1] = hexToChar(uid[i] % 16);
		id[3*i+2] = ':';
	}
	id[14] = 0;
	if (!strcmp(id, "36:91:A4:3F:3C")) {
		return(8);
	}
	else if (!strcmp(id,"87:89:80:F5:7B")) {
		return(9);
	}
	else if (!strcmp(id,"C6:53:A6:3F:0C")) {
		return(10);
	}
	else if (!strcmp(id,"06:AD:A2:3F:36")) {
		return(11);
	}
	else if (!strcmp(id,"96:8F:A4:3F:82")) {
		return(12);
	}
	else if (!strcmp(id,"B7:4F:80:F5:8D")) {
		return(13);
	}
	else if (!strcmp(id,"B6:A8:A4:3F:85")) {
		return(14);
	}
	else if (!strcmp(id,"E6:B6:A2:3F:CD")) {
		return(15);
	}
	else if (!strcmp(id,"F6:8D:A4:3F:E0")) {
		return(16);
	}
	else if (!strcmp(id,"57:4F:80:F5:6D")) {
		return(17);
	}
	else if (!strcmp(id,"26:A4:A4:3F:19")) {
		return(18);
	}
	else if (!strcmp(id,"46:B6:A2:3F:6D")) {
		return(19);
	}
	else if (!strcmp(id,"36:A7:A4:3F:0A")) {
		return(20);
	}
	else if (!strcmp(id,"F7:4E:80:F5:CC")) {
		return(21);
	}
	else if (!strcmp(id,"E6:9E:A4:3F:E3")) {
		return(22);
	}
	else if (!strcmp(id,"26:BC:A2:3F:07")) {
		return(23);
	}
	else if (!strcmp(id,"C6:A5:A4:3F:F8")) {
		return(24);
	}
	else if (!strcmp(id,"57:EF:7E:F5:33")) {
		return(25);
	}
	else if (!strcmp(id,"26	A6:A2:A4:3F:9F")) {
		return(26);
	}
	else if (!strcmp(id,"36:9A:A4:3F:37")) {
		return(27);
	}
	else if (!strcmp(id,"06:AB:A4:3F:36")) {
		return(28);
	}
	else if (!strcmp(id,"06:97:A4:3F:0A")) {
		return(29);
	}
	else if (!strcmp(id,"97:83:84:F5:65")) {
		return(30);
	}
	else if (!strcmp(id,"96:98:A4:3F:95")) {
		return(31);
	}
	else if (!strcmp(id,"86:50:A6:3F:4F")) {
		return(32);
	}
	else if (!strcmp(id,"76:95:A4:3F:78")) {
		return(33);
	}
	else if (!strcmp(id,"37:83:84:F5:C5")) {
		return(34);
	}
	else if (!strcmp(id,"E6:92:A4:3F:EF")) {
		return(35);
	}
	else if (!strcmp(id,"E6:50:A6:3F:2F")) {
		return(36);
	}
	else if (!strcmp(id,"66:A0:A4:3F:5D")) {
		return(37);
	}
	else if (!strcmp(id,"A7:14:84:F5:C2")) {
		return(38);
	}
	else if (!strcmp(id,"D7:59:84:F5:FF")) {
		return(39);
	}
	else if (!strcmp(id,"36:50:A6:3F:FF")) {
		return(40);
	}
	else if (!strcmp(id,"46:A1:A4:3F:7C")) {
		return(41);
	}
	else if (!strcmp(id,"F7:83:84:F5:05")) {
		return(42);
	}
	else if (!strcmp(id,"E7:56:86:F5:C2")) {
		return(43);
	}
	else if (!strcmp(id,"97:16:82:F5:F6")) {
		return(44);
	}
	else if (!strcmp(id,"D6:9F:A4:3F:D2")) {
		return(45);
	}
	else if (!strcmp(id,"B7:2C:86:F5:E8")) {
		return(46);
	}
	else if (!strcmp(id,"47:57:86:F5:63")) {
		return(47);
	}
	else if (!strcmp(id,"37:16:82:F5:56")) {
		return(48);
	}
	else if (!strcmp(id,"36:A5:A4:3F:08")) {
		return(49);
	}
	else if (!strcmp(id,"57:14:84:F5:32")) {
		return(50);
	}
	else if (!strcmp(id,"A7:57:86:F5:83")) {
		return(51);
	}
	else if (!strcmp(id,"D7:15:82:F5:B5")) {
		return(52);
	}
	else if (!strcmp(id,"66:96:A4:3F:6B")) {
		return(53);
	}
	else if (!strcmp(id,"17:2D:86:F5:49")) {
		return(54);
	}
	else if (!strcmp(id,"27:71:86:F5:25")) {
		return(55);
	}
	else if (!strcmp(id,"E7:89:80:F5:1B")) {
		return(56);
	}
	else if (!strcmp(id,"F6:97:A4:3F:FA")) {
		return(57);
	}
	else if (!strcmp(id,"77:73:82:F5:73")) {
		return(58);
	}
	else if (!strcmp(id,"87:71:86:F5:85")) {
		return(59);
	}
	else if (!strcmp(id,"F7:EE:7E:F5:92")) {
		return(60);
	}
	else if (!strcmp(id,"17:73:82:F5:13")) {
		return(61);
	}
	else {
		return(getCard());
	}

	/*
	8	36:91:A4:3F:3C
	9	87:89:80:F5:7B
	10	C6:53:A6:3F:0C
	11	06:AD:A2:3F:36
	12	96:8F:A4:3F:82
	13	B7:4F:80:F5:8D
	14	B6:A8:A4:3F:85
	15	E6:B6:A2:3F:CD
	16	F6:8D:A4:3F:E0
	17	57:4F:80:F5:6D
	18	26:A4:A4:3F:19
	19	46:B6:A2:3F:6D
	20	36:A7:A4:3F:0A
	21	F7:4E:80:F5:CC
	22	E6:9E:A4:3F:E3
	23	26:BC:A2:3F:07
	24	C6:A5:A4:3F:F8
	25	57:EF:7E:F5:33
	26	A6:A2:A4:3F:9F
	27	36:9A:A4:3F:37
	28	06:AB:A4:3F:36
	29	06:97:A4:3F:0A
	30	97:83:84:F5:65
	31	96:98:A4:3F:95
	32	86:50:A6:3F:4F
	33	76:95:A4:3F:78
	34	37:83:84:F5:C5
	35	E6:92:A4:3F:EF
	36	E6:50:A6:3F:2F
	37	66:A0:A4:3F:5D
	38	A7:14:84:F5:C2
	39	D7:59:84:F5:FF		9 SPADES SOMETIMES BUGGY
	40	36:50:A6:3F:FF
	41	46:A1:A4:3F:7C
	42	F7:83:84:F5:05
	43	E7:56:86:F5:C2
	44	97:16:82:F5:F6
	45	D6:9F:A4:3F:D2
	46	B7:2C:86:F5:E8
	47	47:57:86:F5:63
	48	37:16:82:F5:56
	49	36:A5:A4:3F:08
	50	57:14:84:F5:32
	51	A7:57:86:F5:83
	52	D7:15:82:F5:B5
	53	66:96:A4:3F:6B
	54	17:2D:86:F5:49
	55	27:71:86:F5:25
	56	E7:89:80:F5:1B
	57	F6:97:A4:3F:FA
	58	77:73:82:F5:73
	59	87:71:86:F5:85
	JB	F7:EE:7E:F5:92
	JC	17:73:82:F5:13
	 */
}

char getCard(void) {
	char size = 0;
	char data = 0;
	char uid[5];
	for (int i = 0; i < 5; i++) {
		uid[i] = 0;
	}
	while(scan4tag() != 0x4);
	sendSignal(0x01, 0x00);
	sendSignal(0x04, 0x7F);
	sendSignal(0x0A, 0x80);
	sendSignal(0x09, 0x93);
	sendSignal(0x09, 0x20);
	sendSignal(0x0D, 0x00);
	sendSignal(0x01, 0x0C);
	sendSignal(0x0D, 0x80);
	for (int i = 0; i < 3000; i++) {
		data = receiveSignal(0x04);
		if (data & 0x20) {
			nano_wait(1000000);
			size = receiveSignal(0x0A);
			break;
		}
	}
	for (int i = 0; i < size; i++) {
		uid[i] = receiveSignal(0x09);
	}
	return(UIDtoCard(uid));
}

char scan4tag(void) {
	char size = 0;
	char data = 0;
	data = receiveSignal(0x0E);
	sendSignal(0x0E, data & (~0x80));
	//data = receiveSignal(0x18);
	sendSignal(0x18, ((data & 0x0F) | 0x40));
	sendSignal(0x14, 0x83);
	//sendSignal(0x26, 0x78);
	//data = receiveSignal(0x11);
	//sendSignal(0x11, data | 0x20);
	while (size != 2) {
		size = 0;
		sendSignal(0x01, 0x00);
		sendSignal(0x04, 0x7F);
		data = receiveSignal(0x04);
		sendSignal(0x0A, 0x80);
		sendSignal(0x09, 0x26);
		sendSignal(0x0D, 0x07);
		sendSignal(0x01, 0x0C);
		sendSignal(0x0D, 0x87);
		//data = receiveSignal(0x06);
		for (int i = 0; i < 3000; i++) {
			data = receiveSignal(0x04);
			if (data & 0x20) {
				nano_wait(1000000);
				size = receiveSignal(0x0A);
				break;
			}
		}
	}
	data = 0;
	for(int i = 0; i < size; i++) {
		data += receiveSignal(0x09);
	}
	if (data == 4) {
		return(data);
	}
	return(scan4tag());
}

void selfTest(void) {
	sendSignal(0x01, 0x0F);
	for (int i = 0; i < 25; i++) {
		sendSignal(0x09, 0x00);
	}
	sendSignal(0x01, 0x01);
	sendSignal(0x36, 0x09);
	sendSignal(0x09, 0x00);
	sendSignal(0x01, 0x03);
	for (int n = 0; n < 64; n++) {
		receiveSignal(0x0A);
		receiveSignal(0x09);
	}
}

void initSPI(void) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |= GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_SPE; // DISABLE SPI1
	SPI1->CR1 &= ~(SPI_CR1_CRCEN | SPI_CR1_CRCL | SPI_CR1_CRCNEXT | SPI_CR1_RXONLY | SPI_CR1_RXONLY | SPI_CR1_SSM | SPI_CR1_LSBFIRST | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR);
	SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BR;
	SPI1->CR1 &= ~SPI_CR1_BR_0; // BAUD RATE 375 KHZ
	SPI1->CR2 |= SPI_CR2_DS | SPI_CR2_NSSP | SPI_CR2_SSOE;
	SPI1->CR2 &= ~(SPI_CR2_FRXTH | SPI_CR2_FRF | SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
	SPI1->CR1 |= SPI_CR1_SPE; // ENABLE SPI1
}

void initRFID(void) {
	sendSignal(0x01, 0x0F);
	nano_wait(100000000);
	sendSignal(0x12, 0x00);
	sendSignal(0x13, 0x00);
	sendSignal(0x24, 0x26);
	sendSignal(0x2A, 0x80);
	sendSignal(0x2B, 0xA9);
	sendSignal(0x2C, 0x03);
	sendSignal(0x2D, 0xE8);
	sendSignal(0x15, 0x40);
	sendSignal(0x11, 0x3D);
	int value = receiveSignal(0x14);
	if ((value & 0x03) != 0x03) {
		sendSignal(0x14, value | 0x03);
	}
}

void sendSignal(char address, char signal) {

	nano_wait(10000);

	int transmission = address << 9;
	transmission |= signal;

	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = transmission; // SEND ADDRESS
}

char receiveSignal(char address) {

	//nano_wait(1000000);

	char data;
	int transmission = address << 9;
	transmission |= 0x8000;

	while((SPI1->SR & SPI_SR_RXNE) == SPI_SR_RXNE) {
		data = SPI1->DR;
	}

	while((SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI1->DR = transmission; // SEND ADDRESS

	while((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
	data = SPI1->DR;

	return(data);
}

void nano_wait(int t) {
	asm("       mov r0,%0\n"
			"repeat:\n"
			"       sub r0,#83\n"
			"       bgt repeat\n"
			: : "r"(t) : "r0", "cc");
}
