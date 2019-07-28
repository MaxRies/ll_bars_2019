#include "LedFunctions.h"
#include <math.h>

unsigned int lin(unsigned int startx, unsigned int starty, unsigned int stopx,
		unsigned int stopy, unsigned int x) {

	float Steigung = (float)(stopy-starty)/(stopx-startx);
	unsigned int result = (unsigned int)( Steigung * (x - startx) + starty);
	return result;
}

unsigned int exp(unsigned int startx, unsigned int starty, unsigned int stopx,
		unsigned int stopy, unsigned int x) {
	float Steigung = (float)(stopy-1)* pow(2.718281, (-1) * stopx);
	unsigned int result = (unsigned int) (Steigung * pow(2.718281, (x-startx)) - 1 + starty);
	return result;

}
/*
 * LedFunctions.cpp
 *
 *  Created on: 12.07.2016
 *      Author: christoph
 */



