/*
 * player.h
 *
 *  Created on: Apr 10, 2019
 *      Author: James
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#endif /* PLAYER_H_ */

int getHandStrengthC(char * cards);
int getHandStrengthB(char * cards);
int getHandStrength(char * cards);
int bestHand(char * cards);
int checkRoyalFlush(char * cards);
int checkStraightFlush(char * cards);
int checkFourOfAKind(char * cards);
int checkFullHouse(char * cards);
int checkFlush(char * cards);
int checkStraight(char * cards);
int checkThreeOfAKind(char * cards);
int checkTwoPair(char * cards);
int checkPair(char * cards);
void sortCards(char * cards);

