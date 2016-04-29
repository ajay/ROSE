/////////////////
// ROSE SCREEN //
/////////////////

#include <string.h>
#include <Wire.h>
#include "Charliplexing.h"
#include "Font.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#define DEV_ID 2001

const int bufsize = 256;
const int safesize = bufsize / 2;
static char buf[bufsize];
static char msg[bufsize];
static char wbuf[safesize];
static char message[256];

unsigned long msecs;

void setup()
{
	// Begin serial commss
	Serial.begin(57600);

	// Set up the LOLSHIELD
  LedSign::Init();

	// Flash led 13
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);
	msecs = millis();
}

void loop()
{
  message = "Rose\0";
	int nbytes = 0;
	if ((nbytes = Serial.available()))
	{
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
				// CHANGE THIS TO RECEIVE STRINGS OR WHATEVER YOU WANT
				sscanf(s, "[%d]\n", &message);
			}
			memmove(buf, &e[1], strlen(&e[1]) + sizeof(char));
		}
	}

	// SEND A MESSAGE BACK TO ROSE
	if (millis() - msecs > 50)
	{
		char *write_string = "hey, wassup";
		sprintf(wbuf, "[%d %s]\n",
				DEV_ID,
				write_string);
		Serial.print(wbuf);
		msecs = millis();
	}
}
