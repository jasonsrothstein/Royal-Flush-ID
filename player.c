
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "player.h"

int getHandStrengthC(char * cards) {
	double average = 0;
	int sum = 0;
	int place = 0;
	for (int i = 8; i < 60; i++) {
		if (cards[place] == i) {
			place++;
		}
		else {
			cards[5] = i;
			sum += getHandStrengthB(cards);
		}
	}
	average = (double) sum / 46;
	if (average - (int) average > .5) {
		average++;
	}
	return( (int) average);
}

int getHandStrengthB(char * cards) {
	double average = 0;
	int sum = 0;
	int place = 0;
	char tempCards[7];
	for (int i = 8; i < 60; i++) {
		if (cards[place] == i || cards[6] == i) {
			place++;
		}
		else {
			for (int n = 0; n < 6; n++) {
				tempCards[n] = cards[n];
			}
			tempCards[6] = i;
			sortCards(tempCards);
			sum += getHandStrength(tempCards);
		}
	}
	average = (double) sum / 46;
	if (average - (int) average > .5) {
		average++;
	}
	return( (int) average);
}

int getHandStrength(char * cards) {
	int hand = bestHand(cards);
	if (hand > 7) {
		return(10);
	}
	else if (hand > 6) {
		return(9);
	}
	else if (hand > 5) {
		return(7);
	}
	else if (hand > 4) {
		return(6);
	}
	else if (hand > 3) {
		return(5);
	}
	else if (hand > 2) {
		return(3);
	}
	else if (hand > 1) {
		return(1);
	}
	else {
		return(0);
	}
}

int bestHand(char * cards) {
	if (checkRoyalFlush(cards)) {
		return(10);
	}
	else if (checkStraightFlush(cards)) {
		return(9);
	}
	else if(checkFourOfAKind(cards)) {
		return(8);
	}
	else if(checkFullHouse(cards)) {
		return(7);
	}
	else if(checkFlush(cards)) {
		return(6);
	}
	else if(checkStraight(cards)) {
		return(5);
	}
	else if(checkThreeOfAKind(cards)) {
		return(4);
	}
	else if(checkTwoPair(cards)) {
		return(3);
	}
	else if(checkPair(cards)) {
		return(2);
	}
	else {
		return(1);
	}
}

int checkRoyalFlush(char * cards) {
	if (!checkStraightFlush(cards)) {
		return(0);
	}
	if ((cards[6] / 4) != 14) {
		return(0);
	}
	int numTens = 0;
	int firstTen = 0;
	for (int i = 0; i < 7; i++) {
		if ((cards[i] / 4) == 10) {
			if (numTens == 0) {
				firstTen = i;
			}
			numTens++;
		}
	}
	if (numTens == 0) {
		return(0);
	}
	int current;
	int suit;
	int num;
	for (int i = 0; i < numTens; i++) {
		current = 10;
		suit = cards[firstTen+i] % 4;
		num = 1;
		for (int n = firstTen + i; n < 7; n++) {
			if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
				num++;
				current++;
			}
		}
		if (num > 4) {
			return(1);
		}
	}
	return(0);
}

int checkStraightFlush(char * cards) {
	if (!(checkFlush(cards) && checkStraight(cards))) {
		return(0);
	}
	int num;
	int suit;
	int current;
	for (int i = 0; i < 3; i++) {
		current = cards[i] / 4;
		suit = cards[i] % 4;
		num = 1;
		for (int n = i + 1; n < 7; n ++) {
			if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
				num++;
				current++;
			}
			if (num > 4) {
				return(1);
			}
		}
	}
	return(0);
}

int checkFourOfAKind(char * cards) {
	int num;
	for (int i = 0; i < 6; i++) {
		num = 1;
		for (int n = i + 1; n < 7; n++) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				num++;
			}
		}
		if (num > 3) {
			return(1);
		}
	}
	return(0);
}

int checkFullHouse(char * cards) {
	if (checkTwoPair(cards) && checkThreeOfAKind(cards)) {
		return(1);
	}
	return(0);
}

int checkFlush(char * cards) {
	int num;
	for (int i = 0; i < 4; i++) {
		num = 0;
		for (int n = 0; n < 7; n++) {
			if ((cards[n] % 4) == i) {
				num++;
			}
		}
		if (num > 4) {
			return(1);
		}
	}
	return(0);
}

int checkStraight(char * cards) {
	int num = 1;
	for (int i = 0; i < 6; i++) {
		if (((cards[i] / 4) + 1) == (cards[i+1] / 4)) {
			num++;
		}
		else if ((cards[i] / 4) == (cards[i+1] / 4)) {}
		else {
			if (num > 4) {
				return(1);
			}
			else if (i > 1) {
				return(0);
			}
			else {
				num = 1;
			}
		}
	}
	if (num > 4) {
		return(1);
	}
	return(0);
}

int checkThreeOfAKind(char * cards) {
	int num;
	for (int i = 0; i < 6; i++) {
		num = 1;
		for (int n = i + 1; n < 7; n++) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				num++;
			}
		}
		if (num > 2) {
			return(1);
		}
	}
	return(0);
}

int checkTwoPair(char * cards) {
	int pairs = 0;
	for (int i = 0; i < 6; i++) {
		for (int n = i + 1; n < 7; n++) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				pairs++;
				if (pairs > 1) {
					return(1);
				}
				while ((cards[i] / 4) == (cards[n] / 4) && n < 7) {
					i++;
					n++;
				}
				break;
			}
		}
	}
	return(0);
}

int checkPair(char * cards) {
	for (int i = 0; i < 6; i++) {
		for (int n = i + 1; n < 7; n++) {
			if (cards[i] / 4 == cards[n] / 4) {
				return(1);
			}
		}
	}
	return(0);
}

void sortCards(char * cards) {
	int temp;
	int place;
	for (int i = 0; i < 6; i++) {
		temp = cards[i];
		place = i;
		for (int n = i; n < 7; n++) {
			if (cards[n] < cards[i]) {
				cards[i] = cards[n];
				place = n;
			}
		}
		cards[place] = temp;
	}
}
