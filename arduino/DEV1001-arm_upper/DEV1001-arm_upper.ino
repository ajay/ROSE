////////////////////
// ROSE ARM UPPER //
////////////////////

#include <Adafruit_MotorShield.h>
#include <Servo.h>
#include <string.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Wire.h>

#define DEV_ID 1001

#define POTPIN1   A0
#define POTPIN2   A1
#define POTPIN3   A2
#define POTPIN4   A3

Adafruit_MotorShield AFMS_base_pivot1(0x60);
Adafruit_MotorShield AFMS_base_pivot2(0x61);
Adafruit_DCMotor *motors[8];
Servo claw;
const int elbow = 0;
const int wrist = 1;
const int twist = 2;
const int grab = 3;

// PID constants
const double Kp[4] = { 3.0, 3.0, 2.5, 2.5 };
const double Ki[4] = {   0,   0,   0,   0 };
const double Kd[4] = {   0,   0,   0,   0 };

static int instr_activate;
static bool arm_theta_act;
static bool arm_vel_act;
static int pos[4];
static int vel[4];
static int pvel[4];

const int bufsize = 256;
const int safesize = bufsize / 2;
static char buf[bufsize];
static char msg[bufsize];
static char wbuf[safesize];
unsigned long msecs;
unsigned long timeout;
unsigned long piddiff;
static char numbuf[8];

static double total_err[4];
static double prev_err[4];

// Limit values of 'x' between a certain range (a < x < b)
int limit(int x, int a, int b)
{
	if 		(x > b)	{ return b; }
	else if (x < a)	{ return a; }
	else 			{ return x; }
}

void setmotors(int vv[])
{
	int v[4];
	int Ktolerance[4] = { 20, 15, 15, 10 };

	for (int i = 0; i < 4; i++)
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
	int rev[8] = { 1, 1, 0, 0, 1, 0, 1, 0 };

	bool isneg[4];

	isneg[elbow] = v[elbow] < 0;
	v[elbow] = limit(abs(v[elbow]), 0, 255);
	isneg[wrist] = v[wrist] < 0;
	v[wrist] = limit(abs(v[wrist]), 0, 255);
	isneg[twist] = v[twist] < 0;
	v[twist] = limit(abs(v[twist]), 0, 255);
	isneg[grab] = false;
	v[grab] = limit(v[grab] * 90 / 255 + 90, 0, 180);

	claw.write(v[grab]);

	int speeds[8] = { v[elbow], v[elbow], v[elbow], v[elbow],
					  v[wrist], v[wrist], v[twist], 0 };
	int isneg2[8] = { isneg[elbow], isneg[elbow], isneg[elbow], isneg[elbow],
					  isneg[wrist], isneg[wrist], isneg[twist], 0 };

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
	claw.attach(10);
	for (int i = 0; i < 4; i++)
	{
		motors[i] = AFMS_base_pivot1.getMotor(i + 1);
		motors[4 + i] = AFMS_base_pivot2.getMotor(i + 1);
	}

	// Set up the sensors
	pinMode(POTPIN1, INPUT); // grab
	pinMode(POTPIN2, INPUT); // twist
	pinMode(POTPIN3, INPUT); // wrist
	pinMode(POTPIN4, INPUT); // elbow

	// Flash led 13
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// Turn on the motor shields
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

		// Extract possible message
		char *s, *e;
		if ((e = strchr(buf, '\n')))
		{
			e[0] = '\0';
			if ((s = strrchr(buf, '[')))
			{
				// CUSTOMIZE (set the setpoint)
				sscanf(s, "[%d %d %d %d %d %d %d %d %d]\n",
						&instr_activate,
						&pos[elbow],
						&pos[wrist],
						&pos[twist],
						&pos[grab],
						&vel[elbow],
						&vel[wrist],
						&vel[twist],
						&vel[grab]);
				// Limit the positional encoders
				pos[elbow] = limit(pos[elbow], 310, 1000);
				pos[wrist] = limit(pos[wrist], 100, 894);
				pos[twist] = limit(pos[twist], 100, 930);
				pos[grab] = limit(pos[grab], 600, 1000);

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
		// After .5 seconds, stop the robot
		memset(pvel, 0, sizeof(int) * 4);
		memset(vel, 0, sizeof(int) * 4);
		setmotors(vel);
		arm_theta_act = false;
		arm_vel_act = false;
		// Safety sets
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

			err = (double)(pos[elbow] - analogRead(POTPIN4));
			total_err[elbow] += err * dt;
			delta_err = (err - prev_err[elbow]) / dt;
			vel[elbow] = err * Kp[elbow] + total_err[elbow] * Ki[elbow] + delta_err * Kd[elbow];
			prev_err[elbow] = err;

			err = (double)(pos[wrist] - analogRead(POTPIN3));
			total_err[wrist] += err * dt;
			delta_err = (err - prev_err[wrist]) / dt;
			vel[wrist] = err * Kp[wrist] + total_err[wrist] * Ki[wrist] + delta_err * Kd[wrist];
			prev_err[wrist] = err;

			err = (double)(pos[twist] - analogRead(POTPIN2));
			total_err[twist] += err * dt;
			delta_err = (err - prev_err[twist]) / dt;
			vel[twist] = err * Kp[twist] + total_err[twist] * Ki[twist] + delta_err * Kd[twist];
			prev_err[twist] = err;

			err = (double)(pos[grab] - analogRead(POTPIN1));
			total_err[grab] += err * dt;
			delta_err = (err - prev_err[grab]) / dt;
			vel[grab] = err * Kp[grab] + total_err[grab] * Ki[grab] + delta_err * Kd[grab];
			prev_err[grab] = err;
		}
	}
	else
	{
		memset(pvel, 0, sizeof(int) * 4);
		memset(vel, 0, sizeof(int) * 4);
		setmotors(vel);
		piddiff = millis();
	}

	// Ramp function for the signal output
	int deltav[4] = { limit(vel[elbow] - pvel[elbow], -8, 8),
					  limit(vel[wrist] - pvel[wrist], -8, 8),
					  limit(vel[twist] - pvel[twist], -8, 8),
					  limit(vel[grab] - pvel[grab], -255, 255) };

	int v[4];
	v[elbow] = limit(pvel[elbow] + deltav[elbow], -172, 172);
	v[wrist] = limit(pvel[wrist] + deltav[wrist], -172, 172);
	v[twist] = limit(pvel[twist] + deltav[twist], -172, 172);
	v[grab] = limit(pvel[grab] + deltav[grab], -255, 255);

	// Push the values to the motors
	setmotors(v);

	pvel[elbow] = v[elbow];
	pvel[wrist] = v[wrist];
	pvel[twist] = v[twist];
	pvel[grab] = v[grab];

	if (millis() - msecs > 50)
	{
		sprintf(wbuf, "[%d %d %d %d %d %d %d %d %d]\n",
				DEV_ID,
				analogRead(POTPIN4),
				analogRead(POTPIN3),
				analogRead(POTPIN2),
				analogRead(POTPIN1),
				pvel[elbow],
				pvel[wrist],
				pvel[twist],
				pvel[grab]);
		Serial.print(wbuf);
		msecs = millis();
	}
}