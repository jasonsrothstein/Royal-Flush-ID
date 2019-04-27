/*
 * LoadCell.h
 *
 *  Created on: Mar 30, 2019
 *      Author: James
 */

#ifndef LOADCELL_H_
#define LOADCELL_H_

#endif /* LOADCELL_H_ */

void initLoadCell(void);
int convert2s(int count);
int weigh(int scale);
void zero(int scale);
int getChips(int scale);
void getChipWeight(int scale);
int findAvg(int scale);
int weigh3(void);

int offset[3];
int chipWeight[3];
int PREVIOUS;
