/*
 * poker.h
 *
 *  Created on: Mar 30, 2019
 *      Author: James
 */

#ifndef POKER_H_
#define POKER_H_

#endif /* POKER_H_ */

typedef struct {
	int holdings;		// Value in chips remaining
	int currentBet;		// Current value of chips in the pot
	char gameStatus;	// -1 - Bankrupt   0 - Folded   1 - Active 	2 - All In
	char address;		// The address that the XBEE chip in the player module will respond to
	int handStrength;	// The strength of the player hand
	char hand[5];		// Player hand
} player;

typedef struct {
	player players[4];	// Players at table
	int pot;			// Total amount of money that has been bet in a hand
	int dealer;			// The player who is dealer of the current hand
	int activePlayers;	// The number of players who have not folded
	int numPlayers;		// The number of players with nonzero holdings
} game;

game games;



void initGame(int, int);
void roundReset(void);
int getStartHoldings(void);
void bet(int start, int playerTurn);
int getBet(void);
void dealHands(void);
void dealToTable(char numCards);
void XBEEsend(int address, int data);
void dealHands(void);
void resolveHand(void);
int tieBreaker(int player1, int player2, int * ties);
void tieHandler(int player1, int player2, int * ties);
