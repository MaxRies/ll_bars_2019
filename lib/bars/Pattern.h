/*
 * Pattern.h
 *
 *  Created on: 23.08.2016
 *      Author: christoph
 */

#ifndef PATTERN_H_
#define PATTERN_H_
#include "Arduino.h"
#include <FastLED.h>
#include <EEPROM.h>
#include <math.h>

#define OFFSET_COMP_BACK_1 0
#define OFFSET_COMP_BACK_2 14
#define OFFSET_COMP_BACK_3 30
#define OFFSET_COMP_BACK_4 45
#define OFFSET_COMP_BACK_5 60

#define LENGTH_COMP_BACK_1 14
#define LENGTH_COMP_BACK_2 15
#define LENGTH_COMP_BACK_3 15
#define LENGTH_COMP_BACK_4 15
#define LENGTH_COMP_BACK_5 13

#define OFFSET_COMP_FRONT_1 74
#define OFFSET_COMP_FRONT_2 88
#define OFFSET_COMP_FRONT_3 103
#define OFFSET_COMP_FRONT_4 118
#define OFFSET_COMP_FRONT_5 134

#define LENGTH_COMP_FRONT_1 14
#define LENGTH_COMP_FRONT_2 15
#define LENGTH_COMP_FRONT_3 15
#define LENGTH_COMP_FRONT_4 16
#define LENGTH_COMP_FRONT_5 14

#define INPUT_MAX_VALUE 65280‬
#define INPUT_MIN_VALUE 0

#define BEATS_TO_SWITCH 4

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_MSG(...)
#endif

#define BALL_COUNT 10
struct ball
{
	uint8_t pos;
	CRGB color;
	bool active;

	ball()
	{
		pos = 0;
		color = CRGB::Black;
		active = false;
	}
};

struct valsToSave
{
	double nBaseDim;
	double nBaseSpeed;
	double nFrontDim;
	double nFrontSpeed;
	double nStrobeDim;
	double nStrobeSpeed;
	bool pristine0;
	bool pristine1;
};

class Pattern
{

	double millisSinceBeat;
	double beatPeriodMillis;
	double beatDistinctiveness;

	double nbasePattern;
	double nbaseColor;
	double nbaseSpeed;
	double nbaseDim;
	double nfrontPattern;
	double nfrontColor;
	double nfrontSpeed;
	double nfrontDim;
	double nstrobePattern;
	double nstrobeColor;
	double nstrobeSpeed;
	double nstrobeDim;
	double dimVal;
	double dutyCycle;

	uint8_t strobeStep;
	double lastcycle;
	double color;

	bool first;
	bool onRand;
	uint comp;
	ball balls[BALL_COUNT];
	long laststep;
	double step;

	bool first_strobe;
	long strobe_start;
	long strobe_time;
	uint8_t strobe_comp;
	uint32_t strobecounter;

	size_t length;
	size_t side_length;

	CRGB baseColor;
	CRGB frontColor;
	CRGB strobeColor;

	CRGB *leds;

	CRGB *backleds;
	CRGB *frontleds;

	CRGB *combartmentback1;
	CRGB *combartmentback2;
	CRGB *combartmentback3;
	CRGB *combartmentback4;
	CRGB *compartmentback5;

	long now;
	long lastShowTime;

	const int patternCombinations[104][3] = {{0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0}, {0, 4, 0}, {0, 5, 0}, {0, 6, 0}, {0, 7, 0}, {0, 8, 0}, {0, 9, 0}, {1, 0, 0}, {1, 1, 0}, {1, 2, 0}, {1, 3, 0}, {1, 4, 0}, {1, 5, 0}, {1, 6, 0}, {1, 7, 0}, {1, 8, 0}, {1, 9, 0}, {2, 0, 0}, {2, 1, 0}, {2, 2, 0}, {2, 3, 0}, {2, 4, 0}, {2, 5, 0}, {2, 6, 0}, {2, 7, 0}, {2, 8, 0}, {2, 9, 0}, {3, 0, 0}, {3, 1, 0}, {3, 2, 0}, {3, 3, 0}, {3, 4, 0}, {3, 5, 0}, {3, 6, 0}, {3, 7, 0}, {3, 8, 0}, {3, 9, 0}, {4, 0, 0}, {4, 1, 0}, {4, 2, 0}, {4, 3, 0}, {4, 4, 0}, {4, 5, 0}, {4, 6, 0}, {4, 7, 0}, {4, 8, 0}, {4, 9, 0}, {5, 0, 0}, {5, 1, 0}, {5, 2, 0}, {5, 3, 0}, {5, 4, 0}, {5, 5, 0}, {5, 6, 0}, {5, 7, 0}, {5, 8, 0}, {5, 9, 0}, {6, 0, 0}, {6, 1, 0}, {6, 2, 0}, {6, 3, 0}, {6, 4, 0}, {6, 5, 0}, {6, 6, 0}, {6, 7, 0}, {6, 8, 0}, {6, 9, 0}, {7, 0, 0}, {7, 1, 0}, {7, 2, 0}, {7, 3, 0}, {7, 4, 0}, {7, 5, 0}, {7, 6, 0}, {7, 7, 0}, {7, 8, 0}, {7, 9, 0}, {8, 0, 0}, {8, 1, 0}, {8, 2, 0}, {8, 3, 0}, {8, 4, 0}, {8, 5, 0}, {8, 6, 0}, {8, 7, 0}, {8, 8, 0}, {8, 9, 0}, {9, 0, 0}, {9, 1, 0}, {9, 2, 0}, {9, 3, 0}, {9, 4, 0}, {9, 5, 0}, {9, 6, 0}, {9, 7, 0}, {9, 8, 0}, {9, 9, 0}, {0, 0, 1}, {0, 0, 2}, {0, 0, 3}, {0, 0, 4}};
	const int colorCombinations[100][2] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}, {4, 9}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7}, {6, 8}, {6, 9}, {7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7}, {7, 8}, {7, 9}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {8, 6}, {8, 7}, {8, 8}, {8, 9}, {9, 0}, {9, 1}, {9, 2}, {9, 3}, {9, 4}, {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9}};
	const int speedCombinations[27][3] = {{42, 42, 42}, {42, 42, 127}, {42, 42, 212}, {42, 127, 42}, {42, 127, 127}, {42, 127, 212}, {42, 212, 42}, {42, 212, 127}, {42, 212, 212}, {127, 42, 42}, {127, 42, 127}, {127, 42, 212}, {127, 127, 42}, {127, 127, 127}, {127, 127, 212}, {127, 212, 42}, {127, 212, 127}, {127, 212, 212}, {212, 42, 42}, {212, 42, 127}, {212, 42, 212}, {212, 127, 42}, {212, 127, 127}, {212, 127, 212}, {212, 212, 42}, {212, 212, 127}, {212, 212, 212}};

	valsToSave saveVals;

	int group;
	int position;
	int maxGroup;
	int maxPosition;

	int currentActivePosition;
	int currentActiveGroup;

	int beatCounter;

public:
	Pattern(CRGB *leds, size_t length);
	virtual ~Pattern();

	bool animationActive;

	//helper
	double linearApp(double amp1, double amp2, double deltax, double x);
	double quadApp(double amp1, double amp2, double deltax, double x);
	CRGB colors(int color);
	CRGB dimByVal(CRGB &color, double Value);

	void fillCompartmentBack(CRGB color, int num);
	void fillCompartmentFront(CRGB color, int num);
	void fillCompartementOneRand(CRGB color, int num);

	//pattern funcitons
	void switchthrough();

	void baseLinDimm();
	void baseQuadDimm();
	void baseRectDimm();
	void baseQuadDimmRand50();
	void baseStatic();
	void baseCompartements();

	void frontBallUp();
	void frontBallDown();
	void frontBallIn();
	void frontBallOut();

	void frontRand1();
	void frontRand2();
	void frontRand3();

	void getMillis();

	void strobeStandard();
	void strobeRand();
	void strobeHalf();
	void strobePow();

	void fillWhite();
	void fillBlack();

	void setSettings();

	void frontChoser();
	void baseChoser();
	void strobeChoser();

	void patternChooser(int number);
	void colorChooser(int number);
	void speedChooser(int number);

	void newBeat();
	void nextPosition();
	void nextGroup();

	void baseCompartmentUp();
	void baseCompartmentDown();

	void frontCompartmentUp();
	void frontCompartmentDown();

	void groupBall();

	void cycleGroup();
	void cycleBaseCompartments();

	void ballAFAP();
	void strobeAFAP();

	void setNow(long newNow)
	{
		this->now = newNow;
	}

	void setLastShowTime(long newLastShowTime)
	{
		this->lastShowTime = newLastShowTime;
	}

	const CRGB &getBaseColor() const
	{
		return baseColor;
	}

	void setBaseColor(const CRGB &baseColor)
	{
		this->baseColor = baseColor;
	}

	double getBeatDistinctiveness() const
	{
		return beatDistinctiveness;
	}

	void setBeatDistinctiveness(double beatDistinctiveness)
	{
		this->beatDistinctiveness = beatDistinctiveness;
	}

	double getBeatPeriodMillis() const
	{
		return beatPeriodMillis;
	}

	void setBeatPeriodMillis(double beatPeriodMillis)
	{
		this->beatPeriodMillis = beatPeriodMillis;
	}

	double getColor() const
	{
		return color;
	}

	void setColor(double color)
	{
		this->color = color;
	}

	double getDimVal() const
	{
		return dimVal;
	}

	void setDimVal(double dimVal)
	{
		this->dimVal = dimVal;
	}

	double getDutyCycle() const
	{
		return dutyCycle;
	}

	void setDutyCycle(double dutyCycle)
	{
		this->dutyCycle = dutyCycle;
	}

	const CRGB &getFrontColor() const
	{
		return frontColor;
	}

	void setFrontColor(const CRGB &frontColor)
	{
		this->frontColor = frontColor;
	}

	size_t getLength() const
	{
		return length;
	}

	void setLength(size_t length)
	{
		this->length = length;
	}

	double getMillisSinceBeat() const
	{
		return millisSinceBeat;
	}

	void setMillisSinceBeat(double millisSinceBeat)
	{
		this->millisSinceBeat = millisSinceBeat;
	}

	const CRGB &getStrobeColor() const
	{
		return strobeColor;
	}

	double getNbaseColor() const
	{
		return nbaseColor;
	}

	void setNbaseColor(double nbaseColor)
	{
		this->nbaseColor = nbaseColor;
	}

	double getNbasePattern() const
	{
		return nbasePattern;
	}

	void setNbasePattern(double nbasePattern)
	{
		this->nbasePattern = nbasePattern;
	}

	double getNfrontColor() const
	{
		return nfrontColor;
	}

	void setNfrontColor(double nfrontColor)
	{
		this->nfrontColor = nfrontColor;
	}

	double getNfrontPattern() const
	{
		return nfrontPattern;
	}

	void setNfrontPattern(double nfrontPattern)
	{
		this->nfrontPattern = nfrontPattern;
	}

	double getNstrobePattern() const
	{
		return nstrobePattern;
	}

	void setNstrobePattern(double nstrobePattern)
	{
		this->nstrobePattern = nstrobePattern;
	}

	double getNstrobeSpeed() const
	{
		return nstrobeSpeed;
	}

	void setNstrobeSpeed(double nstrobeSpeed)
	{
		this->nstrobeSpeed = nstrobeSpeed;
	}

	void setStrobeColor(const CRGB &strobeColor)
	{
		this->strobeColor = strobeColor;
	}

	bool isFirst() const
	{
		return first;
	}

	void setFirst(bool first)
	{
		this->first = first;
	}

	double getLastcycle() const
	{
		return lastcycle;
	}

	void setLastcycle(double lastcycle)
	{
		this->lastcycle = lastcycle;
	}

	long getLaststep() const
	{
		return laststep;
	}

	void setLaststep(long laststep)
	{
		this->laststep = laststep;
	}

	double getNbaseDim() const
	{
		return nbaseDim;
	}

	void setNbaseDim(double nbaseDim)
	{
		this->nbaseDim = nbaseDim;
	}

	double getNbaseSpeed() const
	{
		return nbaseSpeed;
	}

	void setNbaseSpeed(double nbaseSpeed)
	{
		this->nbaseSpeed = nbaseSpeed;
	}

	double getNfrontDim() const
	{
		return nfrontDim;
	}

	void setNfrontDim(double nfrontDim)
	{
		this->nfrontDim = nfrontDim;
	}

	double getNfrontSpeed() const
	{
		return nfrontSpeed;
	}

	void setNfrontSpeed(double nfrontSpeed)
	{
		this->nfrontSpeed = nfrontSpeed;
	}

	double getNstrobeColor() const
	{
		return nstrobeColor;
	}

	void setNstrobeColor(double nstrobeColor)
	{
		this->nstrobeColor = nstrobeColor;
	}

	double getNstrobeDim() const
	{
		return nstrobeDim;
	}

	void setNstrobeDim(double nstrobeDim)
	{
		this->nstrobeDim = nstrobeDim;
	}

	bool isOnRand() const
	{
		return onRand;
	}

	void setOnRand(bool onRand)
	{
		this->onRand = onRand;
	}

	double getStep() const
	{
		return step;
	}

	void setStep(double step)
	{
		this->step = step;
	}

	uint32_t getStrobecounter() const
	{
		return strobecounter;
	}

	void setStrobecounter(uint32_t strobecounter)
	{
		this->strobecounter = strobecounter;
	}

	uint8_t getStrobeStep() const
	{
		return strobeStep;
	}

	void setStrobeStep(uint8_t strobeStep)
	{
		this->strobeStep = strobeStep;
	}

	bool isFirstStrobe() const
	{
		return first_strobe;
	}

	void setFirstStrobe(bool firstStrobe)
	{
		first_strobe = firstStrobe;
	}

	long getStrobeStart() const
	{
		return strobe_start;
	}

	void setStrobeStart(long strobeStart)
	{
		strobe_start = strobeStart;
	}

	long getStrobeTime() const
	{
		return strobe_time;
	}

	void setStrobeTime(long strobeTime)
	{
		strobe_time = strobeTime;
	}

	void setGroup(int newGroup)
	{
		this->group = newGroup;
	}

	int getGroup()
	{
		return this->group;
	}

	void setPosition(int newPosition)
	{
		this->position = newPosition;
	}

	int getPosition()
	{
		return this->position;
	}

	void setMaxGroupNumber(int newMaxGroup)
	{
		this->maxGroup = newMaxGroup;
	}

	void setMaxPositionNumber(int newMaxPos)
	{
		this->maxPosition = newMaxPos;
	}

	bool canWeUpdate()
	{
		return (this->now - this->lastShowTime > 5);
	}
};

#endif /* PATTERN_H_ */
