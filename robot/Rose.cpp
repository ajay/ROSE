#include <dirent.h>
#include <pthread.h>
#include <termios.h>

#include "Rose.h"

#define WBUFSIZE 128
#define DEV_BAUD  B57600
#define SYNC_NSEC 100000000

using namespace arma;

// Limit the value of x between min and max (min < x < max)
static double limitf(double x, double min, double max)
{
	if (x < min)
	{
		return min;
	}

	else if (x > max)
	{
		return max;
	}

	else
	{
		return x;
	}
}

static void *commHandler(void *args);

// Connect to all arudinos currently mounted on system
bool Rose::connect(void)
{
	// Open directory to look for arduinos
	DIR *device_dir = opendir("/dev/");
	struct dirent *entry;

	// Iterate through all possible mounted arduinos
	while ((entry = readdir(device_dir)))
	{
		if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
		{
			if (strstr(entry->d_name, "ttyACM"))
			{
				char *pport = new char[strlen("/dev/") + strlen(entry->d_name) + 1];
				sprintf(pport, "/dev/%s", entry->d_name);
				this->pports.push_back(pport);
			}
		}
	}

	// Close directy that was being traversed
	closedir(device_dir);

	// If file is empty, then disconnect
	if (this->pports.size() == 0)
	{
		this->disconnect();
		return -1;
	}

	// Attempt to connect to all arduinos
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;
	for (char *pport : this->pports)
	{
		// Connect Device
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
	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		while (!msg || strlen(msg) == 0)
		{
			msg = serial_read(connection);
		}
	}

	// Read another one in case that one was garbage
	nanosleep(&synctime, NULL);
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		serial_t *connection = this->connections[i];

		// Read message from arduino
		while (!msg || strlen(msg) == 0)
		{
			msg = serial_read(connection);
		}

		// If a valid device, add as connected, otherwise disconnect
		int id;
		sscanf(msg, "[%d ", &id);

		// Make sure DEV_ID is not less than 1
		if (id > 0)
		{
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
		printf("Stuck checking for connected()\n");
		this->disconnect();
		return false;
	}

	else
	{
		printf("[ROSE] Connected to all\n");

		// create thread locks and thread
		this->update_thread = new pthread_t;
		this->commSendLock = new pthread_mutex_t;
		this->commRecvLock = new pthread_mutex_t;
		pthread_mutex_init(this->commSendLock, NULL);
		pthread_mutex_init(this->commRecvLock, NULL);

		// start the thread
		pthread_create(this->update_thread, NULL, commHandler, this);

		return true;
	}
}

static void *commHandler(void *args)
{
	Rose *bot = (Rose *)args;

	while (!(bot->startStop))
	{
		vec tempSendVec;
		pthread_mutex_lock(bot->commSendLock);
		tempSendVec = bot->commSend;
		pthread_mutex_unlock(bot->commSendLock);
		bot->threadSend(tempSendVec);

		vec tempRecvVec = bot->threadRecv();
		pthread_mutex_lock(bot->commRecvLock);
		bot->commRecv = tempRecvVec;
		pthread_mutex_unlock(bot->commRecvLock);

	}

	return NULL;
}


/** Default connect detection method
 *  @return true if connected, false otherwise
 */
bool Rose::connected(void)
{
	return this->connections.size() > 0;
}

void Rose::disconnect(void)
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

Rose::Rose(void)
{
	this->prev_motion = zeros<vec>(4);
	this->motion_const = ones<vec>(4) * 255.0;
	if (this->connect())
	{
		this->reset();
		this->send(zeros<vec>(4));
	}
}

Rose::~Rose(void)
{
	if (this->connected())
	{
		this->send(zeros<vec>(4));
		this->reset();
		this->disconnect();
	}
}

void Rose::readClear()
{
	/* Idea is here:
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;

	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		do
		{
			msg = serial_read(connection);
			printf("Read message was: %s\n", msg);
		} while (!msg || strlen(msg) == 0);
	}
	*/

	int devid;

	// Go through every device
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		switch ((devid = this->ids[i]))
		{
			case 1:
				serial_read(this->connections[i]);
				break;
			case 2:
				serial_read(this->connections[i]); // just read everything, do nothing with it
				break;
			default:
				break;
		}
	}

	return;
}

int Rose::numconnected(void)
{
	return this->connections.size();
}

void Rose::reset(void)
{
	this->prev_motion.zeros();
}

void Rose::send(const vec &motion)
{
	// Lock the data before setting it...avoids the thread from reading the motion vector before it finishes copying over
	pthread_mutex_lock(this->commSendLock);
	this->commSend = motion;
	pthread_mutex_unlock(this->commSendLock);
}

void Rose::threadSend(const vec &motion)
{
	vec new_motion = motion;

	// Safety check
	if (new_motion.n_elem != motion_const.n_elem)
	{
		new_motion = zeros<vec>(motion_const.n_elem);
	}

	// Boundary check
	for (int i = 0; i < (int)new_motion.n_elem; i++)
	{
		new_motion(i) = limitf(new_motion(i), -1.0, 1.0);
	}

	new_motion %= motion_const;

	char msg[WBUFSIZE];
	for (int i = 0; i < (int)this->connections.size(); i++)
	{
		switch (this->ids[i])
		{
			// Arduino #1: Drive base
			case 1:

				if (new_motion(0) == this->prev_motion(0) && new_motion(1) == this->prev_motion(1) && new_motion(2) == this->prev_motion(2) && new_motion(3) == this->prev_motion(3))
				{
					continue;
				}

				else
				{
					this->prev_motion(0) = new_motion(0);
					this->prev_motion(1) = new_motion(1);
					this->prev_motion(2) = new_motion(2);
					this->prev_motion(3) = new_motion(3);

					sprintf(msg, "[%d %d %d %d]\n",
							(int)new_motion(0),
							(int)new_motion(1),
							(int)new_motion(2),
							(int)new_motion(3));
					serial_write(this->connections[i], msg);
				}

				break;

			// Arduino #2: Arm
			case 2:

				if (new_motion(4) == this->prev_motion(4))
				{
					 continue;
				}

				else
				{
					// Placeholder for now
					 continue;
				}

				break;

			default:
				break;
		}
	}
}

vec Rose::threadRecv(void)
{
	//this->encVal encoderValues;

	// char *msg;
	
	// Attempt to connect to all arduinos
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;


	// Read a message from each device
	nanosleep(&synctime, NULL);
	char *msg;
	for (serial_t *connection : this->connections)
	{
		msg = serial_read(connection);
		printf("%s\n", msg);
	}	
	

	// printf("in recv\n");

	return zeros<vec>(4);
}

vec Rose::recv(void)
{
	// Add a lock to wait until the commthread is done setting the vector
	vec tempVec;
	pthread_mutex_lock(this->commRecvLock);
	tempVec = this->commRecv;
	pthread_mutex_unlock(this->commRecvLock);
	return tempVec;
}
