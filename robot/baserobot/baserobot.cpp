#include <cstdio>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <termios.h>

#include "baserobot.h"

#define DEV_BAUD  B57600
#define SYNC_NSEC 200000000

using namespace arma;

BaseRobot::BaseRobot(int robotid)
{
	this->robotid = robotid;
}

BaseRobot::~BaseRobot(void)
{
}

int BaseRobot::id(void)
{
	return this->robotid;
}

bool BaseRobot::connect(void)
{
	DIR *device_dir = opendir("/dev/");
	struct dirent *entry;
	// Iterate through all the filenames in the directory,
	// Add all the possible connections to the list
	while ((entry = readdir(device_dir)))
	{
		if (strcmp(entry->d_name, ".") != 0 &&
				strcmp(entry->d_name, "..") != 0 &&
				strstr(entry->d_name, "ttyACM"))
		{
			char *pport = new char[strlen("/dev/") + strlen(entry->d_name) + 1];
			sprintf(pport, "/dev/%s", entry->d_name);
			this->pports.push_back(pport);
		}
	}
	closedir(device_dir);
	if (this->pports.size() == 0)
	{
		this->disconnect();
		return -1;
	}

	// When finished adding all the possible filenames,
	// try to connect to a couple of them (NUM_DEV)
	// and identify their ids
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;
	for (char *pport : this->pports)
	{
		// Connect device
		serial_t *connection = new serial_t;
		serial_connect(connection, pport, DEV_BAUD);
		if (!connection->connected)
		{
			continue;
		}
		else
		{
			this->connections.push_back(connection);
		}
	}

	// Read a message from each device
	printf("[BASEROBOT] Sync 1: garbage...\n");
	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);
	}

	// Read another one in case that one was garbage
	printf("[BASEROBOT] Sync 2: actual...\n");
	nanosleep(&synctime, NULL);
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		serial_t *connection = this->connections[i];
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);

		// If a valid device, add as connected, otherwise disconnect
		int id;
		sscanf(msg, "[%d ", &id);

		if (id > 0)
		{ // Make sure the id is not 0
			this->ids.push_back(id);
		}
		else
		{
			serial_disconnect(connection);
			this->connections.erase(this->connections.begin() + i);
			delete connection;
		}
	}

	// Disconnect if number of devices is not enough, or there are too many
	if (!this->connected())
	{
		this->disconnect();
		return false;
	}
	else
	{
		printf("[BASEROBOT] Finished connecting.\n");
		return true;
	}
}

bool BaseRobot::connected(void)
{
	return this->connections.size() > 0;
}

void BaseRobot::disconnect(void)
{
	if (this->connections.size() > 0)
	{
		for (serial_t *connection : this->connections)
		{
			serial_disconnect(connection);
			delete connection;
		}
		this->connections.clear();
		this->ids.clear();
	}

	if (this->pports.size() > 0)
	{
		for (char *pport : this->pports)
		{
			delete pport;
		}
		this->pports.clear();
	}
	this->robotid = 0;
}

void BaseRobot::send(const vec &motion)
{
}

vec BaseRobot::recv(void)
{
	vec v;
	return v;
}

void BaseRobot::reset(void)
{
}