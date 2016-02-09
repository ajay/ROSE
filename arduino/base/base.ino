/////////////////////
// ROSE DRIVE BASE //
/////////////////////

#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Wire.h>

#define DEV_ID 0
#define ramp_const 8

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motors[4];

const int bufsize = 256;
const int safesize = bufsize / 2;
char buf[bufsize];
char msg[bufsize];
char wbuf[safesize];
unsigned long msecs;
char numbuf[4];

static int targetv[4];
static int prevv[4];

// Limit values of 'x' between a certain range (a < x < b)
int limit(int x, int a, int b)
{
	if (x > b)
	{
		return b;
	}
	else if (x < a)
	{
		return a;
	}
	else
	{
		return x;
	}
}

// Ramps motor based on ramp_const
int rampmotor(int curr, int target)
{
	int delta = target - curr;
	delta = limit(delta, -ramp_const, ramp_const);
	curr = limit(curr + delta, -255, 255);
	return curr;
}

// Assigns velocity values to motors
void setmotors(int leftFront, int rightFront, int leftBack, int rightBack)
{
	// Determine whether motor assignments need to be forward or backward
	bool negative[4] = {rightFront < 0, leftFront < 0, leftBack < 0, rightBack < 0};

	// Limit values to be assigned within acceptable range (0 - 255)
	rightFront 	= limit(abs(rightFront), 	0, 255);
	leftFront 	= limit(abs(leftFront), 	0, 255);
	leftBack 	= limit(abs(leftBack), 		0, 255);
	rightBack 	= limit(abs(rightBack), 	0, 255);

	// Set motors to correct directions
	for (int i = 0; i < 4; i++)
	{
		if (negative[i])
		{
			motors[i]->run(FORWARD);
		}
		else
		{
			motors[i]->run(BACKWARD);
		}
	}

	// Set motors to assigned values
	motors[0]->setSpeed(rightFront);
	motors[1]->setSpeed(leftFront);
	motors[2]->setSpeed(leftBack);
	motors[3]->setSpeed(rightBack);
}

// Initial set up (attach motors & begin serial comm)
void setup()
{
	// Attach motors to motor array
	motors[0] = AFMS.getMotor(1);
	motors[1] = AFMS.getMotor(2);
	motors[2] = AFMS.getMotor(3);
	motors[3] = AFMS.getMotor(4);

	// Set status LED to OUTPUT and HIGH
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// Start motorshield & serial comm
	AFMS.begin();
	setmotors(0, 0, 0, 0);
	Serial.begin(57600);
	msecs = millis();
}

void loop()
{
	// See if there are any available bytes to be read over serial comm
	int nbytes = 0;
	if ((nbytes = Serial.available()))
	{
		// Read + attach null byte to read string
		int obytes = strlen(buf);
		Serial.readBytes(&buf[obytes], nbytes);
		buf[nbytes + obytes] = '\0';

		// Resize read string just in case
		if (strlen(buf) > safesize)
		{
			memmove(buf, &buf[strlen(buf) - safesize], safesize);
			buf[safesize] = '\0';
		}

		// Extract possible message
		char *s, *e;
		if ((e = strchr(buf, '\n')))
		{
			e[0] = '\0';
			if ((s = strrchr(buf, '[')))
			{
				// Parse string being read
				sscanf(s, "[%d %d %d %d]\n", &targetv[0], &targetv[1], &targetv[2], &targetv[3]);
			}
			memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));
		}
	}

	// Assign read values to motors
	for (int i = 0; i < 4; i++)
	{
		prevv[i] = rampmotor(prevv[i], targetv[i]);
	}
	setmotors(-prevv[0], -prevv[1], prevv[2], prevv[3]);

	// Send back data over serial every 100ms
	if (millis() - msecs > 100)
	{
		sprintf(wbuf, "[%d %d %d %d %d]\n",
				DEV_ID,
				prevv[0],
				prevv[1],
				prevv[2],
				prevv[3]);
		Serial.print(wbuf);
		msecs = millis();
	}
}
