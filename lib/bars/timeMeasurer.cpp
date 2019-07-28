/*
 * timeMeasurer.cpp
 *
 *  Created on: 12.07.2016
 *      Author: christoph
 */

#include "timeMeasurer.h"

timeMeasurer::timeMeasurer() {
	// TODO Auto-generated constructor stub
	startTime = 0;
	stopTime = 0;
	currentTime = 0;
	duration = 0;
	running = false;
}

timeMeasurer::~timeMeasurer() {
	// TODO Auto-generated destructor stub
}

void timeMeasurer::start(unsigned int duration) {
	this->duration = duration;
	startTime = millis();
	stopTime = startTime + duration;
	running = true;

}

void timeMeasurer::restart() {
	startTime = millis();
	stopTime = startTime + duration;
}

uint8_t timeMeasurer::getRatio() {
	currentTime = millis();
	if(currentTime > stopTime || running == false)
	{
		running = false;
		return 255;

	}
	else{
		return (uint8_t)((float)(currentTime-startTime)/duration)*255;
	}
}
