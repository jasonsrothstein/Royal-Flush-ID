/*
 * player.h
 *
 *  Created on: Apr 10, 2019
 *      Author: James
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#endif /* PLAYER_H_ */

int getHandStrength5Cards(char * cards);
int getHandStrength6Cards(char * cards);
int getHandStrength7Cards(char * cards, int makeHand);
int bestHand(char * cards, int makeHand);
int checkRoyalFlush(char * cards, int makeHand);
int checkStraightFlush(char * cards, int makeHand);
int checkFourOfAKind(char * cards, int makeHand);
int checkFullHouse(char * cards, int makeHand);
int checkFlush(char * cards, int makeHand);
int checkStraight(char * cards, int makeHand);
int checkThreeOfAKind(char * cards, int makeHand);
int checkTwoPair(char * cards, int makeHand);
int checkPair(char * cards, int makeHand);
void sortCardsLtoG(char * cards);
void sortCardsGtoL(char * cards);
int getHandStrength2Cards(char *);

char hand[7];
char fiveCardHand[5];
int currentHand;
