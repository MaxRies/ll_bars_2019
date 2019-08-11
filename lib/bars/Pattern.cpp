/*
 * Pattern.cpp
 *
 *  Created on: 23.08.2016
 *      Author: christoph
 */

#include "Pattern.h"

Pattern::Pattern(CRGB *leds, size_t length)
{
	// TODO Auto-generated constructor stub
	this->leds = leds;
	this->length = length;
	this->backleds = leds;
	this->frontleds = leds + length / 2;
	this->side_length = length / 2;

	nbasePattern = 1;
	nbaseColor = 1;
	nbaseSpeed = 125;
	nbaseDim = 225;

	nfrontPattern = 1;
	nfrontColor = 1;
	nfrontSpeed = 125;
	nfrontDim = 120;

	nstrobePattern = 1;
	nstrobeColor = 1;
	nstrobeSpeed = 200;
	nstrobeDim = 200.0;

	dimVal = 255;
	dutyCycle = 1;
	group = 0;
	position = 0;
	maxGroup = 0;
	maxPosition = 0;

	animationActive = false;
}

Pattern::~Pattern()
{
	// TODO Auto-generated destructor stub
}

double Pattern::linearApp(double amp1, double amp2, double deltax, double x)
{
	double a = (((amp2 - amp1) / deltax) * x + amp1);
	if (a < 0)
		a = 0;

	return a;
}

double Pattern::quadApp(double amp1, double amp2, double deltax, double x)
{
	if (amp1 > amp2)
	{
		//cout << "1 ";
		return (double)(amp1 - amp2) / (deltax * deltax) * x * x - (double)2 * (amp1 - amp2) / (deltax)*x + amp1;
	}
	else
	{
		//cout << "2 ";
		return (double)(amp2 - amp1) / (deltax * deltax) * x * x + amp1;
	}
}

void Pattern::baseLinDimm()
{
	if (millisSinceBeat < nbaseSpeed / 255 * beatPeriodMillis)
	{
		CRGB col(baseColor);

		col.r = (unsigned int)(linearApp(col.r, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.g = (unsigned int)(linearApp(col.g, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.b = (unsigned int)(linearApp(col.b, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		fill_solid(backleds, side_length, dimByVal(col, nbaseDim));
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
	}
}

void Pattern::baseQuadDimm()
{
	if (millisSinceBeat < nbaseSpeed / 255 * beatPeriodMillis)
	{
		CRGB col(baseColor);

		col.r = (unsigned int)(quadApp(col.r, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.g = (unsigned int)(quadApp(col.g, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.b = (unsigned int)(quadApp(col.b, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		fill_solid(backleds, side_length, dimByVal(col, nbaseDim));
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
	}
}

void Pattern::baseRectDimm()
{
	if (millisSinceBeat < nbaseSpeed / 255 * beatPeriodMillis)
	{
		fill_solid(this->backleds, side_length, dimByVal(baseColor, nbaseDim));
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
	}
}
void Pattern::baseStatic()
{
	fill_solid(leds, length, dimByVal(baseColor, 120));
}

void Pattern::baseQuadDimmRand50()
{
	if (millisSinceBeat == 0 && first)
	{
		if (rand() > RAND_MAX / 2)
			onRand = true;
		else
			onRand = false;
		first = false;
	}
	if (millisSinceBeat > 0)
	{
		first = true;
	}
	if (millisSinceBeat < nbaseSpeed / 255 * beatPeriodMillis && onRand)
	{
		CRGB col(baseColor);

		col.r = (unsigned int)(quadApp(col.r, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.g = (unsigned int)(quadApp(col.g, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.b = (unsigned int)(quadApp(col.b, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		fill_solid(backleds, side_length, dimByVal(col, nbaseDim));
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
	}
}

void Pattern::baseCompartements()
{
	if (millisSinceBeat == 0 && first)
	{
		first = false;
		comp = rand() % 4;
	}
	if (millisSinceBeat > 0)
	{
		first = true;
	}

	if (millisSinceBeat < nbaseSpeed / 255 * beatPeriodMillis)
	{
		CRGB col(baseColor);

		col.r = (unsigned int)(quadApp(col.r, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.g = (unsigned int)(quadApp(col.g, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		col.b = (unsigned int)(quadApp(col.b, 0, nbaseSpeed / 255 * beatPeriodMillis, millisSinceBeat));
		fillCompartmentBack(dimByVal(col, nbaseDim), comp);
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
	}
}

void Pattern::frontBallUp()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)side_length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = 0;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				balls[i].pos++;
				if (balls[i].pos < side_length)
				{
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
			}
		}
	}
}

void Pattern::frontBallDown()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)side_length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = side_length - 1;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{

				if (balls[i].pos > 0)
				{
					balls[i].pos--;
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
			}
		}
	}
}

void Pattern::frontBallIn()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = 0;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				balls[i].pos++;
				if (balls[i].pos < length)
				{
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
			}
		}
	}
}

void Pattern::frontBallOut()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = length - 1;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{

				if (balls[i].pos > 0)
				{
					balls[i].pos--;
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
			}
		}
	}
}

void Pattern::frontRand1()
{
	if (millisSinceBeat == 0 && lastcycle != 0)
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			balls[i].active = false;
		}
		for (int i = 0; i < (int)(dutyCycle / 255 * BALL_COUNT); i++)
		{
			balls[i].active = true;
			balls[i].color = frontColor;
			balls[i].pos = rand() % (side_length);
		}
	}
	if (millisSinceBeat < beatPeriodMillis)
	{
		CRGB col(frontColor);

		/*col.r = (unsigned int)quadApp(col.r, 0, dutyCycle/255 * beatPeriodMillis, millisSinceBeat);
		col.g = (unsigned int)quadApp(col.g, 0, dutyCycle/255 * beatPeriodMillis, millisSinceBeat);
		col.b = (unsigned int)quadApp(col.b, 0, dutyCycle/255 * beatPeriodMillis, millisSinceBeat);*/

		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
			}
		}
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active)
			{
				backleds[balls[i].pos] = CRGB::Black;
			}
		}
	}
	lastcycle = millisSinceBeat;
}

void Pattern::frontRand2()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)side_length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = 0;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				balls[i].pos++;
				if (balls[i].pos < side_length)
				{
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
				if (balls[i].pos > 0)
				{
					backleds[balls[i].pos - 1] = dimByVal(balls[i].color, nfrontDim * 0.5);
				}
				if (balls[i].pos > 1)
				{
					backleds[balls[i].pos - 2] = dimByVal(balls[i].color, nfrontDim * 0.1);
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				if (balls[i].pos > 0)
				{
					backleds[balls[i].pos - 1] = dimByVal(balls[i].color, nfrontDim * 0.5);
				}
				if (balls[i].pos > 1)
				{
					backleds[balls[i].pos - 2] = dimByVal(balls[i].color, nfrontDim * 0.1);
				}
			}
		}
	}
}

void Pattern::frontRand3()
{
	step = (double)this->nfrontSpeed / 20.0 * beatPeriodMillis / ((double)side_length);
	if (millisSinceBeat == 0)
	{
		//Serial.print("New");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == false)
			{
				balls[i].active = true;
				balls[i].color = frontColor;
				balls[i].pos = side_length - 1;
				break;
			}
		}

		laststep = millis();
		//Serial.println(step);
	}

	if (millis() - laststep > step)
	{
		//Serial.println("Step");
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{

				if (balls[i].pos > 0)
				{
					balls[i].pos--;
					backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				}
				else
				{
					balls[i].active = false;
				}
				if (balls[i].pos < side_length - 1)
				{
					backleds[balls[i].pos + 1] = dimByVal(balls[i].color, nfrontDim * 0.5);
				}
				if (balls[i].pos < side_length - 2)
				{
					backleds[balls[i].pos + 2] = dimByVal(balls[i].color, nfrontDim * 0.1);
				}
			}
		}
		laststep = millis();
	}
	else
	{
		for (int i = 0; i < BALL_COUNT; i++)
		{
			if (balls[i].active == true)
			{
				backleds[balls[i].pos] = dimByVal(balls[i].color, nfrontDim);
				if (balls[i].pos < side_length - 1)
				{
					backleds[balls[i].pos + 1] = dimByVal(balls[i].color, nfrontDim * 0.5);
				}
				if (balls[i].pos < side_length - 2)
				{
					backleds[balls[i].pos + 2] = dimByVal(balls[i].color, nfrontDim * 0.1);
				}
			}
		}
	}
}

void Pattern::strobeStandard()
{
	//Serial.print("StandardStrobe.\n");
	if (strobe_time % (int)(nstrobeSpeed) < 20)
	{

		fill_solid(leds, length, dimByVal(strobeColor, nstrobeDim));
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
		strobeStep++;
	}
}

void Pattern::strobeRand()
{
	if (millisSinceBeat == 0 && first == true)
	{
		strobe_comp = rand() % 5;
		first = false;
	}
	else
	{
		first = true;
	}
	if (strobe_time % (int)(nstrobeSpeed) < 20)
	{
		fillCompartmentBack(dimByVal(strobeColor, nstrobeDim), strobe_comp);
		fillCompartmentFront(dimByVal(strobeColor, nstrobeDim), strobe_comp);
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
		strobeStep++;
	}
}

void Pattern::strobeHalf()
{
	if (strobe_time % (int)(nstrobeSpeed / 2) == 0 && first == true)
	{
		strobe_comp++;
		first = false;
	}
	else
	{
		first = true;
	}
	if (strobe_time % (int)(nstrobeSpeed) < 20)
	{
		fillCompartmentBack(dimByVal(strobeColor, nstrobeDim), strobe_comp % 5);
		fillCompartmentFront(dimByVal(strobeColor, nstrobeDim), strobe_comp % 5);
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
		strobeStep++;
	}
}

void Pattern::strobePow()
{
	if (strobe_time % (int)(nstrobeSpeed) == 0 && first_strobe)
	{
		for (int i = 0; i < 4; i++)
		{
			balls[i].active = true;
			balls[i].color = dimByVal(strobeColor, nstrobeDim);
			balls[i].pos = rand() % length;
		}
		first_strobe = false;
	}
	else
	{
		first_strobe = true;
	}
	if (strobe_time % (int)(nstrobeSpeed) < 20)
	{
		for (int i = 0; i < 4; i++)
		{
			if (balls[i].active)
			{
				leds[balls[i].pos] = balls[i].color;
			}
		}
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
		strobeStep++;
	}
}

void Pattern::fillWhite()
{
	fill_solid(leds, length, CRGB::White);
}

CRGB Pattern::colors(int color)
{
	CRGB ret;
	switch (color)
	{
	case 1:
		ret = CRGB::White;
		break;

	case 2:
		ret = CRGB::Blue;
		break;

	case 3:
		ret = CRGB::Purple;
		break;

	case 4:
		ret = CRGB::RosyBrown;
		break;

	case 5:
		ret = CRGB::DarkOrchid;
		break;

	case 6:
		ret = CRGB::Tomato;
		break;

	case 7:
		ret = CRGB::Red;
		break;

	case 8:
		ret = CRGB::Sienna;
		break;

	case 9:
		ret = CRGB::Fuchsia;
		break;
	default:
		ret = CRGB::Black;
		break;
	}

	return ret;
}

void Pattern::fillBlack()
{
	fill_solid(leds, length, CRGB::Black);
}

void Pattern::setSettings()
{
	int temp = (int)(this->nbaseColor);
	baseColor = colors(temp);

	temp = (int)(this->nfrontColor);
	frontColor = colors(temp);

	temp = (int)(this->nstrobeColor);
	strobeColor = colors(temp);
}

void Pattern::groupBallUp()
{
	static long startTime;
	static bool animationRunning = false;
	static int animationCounter = 0;
	static int myCounter = 0;
	static long lastStepTime = 0;

	double timeForAnimation = 3000; // normalerweise um die 500
	int groupLength = 3 * 5;
	int stepTime = timeForAnimation / groupLength; // dann etwa 1.29 ms! Ob das reicht...

	int myPartStart = this->position * 5;
	int myPartEnd = (this->position + 1) * 5;
	if (animationActive)
	{
		if (millisSinceBeat == 0)
		{
			if (!animationRunning)
			{
				animationRunning = true;
				animationCounter = 0;
				myCounter = 0;
				lastStepTime = millis();
				fill_solid(leds, length, CRGB::Green);
				DEBUG_MSG("START GROUP ANIMATION\n -----------------------------\n");
			}
		}

		if (animationRunning)
		{
			long now = millis();
			if (now - lastStepTime > stepTime)
			{
				lastStepTime = now;
				animationCounter++;
				if ((animationCounter > myPartStart) && (animationCounter <= myPartEnd))
				{
					fill_solid(leds, length, CRGB::Black);
					fillCompartmentBack(CRGB::Red, myCounter);
					myCounter++;
				}
				else
				{
					fill_solid(leds, length, CRGB::Black);
				}

				if (animationCounter >= groupLength)
				{
					animationRunning = false;
					DEBUG_MSG("END GROUP ANIMATION\n -----------------------------\n");
				}
				DEBUG_MSG("myCounter: %i \t animationCounter: %i\n", myCounter, animationCounter);
			}
		}
	}
	else
	{
		fill_solid(leds, length, CRGB::Black);
		fillCompartmentBack(CRGB::Blue, this->position);
		fillCompartmentBack(CRGB::Red, position);
	}
}

void Pattern::frontChoser()
{
	//int temp = (int)nfrontPattern;
	groupBallUp();
	/*
	switch (temp)
	{
	case 1:
		frontBallUp();
		break;
	case 2:
		frontBallDown();
		break;
	case 3:
		frontBallIn();
		break;
	case 4:
		frontBallOut();
		break;
	case 5:
		frontRand1();
		break;
	case 6:
		frontRand2();
		break;
	case 7:
		frontRand3();
		break;
	default:
		break;
	}*/
}

void Pattern::baseChoser()
{
	int temp = (int)nbasePattern;
	//Serial.println(temp);
	switch (temp)
	{
	case 1:
		baseRectDimm();
		break;
	case 2:
		baseLinDimm();
		break;
	case 3:
		baseQuadDimm();
		break;
	case 4:
		baseQuadDimmRand50();
		break;
	case 5:
		baseCompartements();
		break;
	case 6:
		baseStatic();
		break;
	default:
		fillBlack();
		break;
	}
}

void Pattern::fillCompartmentBack(CRGB color, int num)
{
	switch (num)
	{
	case 0:
		fill_solid(leds + OFFSET_COMP_BACK_1, LENGTH_COMP_BACK_1, color);
		break;
	case 1:
		fill_solid(leds + OFFSET_COMP_BACK_2, LENGTH_COMP_BACK_2, color);
		break;
	case 2:
		fill_solid(leds + OFFSET_COMP_BACK_3, LENGTH_COMP_BACK_3, color);
		break;
	case 3:
		fill_solid(leds + OFFSET_COMP_BACK_4, LENGTH_COMP_BACK_4, color);
		break;
	case 4:
		fill_solid(leds + OFFSET_COMP_BACK_5, LENGTH_COMP_BACK_5, color);
		break;
	}
}

void Pattern::fillCompartmentFront(CRGB color, int num)
{
	switch (num)
	{
	case 0:
		fill_solid(leds + OFFSET_COMP_FRONT_1, LENGTH_COMP_FRONT_1, color);
		break;
	case 1:
		fill_solid(leds + OFFSET_COMP_FRONT_2, LENGTH_COMP_FRONT_2, color);
		break;
	case 2:
		fill_solid(leds + OFFSET_COMP_FRONT_3, LENGTH_COMP_FRONT_3, color);
		break;
	case 3:
		fill_solid(leds + OFFSET_COMP_FRONT_4, LENGTH_COMP_FRONT_4, color);
		break;
	case 4:
		fill_solid(leds + OFFSET_COMP_FRONT_5, LENGTH_COMP_FRONT_5, color);
		break;
	}
}
void Pattern::fillCompartementOneRand(CRGB color, int num)
{
	switch (num)
	{
	case 0:
		fill_solid(leds + OFFSET_COMP_BACK_1 + rand() % LENGTH_COMP_BACK_1, 1, color);
		break;
	case 1:
		fill_solid(leds + OFFSET_COMP_BACK_2 + rand() % LENGTH_COMP_BACK_2, 1, color);
		break;
	case 2:
		fill_solid(leds + OFFSET_COMP_BACK_3 + rand() % LENGTH_COMP_BACK_3, 1, color);
		break;
	case 3:
		fill_solid(leds + OFFSET_COMP_BACK_4 + rand() % LENGTH_COMP_BACK_4, 1, color);
		break;
	case 4:
		fill_solid(leds + OFFSET_COMP_BACK_5 + rand() % LENGTH_COMP_BACK_5, 1, color);
		break;
	}
}

void Pattern::strobeChoser()
{
	//Serial.printf("StrobePattern %i\n", (int)nstrobePattern);
	int temp = (int)nstrobePattern;
	if (temp >= 1)
	{
		fillBlack();
	}

	//Serial.println(temp);
	switch (temp)
	{
	case 1:
		strobeStandard();
		break;
	case 2:
		strobeRand();
		break;
	case 3:
		strobeHalf();
		break;
	case 4:
		strobePow();
		break;
	default:
		break;
	}
}

void Pattern::patternChooser(int number)
{
	/*
		@param number between 0 and 167.
	 */
	if (number < 0)
	{
		number = 0;
	}
	else if (number > 167)
	{
		number = 167;
	}

	nfrontPattern = patternCombinations[number][0];
	nbasePattern = patternCombinations[number][1];
	nstrobePattern = patternCombinations[number][2];
}

void Pattern::colorChooser(int number)
{
	/*
		@param number between 0 and 447
	 */
	if (number < 0)
	{
		number = 0;
	}
	else if (number > 447)
	{
		number = 447;
	}
	nfrontColor = colorCombinations[number][0];
	nbaseColor = colorCombinations[number][1];
	nstrobeColor = colorCombinations[number][2];

	setSettings();
}

CRGB Pattern::dimByVal(CRGB &color, double Value)
{
	/* 
		@param value between 0 and 255
	*/
	CRGB col;
	col.r = Value / 255 * color.r;
	col.g = Value / 255 * color.g;
	col.b = Value / 255 * color.b;

	return col;
}

void Pattern::saveValues()
{

	valsToSave previousValues{
		nbaseDim,
		nbaseSpeed,
		nfrontDim,
		nfrontSpeed,
		nstrobeDim,
		nstrobeSpeed,
		false,
		false};

	EEPROM.begin(100);
	EEPROM.put(0, previousValues);
	EEPROM.commit();
	EEPROM.end();
	DEBUG_MSG("VALUES SAVED!");
}

void Pattern::getValues()
{
	valsToSave retrieved;
	EEPROM.begin(100);
	EEPROM.get(0, retrieved);
	if (retrieved.pristine0)
	{
		DEBUG_MSG("FLASH PRISTINE, NO VALUES READ!");
		EEPROM.end();
	}
	else if (retrieved.pristine1)
	{
		DEBUG_MSG("FLASH PRISTINE, NO VALUES READ!");
		EEPROM.end();
	}
	else
	{
		nbaseDim = retrieved.nBaseDim;
		nbaseSpeed = retrieved.nBaseSpeed;
		nfrontDim = retrieved.nFrontDim;
		nfrontSpeed = retrieved.nFrontSpeed;
		nstrobeDim = retrieved.nStrobeDim;
		nstrobeSpeed = retrieved.nStrobeSpeed;
		DEBUG_MSG("FLASH VALUES RETRIEVED!");
	}
}

void Pattern::nextPosition()
{
	currentActivePosition++;
	if (currentActivePosition > maxPosition)
	{
		currentActivePosition = 0;
	}
}

void Pattern::nextGroup()
{
	currentActiveGroup++;
	if (currentActiveGroup > maxGroup)
	{
		currentActiveGroup = 0;
	}
}

void Pattern::newBeat()
{
	beatCounter++;
	nextPosition();
	if (beatCounter >= BEATS_TO_SWITCH)
	{
		beatCounter = 0;
		nextGroup();
	}
}
