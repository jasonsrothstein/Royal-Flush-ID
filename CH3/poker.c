/*
 * poker.c
 *
 *  Created on: Apr 26, 2019
 *      Author: cunnin66
 */

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
#include "xbee.h"
#include "home.h"

uint8_t BROADCAST = 0;


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

void initGame(int num_players, int init_bank) {
	games.numPlayers = num_players;
	games.activePlayers = num_players;

	games.pot = 0;
	for (int i = 0; i < num_players; i++) {
		games.players[i].currentBet = 0;
		games.players[i].gameStatus = 1;

		//games.players[i].holdings = getStartHoldings();
		games.players[i].holdings = init_bank;
		games.players[i].address = 1 << i;
		BROADCAST |= 1<<i; //dictate who all is in


	}

	//set all to active
	int status = 1;
	xbee_send(BROADCAST, 4, &status, 1);
	//send init bank info to all
	nano_wait(500*1000*1000);
	xbee_send(BROADCAST, 9, &init_bank, 1);
	nano_wait(500*1000*1000);

	games.dealer = -1;
	roundReset();
}

void roundReset(void) {
	//notify players of new round
	xbee_sendX((BROADCAST<<4) | 12);
	nano_wait(500*1000*1000);

	for (int i = 0; i < games.numPlayers; i++) {
		games.players[i].currentBet = 0;
		games.players[i].handStrength = 0;
		//hand resets?
		if (games.players[i].gameStatus > -1) {
			games.players[i].gameStatus = 1;
		}
	}
	games.pot = 0;
	games.activePlayers = games.numPlayers;
	games.dealer = (games.dealer + 1) % games.numPlayers; //advance dealer
	PREVIOUS = 0;
	BLIND_INDEX = games.dealer;
	//notify dealer
	int status = 3;
	xbee_send(games.players[games.dealer].address, 4, &status, 1);
	nano_wait(500*1000*1000);
}


void dealHands() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < games.activePlayers; j++) { // check active players
			//Brian-- you'll see me use this mod thing
			//its to ensure we start the deal from the left of the dealer
			led(3,0);
			int index = (games.dealer + 1 + j)%games.numPlayers;
			if (games.players[index].gameStatus > 0) {
				// Turn on LED
				int new_card = getCard();
				led(3,1);
				clear_line(3);
				display(3, "(GOOD)", 'c');
				xbee_send(games.players[index].address, 5, &new_card, 1);
				nano_wait(1000*1000*1000);
				choice_update(1); //advance screen
			}
		}
	}
}

void dealToTable(char numCards) {
	for (int i = 0; i < numCards; i++) {
		led(3,0);
		int new_card = getCard();
		led(3,1);
		//send to everybody
		xbee_send(BROADCAST,5, &new_card, 1);
		nano_wait(1000*1000*1000);
	}
}

int blind(int turn, int type) {
	int index = (turn + 1) % games.activePlayers;
	while(games.players[index].gameStatus != 1) { //advance to first active player
		index = (index+1)%games.activePlayers;
	}
	int status;
	if(type == 0 ) { //little blind
		status = 4;
		//call update
		xbee_send(games.players[index].address, 7, (BLINDS), 1);
		nano_wait(500*1000*1000);
	} else { // big blind
		status = 5;
		//call update
		xbee_send(games.players[index].address, 7, (BLINDS+1), 1);
		nano_wait(500*1000*1000);
	}
	//activate player
	xbee_send(games.players[index].address, 4, &status, 1);
	nano_wait(500*1000*1000);


	int bet_val = 0;
	while(((USART2->ISR & USART_ISR_RXNE) != USART_ISR_RXNE)) { //wait for xbee command back
		//POTENTIAL ERROR: does not check what the message is
		bet_val = getBet() - PREVIOUS;
		//update player with current bet
		xbee_send(games.players[index].address, 8, &bet_val, 1);
		nano_wait(500*1000*1000); //give the LCD some time to catch up
	}
	char response = xbee_read();
	games.players[index].currentBet += bet_val;
	PREVIOUS += bet_val; //set the offset
	games.players[index].holdings -= bet_val; //games.players[playerTurn].currentBet;
	//notify all of pot update
	games.pot += bet_val;
	xbee_send(BROADCAST, 6, &(games.pot), 1);
	nano_wait(100*1000*1000);
	if(type ==1){ //send call amount to everyone
		int new_call = games.players[index].currentBet;
		xbee_send(BROADCAST, 7, &new_call, 1);
		nano_wait(100*1000*1000);
	}
	return index;
}

void bet(int start, int playerTurn) {
	for (int i = 0; i < games.activePlayers; i++) {
		if (games.activePlayers > 1) {
			if (++playerTurn > games.activePlayers) {
				playerTurn = 0;
			}
			if (games.players[playerTurn].gameStatus == 1) {
				//update screen
				CHOICE = playerTurn;
				choice_update(0);
				//notify player
				int status = 6;
				xbee_send(games.players[playerTurn].address, 4, &status, 1);
				int bet_val = 0;
				while(((USART2->ISR & USART_ISR_RXNE) != USART_ISR_RXNE)) { //wait for xbee command back
					//POTENTIAL ERROR: does not check what the message is
					bet_val = getBet() - PREVIOUS;
					//update player with current bet
					xbee_send(games.players[playerTurn].address, 8, &bet_val, 1);
					nano_wait(100*1000*1000); //give the LCD some time to catch up
				}
				//read what the PM sent back
				char response = xbee_read();
				if(response & 0xF) { //fold
					games.players[playerTurn].gameStatus = 0;
					games.activePlayers--;
				} else {
					games.players[playerTurn].currentBet += bet_val;
					PREVIOUS += bet_val; //set the offset
					games.players[playerTurn].holdings -= bet_val; //games.players[playerTurn].currentBet;
					//notify all of pot update
					games.pot += bet_val;
					xbee_send(BROADCAST, 6, &(games.pot), 1);
					nano_wait(100*1000*1000);
					if (games.players[playerTurn].currentBet > start) {
						//a new raise, notify all via call update
						int new_call = games.players[playerTurn].currentBet;
						xbee_send(BROADCAST, 7, &new_call, 1);
						bet(new_call, playerTurn);
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
}

int getBet(void) {
	return(TRAY_VALS[0]*getChips(0));// + TRAY_VALS[1]*getChips(1) + TRAY_VALS[2]*getChips(2));
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
				if (games.players[i].handStrength > games.players[winner].handStrength) { //edge case- first winner needs to be
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
			//update winner
			xbee_send(games.players[winner].address, 11, &(games.players[winner].holdings), 1);
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
				//send command for split ties
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
