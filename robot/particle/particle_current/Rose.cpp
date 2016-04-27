#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <termios.h>

#include "mathfun.h"
#include "Rose.h"

#define WBUFSIZE 128
#define DEV_BAUD  B57600
#define SYNC_NSEC 100000000

using namespace arma;
using namespace std;
using json = nlohmann::json;

Rose::Rose(void)
{
	this->motion_const = ones<vec>(10);
	this->motion_const(span(0,3)) *= 255;
	this->startStop = false;

	// ARM STUFF
	this->arm_active = false;
	this->calibration_loaded = false;
	this->arm_mint = zeros<vec>(6);
	this->arm_maxt = zeros<vec>(6);
	this->arm_minv = zeros<vec>(6);
	this->arm_maxv = zeros<vec>(6);
	this->arm_link_length = { 0.2, 5.0, 10.5, 6.5, 4.5, 3.8, 3.75 };

	if (this->connect())
	{
		this->reset();
		this->send(zeros<vec>(10));
	}
	else
	{
		if (this->numconnected() <= 0)
		{
			printf("[ROSE] Could not connect to any arduinos...\n");
		}

		else
		{
			printf("[ROSE] Connecting to a particular arduino failed\n");
		}

		printf("Exiting due to the existance of a mind without the body.\n");
		exit(0);
	}
}

Rose::~Rose(void)
{
	if (this->connected())
	{
		this->send(zeros<vec>(10));
		this->reset();
		this->disconnect();
	}
}

int Rose::numconnected(void)
{
	return this->connections.size();
}

bool Rose::connected(void)
{
	return this->connections.size() > 0;
}

void Rose::reset(void)
{
}

void Rose::reset_encoders(void)
{
	this->reset_enc = true;
}

void* Rose::commHandler(void* args)
{
	Rose *rose = (Rose *)args;

	while (!(rose->startStop))
	{
		vec tempSendVec;
		pthread_mutex_lock(rose->commSendLock);
		tempSendVec = rose->commSend;
		//cout << rose->commSend << endl;
		pthread_mutex_unlock(rose->commSendLock);
		rose->threadSend(tempSendVec);
		rose->threadRecv();
	}
	printf("[ROSE] Exiting\n");

	return NULL;
}

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
		return false;
	}

	// Create delay to read at correct rate
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;

	// Attempt to connect to all arduinos
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
	char *msg = (char*)"";
	for (serial_t *connection : this->connections)
	{
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);
	}

	// Read another one in case that one was garbage
	nanosleep(&synctime, NULL);
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		serial_t *connection = this->connections[i];

		// Read message from arduino
		do
		{
			msg = serial_read(connection);
		} while (!msg || strlen(msg) == 0);

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

	else if (this->numconnected() == 0)
	{
		printf("Could not connect to all arduinos\n");
		return false;
	}

	else
	{
		printf("[ROSE] Connected to all\n");

		// Create thread locks and threads
		this->update_thread = new pthread_t;
		this->commSendLock = new pthread_mutex_t;
		this->commRecvLock = new pthread_mutex_t;
		this->commRecv = vec(10);
		this->commSend = vec(10);
		this->load_calibration_params("calib_params.json");
		pthread_mutex_init(this->commSendLock, NULL);
		pthread_mutex_init(this->commRecvLock, NULL);

		// Start the update thread
		pthread_create(this->update_thread, NULL, this->commHandler, this);
		return true;
	}
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
	this->arm_active = false;
}

void Rose::send(const vec &motion)
{
	// Lock the data before setting it...avoids the thread from reading the motion vector before it finishes copying over
	if (this->numconnected() > 0)
	{
		pthread_mutex_lock(this->commSendLock);
		this->commSend = motion;
		pthread_mutex_unlock(this->commSendLock);
	}
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
	for (int i = 0; i < 4; i++)
	{
		new_motion(i) = limit_value(new_motion(i), -1.0, 1.0);
	}
	if (this->calibrated())
	{
		for (int i = 0; i < 6; i++)
		{
			new_motion(i+4) = map_value(new_motion(i+4), arm_mint(i), arm_maxt(i), arm_minv(i), arm_maxv(i));
			new_motion(i+4) = limit_value(new_motion(i+4), arm_minv(i), arm_maxv(i));
		}
	}
	else
	{
		arm_active = false; // don't turn on if not calibrated
	}

	// Parse the new motion originally from (-1 to 1) to (-255 to 255)
	new_motion %= motion_const;

	char instr_activate = 0x80 | (this->arm_active);
	char msg[WBUFSIZE];

	//cout << new_motion << endl;

	for (int i = 0; i < (int)this->connections.size(); i++)
	{
		switch (this->ids[i])
		{
			// Arduino #1: Drive base
			case 1:
				sprintf(msg, "[%d %d %d %d %d]\n",
						(int)new_motion(0),
						(int)new_motion(1),
						(int)new_motion(2),
						(int)new_motion(3),
						(int)this->reset_enc);

				serial_write(this->connections[i], msg);
				this->reset_enc = false;
				break;

			// Arduino #2: Arm Upper
			case 1001:
				sprintf(msg, "[%d %d %d %d %d %d %d %d %d]\n",
						instr_activate,
						(int)new_motion(6),
						(int)new_motion(7),
						(int)new_motion(8),
						(int)new_motion(9),
						0, 0, 0, 0);
				serial_write(this->connections[i], msg);
				break;

			// Arduino #3: Arm Lower
			case 1002:
				sprintf(msg, "[%d %d %d %d %d]\n",
						instr_activate,
						(int)new_motion(4),
						(int)new_motion(5),
						0, 0);
				serial_write(this->connections[i], msg);
				break;

			default:
				break;
		}
	}
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

void Rose::threadRecv(void)
{
	// Create delay to read at correct rate
	struct timespec synctime;
	synctime.tv_nsec = SYNC_NSEC % 1000000000;
	synctime.tv_sec = SYNC_NSEC / 1000000000;
	nanosleep(&synctime, NULL);
	int storage[8];

	for (int i = 0; i < (int)this->connections.size(); i++)
	{
		char* msg = serial_read(this->connections[i]);

		if (msg != NULL)
		{
			// printf("[ROSE] RECEIVED: %s\n", msg);
		}

		switch (this->ids[i])
		{
			case 1: // Arduino #1: Drive base
				// Convert msg into int array
				if (msg != NULL)
					//  && (strstr(msg, "\n") != NULL)
				{
					int temp_voltage = 0;
					int temp_current = 0;

					sscanf(msg, "[%*d %d %d %d %d %d %d %d %d %d %d]\n",
							&this->motor_speeds[0],
							&this->motor_speeds[1],
							&this->motor_speeds[2],
							&this->motor_speeds[3],
							&this->encoder[0],
							&this->encoder[1],
							&this->encoder[2],
							&this->encoder[3],
							&temp_voltage,
							&temp_current);

					this->twelve_volt_voltage = (float)((float)temp_voltage / 1000);
					this->twelve_volt_current = (float)((float)temp_current / 1000);
					pthread_mutex_lock(this->commRecvLock);
					this->commRecv = vec({ (double)this->encoder[0], (double)this->encoder[1], (double)this->encoder[2], (double)this->encoder[3], 0, 0, 0, 0, 0, 0 });
					pthread_mutex_unlock(this->commRecvLock);
				}
				break;

			case 1001:
				if (msg)
				{
					sscanf(msg, "[%d %d %d %d %d %d %d %d %d]\n", &this->ids[i],
							&storage[0],
							&storage[1],
							&storage[2],
							&storage[3],
							&storage[4],
							&storage[5],
							&storage[6],
							&storage[7]);
					for (int i = 0; i < 4; i++)
					{
						this->commRecv(i + 6) = map_value(storage[i], this->arm_minv(i + 2), this->arm_maxv(i + 2), this->arm_mint(i + 2), this->arm_maxt(i + 2));
					}
				}
				break;

			case 1002:
				if (msg)
				{
					sscanf(msg, "[%d %d %d %d %d %d %d]\n", &this->ids[i],
							&storage[0],
							&storage[1],
							&storage[2],
							&storage[3],
							&storage[4],
							&storage[5]);
					for (int i = 0; i < 2; i++)
					{
						this->commRecv(i + 4) = map_value(storage[i], this->arm_minv(i), this->arm_maxv(i), this->arm_mint(i), this->arm_maxt(i));
					}
				}
				break;

			default:
				break;
		}
	}
}

void Rose::set_wheels(double front_left, double front_right, double back_left, double back_right)
{
	pthread_mutex_lock(this->commSendLock);
	this->commSend(0) = front_left;
	this->commSend(1) = front_right;
	this->commSend(2) = back_left;
	this->commSend(3) = back_right;
	pthread_mutex_unlock(this->commSendLock);
}

void Rose::set_arm(double base_joint, double pivot1, double pivot2, double pivot3, double rotate_wrist, double close_claw)
{
	pthread_mutex_lock(this->commSendLock);
	this->commSend(4) = base_joint;
	this->commSend(5) = pivot1;
	this->commSend(6) = pivot2;
	this->commSend(7) = pivot3;
	this->commSend(8) = rotate_wrist;
	this->commSend(9) = close_claw;
	this->arm_active = true;
	pthread_mutex_unlock(this->commSendLock);
}

void Rose::stop_arm(void)
{
	this->arm_active = false;
}

void Rose::load_calibration_params(const string &filename)
{
	string params;
	ifstream params_file(filename);
	if (!params_file.is_open())
	{
		printf("Error: cannot find \"calib_params.json\". Please place a calibration file in here.\n");
	}

	string temp;
	while (getline(params_file, temp))
	{
		params += temp;
	}
	params_file.close();
	this->set_calibration_params(json::parse(params));
}

void Rose::set_calibration_params(json cp)
{
	vector<string> dofnames =
	{
		"joint0",
		"joint1",
		"joint2",
		"joint3",
		"joint4",
		"joint5"
	};

	for (int i = 0; i < 6; i++)
	{
		string name = dofnames[i];
		this->arm_minv(i) = cp[name]["raw_min"];
		this->arm_maxv(i) = cp[name]["raw_max"];
		this->arm_mint(i) = cp[name]["theta_min"];
		this->arm_maxt(i) = cp[name]["theta_max"];
	}
	this->calibration_loaded = true;
}

bool Rose::calibrated(void)
{
	return this->calibration_loaded;
}

/////////////////
// ARM IK & FK //
/////////////////

vec Rose::get_end_effector_pos(int linkid)
{
	// solve arm (using D-H notation and forward kinematics)
	vec angles = this->commRecv(span(0, 5));
	vec lengths = this->arm_link_length;

	// get the rotations
	vector<mat> rotate =
	{
		rotationMat(0, 0, angles(0)),
		rotationMat(-angles(1), 0, 0),
		rotationMat(-angles(2), 0, 0),
		rotationMat(-angles(3), 0, 0),
		rotationMat(0, 0, angles(4)),
		rotationMat(0, 0, 0)
	};

	// get the translations
	vector<vec> translate =
	{
		{ 0, 0, lengths(0) },
		{ 0, 0, lengths(1) },
		{ 0, 0, lengths(2) },
		{ 0, 0, lengths(3) },
		{ 0, 0, lengths(4) },
		{ 0, 0, lengths(5) },
		{ 0, 0, lengths(6) }
	};

	// get the position using the combination of rotations
	// and the translations up to the linkid [0|1|2|3|4|5|6]
	vec pos = translate[linkid];
	for (int i = linkid - 1; i >= 0; i--)
	{
		pos = rotate[i] * pos + translate[i];
	}

	return pos;
}

bool Rose::get_arm_position_placement(vec target_pos, vec target_pose, double target_spin, double target_grab, vec &solution_enc)
{
	const int DOF = 6;
	const int JOINT0 = 0;
	const int JOINT1 = 1;
	const int JOINT2 = 2;
	const int JOINT3 = 3;
	const int JOINT4 = 4;
	const int JOINT5 = 5;

	solution_enc = vec(DOF, fill::zeros);
	vec angles(DOF);

	// solve first for the direction of the base
	angles(JOINT0) = angle(target_pos(span(0, 1))); // this is due to the y axis
	target_pose = rotationMat(0, 0, -angles(JOINT0)) * target_pose;
	double r = eucdist(target_pos(span(0, 1)));
	// solve for the height next
	double h = target_pos(2) - sum(this->arm_link_length(span(0, 1)));
	vec interpos({ 0, r, h });

	// grab the target pose and the distance away necessary to make such a pose
	target_pose /= eucdist(target_pose);
	target_pose *= sum(this->arm_link_length(span(4, 6)));
	interpos -= target_pose;

	// find the length
	double l = eucdist(interpos);
	// if longer than the maximum length, just stop trying
	if (l > sum(this->arm_link_length(span(2, 3))))
	{
		return false;
	}

	// determine some offset angle
	double phi = angle(interpos(span(1,2)));

	// calculate the triangle edges
	double link2 = this->arm_link_length(2);
	double link3 = this->arm_link_length(3);
	angles(JOINT1) = 90.0 - (cos_rule_angle(link2, l, link3) + phi); // this is due to the z axis

	angles(JOINT2) = 180.0 - cos_rule_angle(link2, link3, l); // this is due to the z axis

	// calculate the angle of the next joint from the offset angle
	angles(JOINT3) = 90.0 - angle(target_pose(span(1,2)))  - angles(JOINT1) - angles(JOINT2); // this is due to the z axis

	// leave spinning and grabbing up to the programmer
	angles(JOINT4) = target_spin;
	angles(JOINT5) = target_grab;

	// safety checks
	for (int i = 0; i < 6; i++)
	{
		if (!within_value(angles(i), arm_minv(i), arm_maxv(i)))
		{
			return false;
		}
	}

	solution_enc = angles;
	return true;
}