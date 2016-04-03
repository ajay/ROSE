/////////////////////
// ROSE DRIVE BASE //
/////////////////////

#include <Adafruit_MotorShield.h>
#include <string.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Wire.h>

#define DEV_ID 1
#define ramp_const 16 // Higher is faster

// Variables associated with buffers for serial comm
const int bufsize = 256;
const int safesize = bufsize / 2;
char buf[bufsize];
char msg[bufsize];
char wbuf[safesize];
unsigned long msecs;

// Mounting motorshield
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motors[4];

// Arrays that hold target and previous values for the motors
static int targetv[4];
static int prevv[4];

// Variables for reading in current and voltage values
int VRaw; //This will store our raw ADC data
int IRaw;
float VFinal; //This will store the converted data
float IFinal;
long int twelve_volt_voltage = 0;
long int twelve_volt_current = 0;

unsigned long timeout;

// QuadEncoder class set up to utilize the encoders
class QuadEncoder
{
	public:
		long pos;
		bool reversed;
		char pin[2];
		int motor;

		QuadEncoder()
		{
			reset();
		}

		// Attach quadencoder based on two input pins on digital IO
		void attach(int pin1, int pin2, int m)
		{
			pin[0] = pin1;
			pin[1] = pin2;
			pinMode(pin[0], INPUT);
			pinMode(pin[1], INPUT);
			pin_state[0] = digitalRead(pin[0]) == HIGH;
			pin_state[1] = digitalRead(pin[1]) == HIGH;
			motor = m;
		}

		// Read values from input pins
		long long read()
		{
			update();
			return pos;
		}

		// Reset QuadEncoder instance
		void reset()
		{
			pin[0] = 0;
			pin[1] = 0;
			pos = 0;
			velocity = 1;
			reversed = false;
			pin_state[0] = 0;
			pin_state[1] = 0;
		}

		// Reset encoder values back to 0
		void reset_pos()
		{
			pos = 0;
		}

	private:
		char pin_state[2];
		long long velocity;
		int prev_state;

		// Private update method to read and interpolate quadencoder data
		void update()
		{
			// FSM : reg :: 00 01 11 10 		(0 -> 1 -> 2 -> 3)
			//     : rev :: 10 11 01 00 		(3 -> 2 -> 1 -> 0)

			char new_state[2] = { digitalRead(pin[0]) == HIGH, digitalRead(pin[1]) == HIGH };
			char delta_state[2] = { new_state[0] != pin_state[0], new_state[1] != pin_state[1] };
			int state;
			int diff;

			if 		(new_state[0] == 0 && new_state[1] == 0) { state = 0; }
			else if (new_state[0] == 0 && new_state[1] == 1) { state = 1; }
			else if (new_state[0] == 1 && new_state[1] == 1) { state = 2; }
			else if (new_state[0] == 1 && new_state[1] == 0) { state = 3; }

			// If motor is moving backwards, then move encoder backwards
			if (prevv[motor] < 0)
			{
				diff = state - prev_state;
				if (diff > 0)
				{
					diff = diff - 4;
				}

				pos += diff;
			}

			// If motor is moving forwards, then move encoder forwards
			else if (prevv[motor] > 0)
			{
				diff = state - prev_state;
				if (diff < 0)
				{
					diff = diff + 4;
				}

				pos += diff;
			}

			// If motor is not moving, then predict which direction it is moving in
			else if (prevv[motor] == 0)
			{
				if (delta_state[0] && delta_state[1])
				{
					pos += velocity * 2 * (reversed ? -1 : 1);
				}

				else if (delta_state[1])
				{
					velocity = (new_state[0] == new_state[1]) ? -1 : 1;
					pos += velocity * (reversed ? -1 : 1);
				}

				else if (delta_state[0])
				{
					velocity = (new_state[0] == new_state[1]) ? 1 : -1;
					pos += velocity * (reversed ? -1 : 1);
				}

				pin_state[0] = new_state[0];
				pin_state[1] = new_state[1];
			}

			prev_state = state;
		}
};

// Arrays to hold data associated with the encoders
QuadEncoder encoders[4];
long encoder_values[4];

// Limit values of 'x' between a certain range (a < x < b)
int limit(int x, int a, int b)
{
	if 		(x > b)	{ return b; }
	else if (x < a)	{ return a; }
	else 			{ return x; }
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
	rightFront	= limit(abs(rightFront),	0, 255);
	leftFront	= limit(abs(leftFront),		0, 255);
	leftBack	= limit(abs(leftBack),		0, 255);
	rightBack	= limit(abs(rightBack),		0, 255);

	// Set motors to correct directions
	for (int i = 0; i < 4; i++)
	{
	if (negative[i])
		{
			motors[i]->run(BACKWARD);
		}
		else
		{
			motors[i]->run(FORWARD);
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
	// Set status LED to OUTPUT and HIGH
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// Attach motors to motor array
	for (int i = 0; i < 4; i++)
	{
		motors[i] = AFMS.getMotor(i+1);
	}

	// Attach encoders (pin0, pin1, motor)
	encoders[0].attach(2, 3, 2);
	encoders[1].attach(4, 5, 3);
	encoders[2].attach(6, 7, 0);
	encoders[3].attach(8, 11, 1);

	// Start motorshield & serial comm
	AFMS.begin();
	setmotors(0, 0, 0, 0);
	Serial.begin(57600);
	msecs = millis();
	timeout = millis();
}

void loop()
{
  // timeout = millis();
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

		char *s, *e;

		// Check for encoder reset
		if (strstr(buf, "[reset]\n") != NULL)
		{
			e[0] = '\0';
			memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));

			for (int i = 0; i < 4; i++)
			{
				encoders[i].reset_pos();
			}
		}

		// Otherwise extract possible message
		else
		{
			if ((e = strchr(buf, '\n')))
			{
				e[0] = '\0';
				if ((s = strrchr(buf, '[')))
				{
					// Parse string being read
					// Left front, right front, left back, right back
					sscanf(s, "[%d %d %d %d]\n", &targetv[1], &targetv[3], &targetv[0], &targetv[2]);
                    timeout = millis();
				}
				memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));
			}
		}
	}

	// EMERGENCY STOP: MASTER COMM LOST (for testing turn this off)
	if (millis() - timeout > 500)
	{
		// After .5 seconds, stop the robot
		memset(targetv, 0, sizeof(int) * 4);
		memset(prevv, 0, sizeof(int) * 4);
		setmotors(0, 0, 0, 0);
		timeout = millis();
	}

	// Ramp motors values, and determine next value to set
	for (int i = 0; i < 4; i++)
	{
		prevv[i] = rampmotor(prevv[i], targetv[i]);
	}

	// Assign determined values to motors
	setmotors(-prevv[0], -prevv[1], prevv[2], prevv[3]);

	// Read encoder values
	for (int i = 0; i < 4; i++)
	{
		encoder_values[i] = encoders[i].read();
	}

	// Read voltage and current
	VRaw = analogRead(A0);
	IRaw = analogRead(A1);

	VFinal = (float)(VRaw) / 12.99;
	IFinal = (float)(IRaw) / 3.7;

	twelve_volt_voltage = (int)(VFinal * 1000);
	twelve_volt_current = (int)(IFinal * 1000);

	// Send back data over serial every 100ms
	if (millis() - msecs > 100)
	{
		sprintf(wbuf, "[%d %d %d %d %d %ld %ld %ld %ld %ld %ld]\n",
				DEV_ID,
				prevv[1],
				prevv[3],
				prevv[0],
				prevv[2],
				encoder_values[3],
				encoder_values[1],
				encoder_values[2],
				encoder_values[0],
				twelve_volt_voltage,
				twelve_volt_current);
		Serial.print(wbuf);
		msecs = millis();
	}
}
