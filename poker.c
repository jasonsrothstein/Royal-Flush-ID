/*
 * poker.c
 *
 *  Created on: Mar 30, 2019
 *      Author: James
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "poker.h"
#include "LoadCell.h"
#include "rfid.h"

void playGame(void) {
	while (games.numPlayers > 1) {
		dealHands();
		bet(0,0);
		dealToTable(3);
		bet(0,0);
		dealToTable(1);
		bet(0,0);
		dealToTable(1);
		bet(0,0);
		resolveHand();
	}
}

void initGame(void) {
	games.activePlayers = 4;
	games.numPlayers = 4;
	games.pot = 0;
	for (int i = 0; i < 4; i++) {
		games.players[i].currentBet = 0;
		games.players[i].gameStatus = 1;
		if (i == 0) {
			games.players[i].holdings = getStartHoldings();
		}
		else {
			games.players[i].holdings = games.players[0].holdings;
		}
	}
}

void roundReset(void) {
	for (int i = 0; i < 4; i++) {
		games.players[i].currentBet = 0;
		if (games.players[i].gameStatus == 0) {
			games.players[i].gameStatus++;
		}
	}
	games.pot = 0;
	games.activePlayers = games.numPlayers;
}

int getStartHoldings(void) {
	return(50);
}

void dealHands(void) {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 4; j++) {
			if (games.players[j].gameStatus > 0) {
				// Turn on LED
				XBEEsend(games.players[j].address, getCard());
			}
		}
	}
}

void dealToTable(char numCards) {
	for (int i = 0; i < numCards; i++) {
		XBEEsend(1111,getCard());
	}
}

void bet(int start, int playerTurn) {
	for (int i = 0; i < 4; i++) {
		if (games.activePlayers > 1) {
			if (++playerTurn > 3) {
				playerTurn = 0;
			}
			if (games.players[playerTurn].gameStatus == 1) {
				games.players[playerTurn].currentBet = getBet();
				games.players[playerTurn].holdings -= games.players[playerTurn].currentBet;
				if (games.players[playerTurn].currentBet > start) {
					bet(games.players[playerTurn].currentBet, playerTurn);
				}
				if (games.players[playerTurn].currentBet <= start) {
					if (games.players[playerTurn].holdings == 0) {
						games.players[playerTurn].gameStatus = 2;
					}
					else {
						games.players[playerTurn].gameStatus = 0;
						games.activePlayers--;
					}
				}

			}
		}
	}
}

int getBet(void) {
	return(weigh());
}

void resolveHand(int ) {
	int winner;
	while (games.pot > 0) {
		winner = 0;
		for (int i = 0; i < 4; i++) {
			if (games.players[i].handStrength > games.players[winner].handStrength) {
				return;
			}
		}
	}
	return;
}



