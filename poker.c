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
		if (games.players[i].gameStatus > -1) {
			games.players[i].gameStatus = 1;
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
		for (int j = 0; j < 4; j++) { // check active players
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
					return;
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
	return(weigh(0));
}

void resolveHand() {
	int winner;
	int ties[5];				// ties[0] represents the number of ties, and ties[1-4] represent a player involved in the tie
	while (games.pot > 0) {
		winner = 0;
		ties[0] = 0;
		for (int i = 0; i < 3; i++) {
			ties[i] = 4;
		}
		for (int i = 0; i < 4; i++) {
			if (winner > 3) {
				break;
			}
			if (games.players[i].gameStatus > 0) {
				if (games.players[i].handStrength > games.players[winner].handStrength) {
					winner = i;
				}
				else if (games.players[i].handStrength == games.players[winner].handStrength) {
					winner = tieBreaker(winner, i, ties);
				}
			}
		}
		if (ties[0] == 0) {
			for (int i = 0; i < 4; i++) {
				if (i != winner) {
					if (games.players[i].currentBet < games.players[winner].currentBet) {
						games.players[winner].holdings += games.players[i].currentBet;
						games.pot -= games.players[i].currentBet;
						games.players[i].currentBet = 0;
					}
					else {
						games.players[winner].holdings += games.players[i].currentBet;
						games.pot -= games.players[i].currentBet;
						games.players[i].currentBet -= games.players[winner].currentBet;
					}
				}
			}
			games.players[winner].currentBet = 0;
			games.players[winner].gameStatus = 0;
		}
		else {
			for (int n = 1; n < ties[0]; n++) {
				for (int i = 0; i < 4; i++) {
					if (i != ties[1] && i != ties[2] && i != ties[3] && i != ties[4]) {
						if (games.players[i].currentBet < games.players[ties[n]].currentBet) {
							games.players[ties[n]].holdings += games.players[i].currentBet;
							games.pot -= games.players[i].currentBet;
							games.players[ties[n]].currentBet = 0;
						}
						else {
							games.players[ties[n]].holdings += games.players[i].currentBet;
							games.pot -= games.players[i].currentBet;
							games.players[i].currentBet -= games.players[ties[n]].currentBet;
						}
					}
				}
				games.players[ties[n]].currentBet = 0;
				games.players[ties[n]].gameStatus = 0;
			}
		}
	}
	return;
}


int tieBreaker(int player1, int player2, int * ties) {
	if (games.players[player1].handStrength == 1) {
		for (int i = 0; i < 5; i++) {
			if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
				return(player1);
			}
			else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
				return(player2);
			}
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 2) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			for (int i = 2; i < 5; i++) {
				if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
					return(player1);
				}
				else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
					return(player2);
				}
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 3) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[2] > games.players[player2].hand[2]) {
				return(player1);
			}
			else if (games.players[player2].hand[2] > games.players[player1].hand[2]) {
				return(player2);
			}
			else {
				if (games.players[player1].hand[5] > games.players[player2].hand[5]) {
					return(player1);
				}
				else if (games.players[player2].hand[5] > games.players[player1].hand[5]) {
					return(player2);
				}
				tieHandler(player1, player2, ties);
				return(player1);
			}
		}
	}
	else if (games.players[player1].handStrength == 4) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			for (int i = 3; i < 5; i++) {
				if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
					return(player1);
				}
				else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
					return(player2);
				}
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 5) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 6) {
		for (int i = 0; i < 5; i++) {
			if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
				return(player1);
			}
			else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
				return(player2);
			}
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 7) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[3] > games.players[player2].hand[3]) {
				return(player1);
			}
			else if (games.players[player2].hand[3] > games.players[player1].hand[3]) {
				return(player2);
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 8) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[4] > games.players[player2].hand[4]) {
				return(player1);
			}
			else if (games.players[player2].hand[4] > games.players[player1].hand[4]) {
				return(player2);
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 9) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 10) {
		tieHandler(player1, player2, ties);
		return(player1);
	}
	return(player1);
}

void tieHandler(int player1, int player2, int * ties) {
	if (ties[0] == 0) {
		ties[0] = 2;
		ties[1] = player1;
		ties[2] = player2;
	}
	else {
		ties[ties[0]] = player2;
		ties[0]++;
	}
}
