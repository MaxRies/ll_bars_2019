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

	nstrobePattern = 0;
	nstrobeColor = 1;
	nstrobeSpeed = 100;
	nstrobeDim = 200.0;

	dimVal = 255;
	dutyCycle = 100;
	group = 0;
	position = 0;
	maxGroup = 0;
	maxPosition = 0;

	now = 0;
	lastShowTime = 0;

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
	if (canWeUpdate())
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
	if (canWeUpdate())
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
	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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
	if (canWeUpdate())
	{
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
	if (canWeUpdate())
	{
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

	if (canWeUpdate())
	{
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
}

void Pattern::strobeStandard()
{
	//Serial.print("StandardStrobe.\n");
	if (canWeUpdate())
	{
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
	if (canWeUpdate())
	{
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
	if (canWeUpdate())
	{

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
	if (canWeUpdate())
	{
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
}

void Pattern::fillWhite()
{
	fill_solid(leds, length, CRGB::White);
}

CRGB Pattern::colors(int color)
{
	/*
	Picks a color based on a number. Essentially a shitty implementation of
	a color palette.
	*/
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
	case 10:
		ret = CRGB::ForestGreen;
		break;
	case 11:
		ret = CRGB::Magenta;
		break;
	case 12:
		ret = CRGB::Azure;
		break;
	case 13:
		ret = CRGB::DarkSeaGreen;
		break;
	case 14:
		ret = CRGB::HotPink;
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

void Pattern::ballAFAP()
{
	//static long lastShowTime = 0;
	static int counter = 0;

	if (canWeUpdate())
	{
		//lastShowTime = now;
		fade_raw(leds, length, 20);
		leds[counter] = CRGB::White;
		counter++;
		if (counter >= length)
		{
			counter = 0;
		}
	}
}

void Pattern::strobeAFAP()
{
	static bool on = false;
	if (on)
	{
		fill_solid(leds, length, CRGB::Black);
		on = false;
	}
	else
	{
		fill_solid(leds, length, CRGB::White);
		on = true;
	}
}

void Pattern::baseCompartmentUp()
{
	static long startTime;
	static long lastStepTime;
	static bool animationRunning = false;
	static int animationCounter = 0;
	static int myCounter = 0;
	int beatCounter = 0;

	double timeForAnimation = (4 * nbaseSpeed / 255 + 1) * beatPeriodMillis; // normalerweise um die 500
	int groupLength = (this->maxPosition + 1) * 5;
	int stepTime = timeForAnimation / groupLength; // dann etwa 1.29 ms! Ob das reicht...

	int myPartStart = this->position * 5;
	int myPartEnd = (this->position + 1) * 5;

	if (millisSinceBeat == 0)
	{
		beatCounter++;
		if (beatCounter > 1)
		{
			startTime = millis();
			lastStepTime = startTime;
			animationRunning = true;
			animationCounter = 0;
			myCounter = 0;
			leds[2] = CRGB::Green;
			DEBUG_MSG("START GROUP ANIMATION\n -----------------------------\n");
		}
	}

	if (canWeUpdate())
	{
		fade_raw(leds, length, 10);
		if (now - lastStepTime > stepTime)
		{
			lastStepTime = now;
			animationCounter++;
			if ((animationCounter > myPartStart) && (animationCounter <= myPartEnd))
			{

				fillCompartmentBack(baseColor, myCounter);

				myCounter++;
			}
			else
			{
				fill_solid(leds, length, CRGB::Black);
			}

			if (animationCounter >= groupLength)
			{
				animationCounter = 0;
				myCounter = 0;
				DEBUG_MSG("END GROUP ANIMATION\n -----------------------------\n");
			}
			DEBUG_MSG("myCounter: %i \t animationCounter: %i\n", myCounter, animationCounter);
		}
	}
}

void Pattern::baseCompartmentDown()
{
	static long startTime;
	static long lastStepTime;
	static bool animationRunning = false;
	static int animationCounter = 0;
	static int myCounter = 0;
	int beatCounter = 0;

	double timeForAnimation = (4 * nbaseSpeed / 255 + 1) * beatPeriodMillis; // normalerweise um die 500
	int groupLength = (this->maxPosition + 1) * 5;
	int stepTime = timeForAnimation / groupLength; // dann etwa 1.29 ms! Ob das reicht...

	int myPartStart = (this->maxPosition - this->position) * 5;
	int myPartEnd = ((this->maxPosition - this->position) + 1) * 5;

	if (millisSinceBeat == 0)
	{
		beatCounter++;
		if (beatCounter > 1)
		{
			startTime = millis();
			lastStepTime = startTime;
			animationRunning = true;
			animationCounter = 0;
			myCounter = 0;
			leds[2] = CRGB::Green;
			DEBUG_MSG("START GROUP ANIMATION\n -----------------------------\n");
		}
	}

	if (canWeUpdate())
	{
		fade_raw(leds, length, 10);
		if (now - lastStepTime > stepTime)
		{
			lastStepTime = now;
			animationCounter++;
			if ((animationCounter > myPartStart) && (animationCounter <= myPartEnd))
			{

				fillCompartmentBack(baseColor, 4 - myCounter);

				myCounter++;
			}
			else
			{
				fill_solid(leds, length, CRGB::Black);
			}

			if (animationCounter >= groupLength)
			{
				animationCounter = 0;
				myCounter = 0;
				DEBUG_MSG("END GROUP ANIMATION\n -----------------------------\n");
			}
			DEBUG_MSG("myCounter: %i \t animationCounter: %i\n", myCounter, animationCounter);
		}
	}
}

void Pattern::frontCompartmentDown()
{
	static long startTime;
	static long lastStepTime;
	static int animationCounter = 0;
	static int myCounter = 0;
	int beatCounter = 0;

	double timeForAnimation = (4 * nbaseSpeed / 255 + 1) * beatPeriodMillis; // normalerweise um die 500
	int groupLength = (this->maxPosition + 1) * 5;
	int stepTime = timeForAnimation / groupLength; // dann etwa 1.29 ms! Ob das reicht...

	int myPartStart = this->position * 5;
	int myPartEnd = (this->position + 1) * 5;

	if (millisSinceBeat == 0)
	{
		beatCounter++;
		if (beatCounter > 1)
		{
			startTime = millis();
			lastStepTime = startTime;
			animationCounter = 0;
			myCounter = 0;
			leds[2] = CRGB::Green;
			DEBUG_MSG("START GROUP ANIMATION\n -----------------------------\n");
		}
	}

	if (canWeUpdate())
	{
		fade_raw(leds, length, 10);
		if (now - lastStepTime > stepTime)
		{
			lastStepTime = now;
			animationCounter++;
			if ((animationCounter > myPartStart) && (animationCounter <= myPartEnd))
			{

				fillCompartmentFront(frontColor, myCounter);

				myCounter++;
			}
			else
			{
				fill_solid(leds, length, CRGB::Black);
			}

			if (animationCounter >= groupLength)
			{
				animationCounter = 0;
				myCounter = 0;
				DEBUG_MSG("END GROUP ANIMATION\n -----------------------------\n");
			}
			DEBUG_MSG("myCounter: %i \t animationCounter: %i\n", myCounter, animationCounter);
		}
	}
}

void Pattern::frontCompartmentUp()
{
	static long startTime;
	static long lastStepTime;
	static int animationCounter = 0;
	static int myCounter = 0;
	int beatCounter = 0;

	double timeForAnimation = (4 * nbaseSpeed / 255 + 1) * beatPeriodMillis; // normalerweise um die 500
	int groupLength = (this->maxPosition + 1) * 5;
	int stepTime = timeForAnimation / groupLength; // dann etwa 1.29 ms! Ob das reicht...

	int myPartStart = (this->maxPosition - this->position) * 5;
	int myPartEnd = ((this->maxPosition - this->position) + 1) * 5;

	if (millisSinceBeat == 0)
	{
		beatCounter++;
		if (beatCounter > 1)
		{
			startTime = millis();
			lastStepTime = startTime;
			animationCounter = 0;
			myCounter = 0;
			leds[2] = CRGB::Green;
			DEBUG_MSG("START GROUP ANIMATION\n -----------------------------\n");
		}
	}

	if (canWeUpdate())
	{
		fade_raw(leds, length, 10);
		if (now - lastStepTime > stepTime)
		{
			lastStepTime = now;
			animationCounter++;
			if ((animationCounter > myPartStart) && (animationCounter <= myPartEnd))
			{

				fillCompartmentFront(frontColor, 4 - myCounter);

				myCounter++;
			}
			else
			{
				fill_solid(leds, length, CRGB::Black);
			}

			if (animationCounter >= groupLength)
			{
				animationCounter = 0;
				myCounter = 0;
				DEBUG_MSG("END GROUP ANIMATION\n -----------------------------\n");
			}
			DEBUG_MSG("myCounter: %i \t animationCounter: %i\n", myCounter, animationCounter);
		}
	}
}

void Pattern::cycleGroup()
{
	/* Alternating group strobe */
	static long timeForAnimation = 500;
	static bool cycleComplete = true;
	static long startTime;
	static long myStartTime;
	static long myEndTime;
	static long lightTime;
	static float lightToBlackRatio = 0.25;

	if (millisSinceBeat == 0 && cycleComplete)
	{
		DEBUG_MSG("SET SETTINGS FOR CYCLE GROUP\n");
		cycleComplete = false;
		timeForAnimation = beatPeriodMillis;
		startTime = now;
		myStartTime = now + timeForAnimation * group;
		myEndTime = now + timeForAnimation * (group + 1);
		lightTime = lightToBlackRatio * timeForAnimation + myStartTime;
		leds[2] = CRGB::Green;
	}
	if (canWeUpdate())
	{
		if (now >= myStartTime && now < myEndTime)
		{
			if (now <= lightTime)
			{
				fill_solid(backleds, side_length, baseColor);
			}
			else
			{
				fade_raw(backleds, side_length, 15);
			}
		}
		else
		{
			// space for less bright animation
			fill_solid(backleds, side_length, CRGB::Black); //debugging
		}
	}
	if ((now > (startTime + timeForAnimation)) && !cycleComplete)
	{
		cycleComplete = true;
		DEBUG_MSG("CYCLE DONE!\n");
	}
	else
	{
		DEBUG_MSG("CYCLE RUNNING!");
	}
}

void Pattern::cycleBaseCompartments()
{
	/* Alternating group strobe */
	static long totalAnimationEndTime;
	static long myGroupAnimationStart;
	static long lastStepTime;
	static int myGroupSteps;
	static int myStartSteps;
	static int myEndSteps;
	static int currentStep;
	static double stepTime;
	static bool cycleComplete = true;
	static int myStep;

	if (millisSinceBeat == 0 && cycleComplete)
	{
		leds[2] = CRGB::Green;
		DEBUG_MSG("SET SETTINGS FOR CYCLE GROUP\n");
		cycleComplete = false;
		totalAnimationEndTime = now + (this->maxGroup + 1) * 2 * beatPeriodMillis + 20; //plus 20 um letzter Gruppe bisschen Zeit zu geben
		myGroupAnimationStart = now + (this->group) * 2 * beatPeriodMillis;
		myGroupSteps = (this->maxPosition + 1) * 5;
		myStartSteps = (this->position) * 5;
		myEndSteps = (this->position + 1) * 5;
		stepTime = (2 * beatPeriodMillis / ((this->maxPosition + 1) * 5));
		lastStepTime = 0;
		myStep = 0;
		currentStep = 0;
	}
	if (canWeUpdate())
	{
		if (now < totalAnimationEndTime)
		{
			if (now >= myGroupAnimationStart)
			{
				fade_raw(leds, length, 10);
				if (now - lastStepTime > stepTime)
				{
					lastStepTime = now;
					if ((currentStep >= myStartSteps) && (currentStep <= myEndSteps))
					{
						fillCompartmentBack(baseColor, myStep);
						fillCompartmentFront(frontColor, 4 - myStep);
						myStep++;
					}
					else
					{
						fill_solid(backleds, side_length, CRGB::Black);
					}
					currentStep++;
				}
			}
			else
			{
				// space for less bright animation
				fill_solid(backleds, side_length, CRGB::Black);
			}
		}
		else
		{
			fade_raw(leds, length, 10);
			cycleComplete = true;
		}
	}
}

void Pattern::frontChoser()
{
	int temp = (int)this->nfrontPattern;

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
	case 8:
		frontCompartmentUp();
		break;
	case 9:
		frontCompartmentDown();
		break;
	default:
		break;
	}
}

void Pattern::baseChoser()
{
	int temp = (int)this->nbasePattern;
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
		baseCompartmentUp();
		break;
	case 7:
		baseCompartmentDown();
		break;
	case 8:
		cycleGroup();
		break;
	case 9:
		cycleBaseCompartments();
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
	int temp = (int)this->nstrobePattern;
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
		@param number between 0 and 103.
	 */
	if (number < 0)
	{
		number = 0;
	}
	else if (number > 103)
	{
		number = 103;
	}

	nfrontPattern = patternCombinations[number][0];
	nbasePattern = patternCombinations[number][1];
	nstrobePattern = patternCombinations[number][2];
}

void Pattern::colorChooser(int number)
{
	/*
		@param number between 0 and 99
	 */
	if (number < 0)
	{
		number = 0;
	}
	else if (number > 99)
	{
		number = 99;
	}
	nfrontColor = colorCombinations[number][0];
	nbaseColor = colorCombinations[number][1];
	nstrobeColor = colorCombinations[number][1]; // strobe color is always base color!

	setSettings();
}

void Pattern::speedChooser(int number)
{
	/*
		@param number between 0 and 26
	 */
	if (number < 0)
	{
		number = 0;
	}
	else if (number > 26)
	{
		number = 26;
	}
	setNfrontSpeed(speedCombinations[number][0]);
	setNbaseSpeed(speedCombinations[number][1]);
	setNstrobeSpeed(speedCombinations[number][2]);

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
