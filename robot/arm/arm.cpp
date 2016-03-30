#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <cassert>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <termios.h>
#include <vector>
#include "arm.h"
#include "defs.h"

#define WBUFSIZE  128
#define FPS 10 // send values as fast as possible

// NOTE on units: all units are either degrees or inches unless otherwise specified

using namespace arma;
using namespace std;
using json = nlohmann::json;

static mat genRotateMat(double x, double y, double z);
static double limitf(double value, double minv, double maxv);
static double cos_rule_angle(double A, double B, double C);
static double map_domain(double value, vec from, vec to);
static double secdiff(struct timeval &t1, struct timeval &t2);

Arm::Arm(void) : BaseRobot(ARM)
{
	this->arm_read = zeros<vec>(DOF);
	this->arm_fback = zeros<vec>(DOF);
	this->arm_current = zeros<vec>(2); // change this if necessary
	this->arm_pos = zeros<vec>(3);
	this->arm_mint = zeros<vec>(DOF);
	this->arm_maxt = zeros<vec>(DOF);
	this->arm_minv = zeros<vec>(DOF);
	this->arm_maxv = zeros<vec>(DOF);
	this->arm_link_length = { 0.2, 5.0, 10.5, 6.5, 4.5, 3.8, 3.75 };
	this->calibration_loaded = false;
	this->devmgr = NULL;
	this->manager_running = false;
	this->buffered_arm_theta = zeros<vec>(DOF);
	this->buffered_arm_vel = zeros<vec>(DOF);
	this->buffered_arm_sensors = zeros<vec>(DOF);
	this->buffered_arm_theta_en = false;
	this->buffered_arm_vel_en = false;
	memset(&this->prevwtime, 0, sizeof(struct timeval));
	gettimeofday(&this->prevwtime, NULL);
}

Arm::~Arm(void)
{
	if (this->connected())
	{
		this->send(zeros<vec>(DOF), zeros<vec>(DOF), false, false);
		this->reset();
		this->disconnect();
	}
}

bool Arm::connect(void)
{
	if (!this->connected())
	{
		bool status = BaseRobot::connect();
		if (!this->connected() || !status)
		{
			this->disconnect();
		}
		else
		{
			this->reset();
			this->send(zeros<vec>(DOF), zeros<vec>(DOF));
			this->manager_running = true;
			this->devmgr = new thread(&Arm::update_uctrl, this);
		}
		if (this->connected())
		{
			printf("[ARM] Connected\n");
		}
		else
		{
			fprintf(stderr, "[ARM] Error! Not all devices might not have been connected\n");
		}
	}
	return this->connected();
}

bool Arm::connected(void)
{
	return this->connections.size() > 0;
}

int Arm::numConnected(void)
{
	return this->connections.size();
}

void Arm::disconnect(void)
{
	if (this->manager_running)
	{
		this->manager_running = false;
		this->devmgr->join();
		delete this->devmgr;
		this->devmgr = NULL;
	}
	BaseRobot::disconnect();
	printf("[ARM] Disconnected\n");
}

void Arm::reset(void)
{
	this->arm_read.zeros();
	this->arm_fback.zeros();
}

void Arm::send(
		const vec &arm_theta,
		const vec &arm_vel,
		bool arm_theta_act,
		bool arm_vel_act) {
	assert(arm_theta.n_elem == DOF);
	assert(arm_vel.n_elem == DOF);

	int devid;
	double arm[DOF];
	double omega[DOF];

	// Safety checks
	for (uword i = 0; i < DOF; i++)
	{
		arm[i] = limitf(arm_theta(i), this->arm_mint(i), this->arm_maxt(i));
		if (this->calibrated())
		{
			arm[i] = map_domain(arm[i],
					vec({ this->arm_mint(i), this->arm_maxt(i) }),
					vec({ this->arm_minv(i), this->arm_maxv(i) }));
			arm[i] = limitf(arm[i], this->arm_minv(i), this->arm_maxv(i));
		}
		else
		{
			arm_theta_act = false;
		}
		omega[i] = limitf(arm_vel(i), -1.0, 1.0);
	}

	char instr_activate = 0x80 | ((arm_theta_act && this->calibrated()) ? 0x01 : 0x00) |
		(arm_vel_act ? 0x02 : 0x00);

	char msg[WBUFSIZE];
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		if (this->ids[i] > 1000 && this->ids[i] <= 1002)
		{
			switch ((devid = this->ids[i]))
			{
				case UPPER_ARM:
					sprintf(msg, "[%d %d %d %d %d %d %d %d %d]\n",
							instr_activate,
							(int)(arm[JOINT2]),
							(int)(arm[JOINT3]),
							(int)(arm[JOINT4]),
							(int)(arm[JOINT5]),
							(int)(omega[JOINT2] * 255),
							(int)(omega[JOINT3] * 255),
							(int)(omega[JOINT4] * 255),
							(int)(omega[JOINT5] * 255));
					serial_write(this->connections[i], msg);
					break;

				case LOWER_ARM:
					sprintf(msg, "[%d %d %d %d %d]\n",
							instr_activate,
							(int)(arm[JOINT0]),
							(int)(arm[JOINT1]),
							(int)(omega[JOINT0] * 255),
							(int)(omega[JOINT1] * 255));
					serial_write(this->connections[i], msg);
					break;

				default:
					break;
			}
		}
	}
}

vec Arm::recv(void)
{
	char *msg;
	int devid;
	int storage[8];

	// Read from device
	for (size_t i = 0; i < this->connections.size(); i++)
	{
		if (this->ids[i] > 1000 && this->ids[i] <= 1002)
		{
			switch ((devid = this->ids[i]))
			{
				case UPPER_ARM:
					if ((msg = serial_read(this->connections[i])))
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
							this->arm_read(i + 2) = map_domain(storage[i],
									vec({ this->arm_minv(i + 2), this->arm_maxv(i + 2) }),
									vec({ this->arm_mint(i + 2), this->arm_maxt(i + 2) }));
							this->arm_fback(i + 2) = storage[i + 4];
						}
					}
					break;

				case LOWER_ARM:
					if ((msg = serial_read(this->connections[i])))
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
							this->arm_read(i) = map_domain(storage[i],
									vec({ this->arm_minv(i), this->arm_maxv(i) }),
									vec({ this->arm_mint(i), this->arm_maxt(i) }));
							this->arm_current(i) = storage[i + 2];
							this->arm_fback(i) = storage[i + 4];
						}
					}
					break;

				default:
					break;
			}
		}
	}
	return this->arm_read;
}

void Arm::load_calibration_params(const string &filename)
{
	string params;
	ifstream params_file(filename);
	string temp;
	while (getline(params_file, temp))
	{
		params += temp;
	}
	params_file.close();
	this->set_calibration_params(json::parse(params));
}

void Arm::set_calibration_params(json cp)
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

	for (int i = 0; i < DOF; i++)
	{
		string name = dofnames[i];
		this->arm_minv(i) = cp[name]["raw_min"];
		this->arm_maxv(i) = cp[name]["raw_max"];
		this->arm_mint(i) = cp[name]["theta_min"];
		this->arm_maxt(i) = cp[name]["theta_max"];
	}
	this->calibration_loaded = true;
}

bool Arm::calibrated(void)
{
	return this->calibration_loaded;
}

/** THREADED STUFF **/

void Arm::update_uctrl(void)
{
	while (this->manager_running)
	{
		this->update_send();
		this->update_recv();
	}
}

void Arm::update_send(void)
{
	struct timeval currtime;
	gettimeofday(&currtime, NULL);
	double secs = secdiff(this->prevwtime, currtime);
	if (secs < 1.0 / (double)FPS)
	{
		return;
	}
	else
	{
		memcpy(&this->prevwtime, &currtime, sizeof(struct timeval));
	}
	this->wlock.lock();
	vec arm_theta = this->buffered_arm_theta;
	vec arm_vel = this->buffered_arm_vel;
	bool arm_theta_en = this->buffered_arm_theta_en;
	bool arm_vel_en = this->buffered_arm_vel_en;
	this->wlock.unlock();
	this->send(arm_theta, arm_vel, arm_theta_en, arm_vel_en);
}

void Arm::update_recv(void)
{
	this->recv();
	this->rlock.lock();
	this->buffered_arm_sensors = this->arm_read;
	this->rlock.unlock();
}

void Arm::move(
		const vec &arm_theta,
		const vec &arm_vel,
		bool arm_theta_act,
		bool arm_vel_act)
{
	this->wlock.lock();
	this->buffered_arm_theta = arm_theta;
	this->buffered_arm_vel = arm_vel;
	this->buffered_arm_theta_en = arm_theta_act;
	this->buffered_arm_vel_en = arm_vel_act;
	this->wlock.unlock();
}

vec Arm::sense(void)
{
	this->rlock.lock();
	vec arm_sensors = this->buffered_arm_sensors;
	this->rlock.unlock();
	return arm_sensors;
}

void Arm::set_pose(
		double joint0,
		double joint1,
		double joint2,
		double joint3,
		double joint4,
		double joint5,
		bool en)
{
	this->wlock.lock();
	this->buffered_arm_theta = { joint0, joint1, joint2, joint3, joint4, joint5 };
	this->buffered_arm_vel = zeros<vec>(DOF);
	this->buffered_arm_theta_en = en;
	this->buffered_arm_vel_en = false;
	this->wlock.unlock();
}

void Arm::set_joint(double joint_value, int joint_id)
{
	this->wlock.lock();
	this->buffered_arm_theta(joint_id) = joint_value;
	this->buffered_arm_theta_en = true;
	this->buffered_arm_vel_en = false;
	this->wlock.unlock();
}

/** KINEMATICS STUFF **/

static mat genRotateMat(double x, double y, double z)
{
	// convert the degrees into radians
	x = x * M_PI / 180.0;
	y = y * M_PI / 180.0;
	z = z * M_PI / 180.0;

	mat X = reshape(mat({
				1, 0, 0,
				0, cos(x), -sin(x),
				0, sin(x), cos(x)
				}), 3, 3).t();
	mat Y = reshape(mat({
				cos(y), 0, sin(y),
				0, 1, 0,
				-sin(y), 0, cos(y)
				}), 3, 3).t();
	mat Z = reshape(mat({
				cos(z), -sin(z), 0,
				sin(z), cos(z), 0,
				0, 0, 1
				}), 3, 3).t();
	return Z * Y * X;
}

vec Arm::get_end_effector_pos(int linkid)
{
	// solve arm (using D-H notation and forward kinematics)
	vec angles = this->arm_read;
	vec lengths = this->arm_link_length;

	// get the rotations
	vector<mat> rotate =
	{
		genRotateMat(0, 0, angles(0)),
		genRotateMat(-angles(1), 0, 0),
		genRotateMat(-angles(2), 0, 0),
		genRotateMat(-angles(3), 0, 0),
		genRotateMat(0, 0, angles(4)),
		genRotateMat(0, 0, 0)
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

bool Arm::get_position_placement(vec target_pos, vec target_pose, double target_spin, vec &solution_enc)
{
	solution_enc = vec(DOF, fill::zeros);
	vec angles(DOF);

	// Solve first for the direction of the base
	angles(JOINT0) = atan2(target_pos(1), target_pos(0)) * 180.0 / M_PI - 90.0; // This is due to the y axis
	target_pose = genRotateMat(0, 0, -angles(JOINT0)) * target_pose;
	double r = sqrt(dot(target_pos(span(0, 1)), target_pos(span(0, 1))));
	// Solve for the height next
	double h = target_pos(2) - sum(this->arm_link_length(span(0, 1)));
	vec interpos({ 0, r, h });

	// Grab the target pose and the distance away necessary to make such a pose
	target_pose /= sqrt(dot(target_pose, target_pose));
	target_pose *= sum(this->arm_link_length(span(4, 6)));
	interpos -= target_pose;

	// Find the length
	double l = sqrt(dot(interpos, interpos));
	if (l > sum(this->arm_link_length(span(2, 3))))
	{
		return false;
	}

	// Determine some offset angle
	double phi = atan2(interpos(2), interpos(1)) * 180.0 / M_PI;

	// Calculate the triangle edges
	angles(JOINT0) = atan2(target_pos(1), target_pos(0)) * 180.0 / M_PI - 90.0; // This is due to the y axis
	double link2 = this->arm_link_length(2);
	double link3 = this->arm_link_length(3);
	angles(JOINT1) = 90.0 - (cos_rule_angle(link2, l, link3) + phi); // This is due to the z axis

	// Check to make sure that the second joint with the third link does not hit the backplane
	double basewidth = 6.0;
	double baselength = 4.0;
	double baseheight = this->arm_link_length(3);
	vec side({ 0, 0, baseheight });
	side = genRotateMat(-angles(JOINT1), 0, 0) * side;
	vec leftside = genRotateMat(0, 0, angles(JOINT0)) * (side + vec({ -basewidth, -baselength, 0 }));
	vec rightside = genRotateMat(0, 0, angles(JOINT0)) * (side + vec({ basewidth, -baselength, 0 }));
	if (leftside(1) < -8.0 || rightside(1) < -8.0)
	{
		return false;
	}

	angles(JOINT2) = 180.0 - cos_rule_angle(link2, link3, l); // this is due to the z axis

	// Calculate the angle of the next joint from the offset angle
	angles(JOINT3) = 90.0 - atan2(target_pose(2), target_pose(1)) * 180.0 / M_PI  - angles(JOINT1) - angles(JOINT2); // This is due to the z axis

	// Leave grabbing up to the programmer
	angles(JOINT4) = target_spin;
	angles(JOINT5) = 45.0;

	solution_enc = angles;
	return true;
}

/** STATIC FUNCTIONS **/

static double limitf(double value, double min_value, double max_value)
{
	if (value < min_value) 			{ return min_value; }
	else if (value > max_value)		{ return max_value;	}
	else							{ return value;		}
}

static double cos_rule_angle(double A, double B, double C)
{
	return acos((A * A + B * B - C * C) / (2.0 * A * B)) * 180 / M_PI;
}

static double map_domain(double value, vec from, vec to)
{
	assert(from.n_elem == 2 && to.n_elem == 2);
	value = limitf(value, from(0), from(1));
	double ratio = (to(1) - to(0)) / (from(1) - from(0));
	return (value - from(0)) * ratio + to(0);
}

static double secdiff(struct timeval &t1, struct timeval &t2)
{
	double usec = (double)(t2.tv_usec - t1.tv_usec) / 1000000.0;
	double sec = (double)(t2.tv_sec - t1.tv_sec);
	return sec + usec;
}