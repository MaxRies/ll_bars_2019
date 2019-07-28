/*
 * timeMeasurer.h
 *
 *  Created on: 12.07.2016
 *      Author: christoph
 */
#ifndef TIMEMEASURER_H_
#define TIMEMEASURER_H_
#include "Arduino.h"
class timeMeasurer {

	long startTime;
	long currentTime;
	long stopTime;
	unsigned int duration;

public:
	bool running;

	timeMeasurer();
	virtual ~timeMeasurer();

	void start(unsigned int duration);
	void restart();
	uint8_t getRatio();

};

#endif /* TIMEMEASURER_H_ */
