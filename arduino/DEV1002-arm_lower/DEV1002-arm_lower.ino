////////////////////
// ROSE ARM LOWER //
////////////////////

#include <Adafruit_MotorShield.h>
#include <string.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Wire.h>

#define DEV_ID 1002

#define POTPIN1   A0
#define POTPIN2   A1
#define CSENSE1   A2
#define CSENSE2   A3

Adafruit_MotorShield AFMS_base_pivot1(0x60);
Adafruit_MotorShield AFMS_base_pivot2(0x62);
Adafruit_DCMotor *motors[8];
const int chassis = 0;
const int shoulder = 1;

// PID constants
const double Kp[2] = { 2.5, 3.0 };
const double Ki[2] = { 0, 0 };
const double Kd[2] = { 0, 0 };

static int instr_activate;
static bool arm_theta_act;
static bool arm_vel_act;
static int pos[2];
static int vel[2];
static int pvel[2];

const int bufsize = 256;
const int safesize = bufsize / 2;
static char buf[bufsize];
static char msg[bufsize];
static char wbuf[safesize];
unsigned long msecs;
unsigned long timeout;
unsigned long piddiff;
static char numbuf[8];

static double total_err[2];
static double prev_err[2];

// Limit values of 'x' between a certain range (a < x < b)
int limit(int x, int a, int b)
{
	if		(x > b)	{ return b; }
	else if (x < a)	{ return a; }
	else			{ return x; }
}

void setmotors(int vv[])
{
	int v[2];
	int Ktolerance[2] = { 15, 20 };
	for (int i = 0; i < 2; i++)
	{
		v[i] = vv[i];
		if (abs(v[i]) < 10)
		{
			v[i] = 0;
		}
		else
		{
			v[i] += v[i] < 0 ? -Ktolerance[i] : Ktolerance[i];
		}
	}
	// This only applies to the motors on the shields
	int rev[8] = { 0, 0, 1, 1, 0, 0, 1, 1 };

	bool isneg[2];

	isneg[chassis] = v[chassis] < 0;
	v[chassis] = limit(abs(v[chassis]), 0, 255);
	isneg[shoulder] = v[shoulder] < 0;
	v[shoulder] = limit(abs(v[shoulder]), 0, 255);

	int speeds[8] = { v[shoulder], v[shoulder], v[shoulder], v[shoulder],
					  0, 0, v[chassis], v[chassis]};
	int isneg2[8] = {isneg[shoulder], isneg[shoulder], isneg[shoulder], isneg[shoulder],
					  0, 0, isneg[chassis], isneg[chassis]};

	for (int i = 0; i < 8; i++)
	{
		motors[i]->setSpeed(speeds[i]);
		if (speeds[i] == 0)
		{
			motors[i]->run(RELEASE);
		}
		else
		{
			bool neg = (isneg2[i] && !rev[i]) || (!isneg2[i] && rev[i]);
			motors[i]->run(neg ? BACKWARD : FORWARD);
		}
	}
}

void setup()
{
	Serial.begin(57600);

	// Set up the motors
	for (int i = 0; i < 4; i++)
	{
		motors[i] = AFMS_base_pivot1.getMotor(i + 1);
		motors[4 + i] = AFMS_base_pivot2.getMotor(i + 1);
	}

	// Set up the sensors
	pinMode(POTPIN1, INPUT);
	pinMode(POTPIN2, INPUT);
	pinMode(CSENSE1, INPUT);
	pinMode(CSENSE2, INPUT);

	// Flash led 13
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// Turn on the motor shield
	AFMS_base_pivot1.begin();
	AFMS_base_pivot2.begin();

	setmotors(vel);
	msecs = millis();
	timeout = millis();
	piddiff = millis();
}

void loop()
{
	int nbytes = 0;
	if ((nbytes = Serial.available()))
	{
		timeout = millis();

		// Read + attach null byte
		int obytes = strlen(buf);
		Serial.readBytes(&buf[obytes], nbytes);
		buf[nbytes + obytes] = '\0';

		// Resize just in case
		if (strlen(buf) > safesize)
		{
			memmove(buf, &buf[strlen(buf) - safesize], safesize);
			buf[safesize] = '\0'; // just in case
		}

		// extract possible message
		char *s, *e;
		if ((e = strchr(buf, '\n')))
		{
			e[0] = '\0';
			if ((s = strrchr(buf, '[')))
			{
				// CUSTOMIZE (set the setpoint)
				sscanf(s, "[%d %d %d %d %d]\n",
						&instr_activate,
						&pos[chassis],
						&pos[shoulder],
						&vel[chassis],
						&vel[shoulder]);
				// limit the positional encoders
				pos[chassis] = limit(pos[chassis], 100, 1000);
				pos[shoulder] = limit(pos[shoulder], 213, 832);

				arm_theta_act = instr_activate & 0x01;
				arm_vel_act = (instr_activate & 0x02) >> 1;
				timeout = millis();
			}
			memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));
		}
	}

	// EMERGENCY STOP: MASTER COMM LOST (for testing turn this off)
	if (millis() - timeout > 500)
	{
		// after .5 seconds, stop the robot
		memset(pvel, 0, sizeof(int) * 2);
		memset(vel, 0, sizeof(int) * 2);
		setmotors(vel);
		arm_theta_act = false;
		arm_vel_act = false;
		// safety sets
		piddiff = millis();
	}

	if (arm_vel_act)
	{
		piddiff = millis();
	}
	else if (arm_theta_act)
	{
		double err;
		double delta_err;
		double dt = (double)(millis() - piddiff) / 1000.0; // in seconds
		if (dt > 0)
		{
			piddiff = millis();

			err = (double)(pos[chassis] - analogRead(POTPIN2));
			total_err[chassis] += err * dt;
			delta_err = (err - prev_err[chassis]) / dt;
			vel[chassis] = err * Kp[chassis] + total_err[chassis] * Ki[chassis] + delta_err * Kd[chassis];
			prev_err[chassis] = err;

			err = (double)(pos[shoulder] - analogRead(POTPIN1));
			total_err[shoulder] += err * dt;
			delta_err = (err - prev_err[shoulder]) / dt;
			vel[shoulder] = err * Kp[shoulder] + total_err[shoulder] * Ki[shoulder] + delta_err * Kd[shoulder];
			prev_err[shoulder] = err;
		}
	}
	else
	{
		memset(pvel, 0, sizeof(int) * 2);
		memset(vel, 0, sizeof(int) * 2);
		setmotors(vel);
		piddiff = millis();
	}

	// Ramp function for the signal output
	int deltav[2] = { limit(vel[chassis] - pvel[chassis], -8, 8),
					  limit(vel[shoulder] - pvel[shoulder], -8, 8) };
	int v[2];
	v[chassis] = limit(pvel[chassis] + deltav[chassis], -255, 255);
	v[shoulder] = limit(pvel[shoulder] + deltav[shoulder], -172, 172);

	// Push the values to the motors
	setmotors(v);

	pvel[chassis] = v[chassis];
	pvel[shoulder] = v[shoulder];

	if (millis() - msecs > 50)
	{
		sprintf(wbuf, "[%d %d %d %d %d %d %d]\n",
				DEV_ID,
				analogRead(POTPIN2),
				analogRead(POTPIN1),
				analogRead(CSENSE1),
				analogRead(CSENSE2),
				pvel[chassis],
				pvel[shoulder]);
		Serial.print(wbuf);
		msecs = millis();
	}
}