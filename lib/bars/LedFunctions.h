/*
 * LedFunctions.h
 *
 *  Created on: 12.07.2016
 *      Author: christoph
 */
#include "Arduino.h"
#include "timeMeasurer.h"
#ifndef LEDFUNCTIONS_H_
#define LEDFUNCTIONS_H_

extern timeMeasurer mess;
//helping functions
unsigned int lin(unsigned int startx, unsigned int starty, unsigned int stopx, unsigned int stopy, unsigned int x );
unsigned int exp(unsigned int startx, unsigned int starty, unsigned int stopx, unsigned int stopy, unsigned int x );





#endif /* LEDFUNCTIONS_H_ */
