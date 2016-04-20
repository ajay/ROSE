#include <random>

#include "highgui.h"
#include "mathfun.h"
#include "sim_robot.h"

using namespace arma;
using namespace std;

sim_robot::sim_robot(sim_map *map)
{
	this->map = map;
	this->x = 0;
	this->y = 0;
	this->t = 0;
	this->r = 0;
	this->vs = 0;
	this->ws = 0;
	this->lidar = NULL;
}

sim_robot::~sim_robot(void)
{
	this->vs = 0;
	this->ws = 0;
}

void sim_robot::set_size(double r)
{
	this->r = r;
}

void sim_robot::set_pose(double x, double y, double t)
{
	this->x = x;
	this->y = y;
	this->t = t;
}

void sim_robot::attach_lidar(void *lidar)
{
	this->lidar = lidar;
}

double limitf(double x, double a, double b)
{
	return (x < a) ? a : (x > b ? b : x);
}

bool sim_robot::collided(double x, double y)
{
	if (this->map == NULL)
	{ // don't care about collisions
		return false;
	}
	if (x < 0 || x >= (double)this->map->n_cols || y < 0 || y >= (double)this->map->n_rows)
	{
		// barrier check
		return false; // always update if it goes out of bounds
	}
	int l = (int)round((double)x - this->r/2);
	int t = (int)round((double)y - this->r/2);
	int r = l + this->r - 1;
	int b = t + this->r - 1;
	l = MAX(0, l);
	t = MAX(0, t);
	r = MIN(this->map->n_cols-1, r);
	b = MIN(this->map->n_rows-1, b);
	mat f = this->map->map.submat(t, l, b, r);
	return (accu(f) > 0);
}

static bool within(double x, double a, double b)
{
	return a <= x && x <= b;
}

static double erfinv(double p)
{
	// approximate maclaurin series (refer to http://mathworld.wolfram.com/InverseErf.html)
	vec a =
	{
		0.88623,
		0.23201,
		0.12756,
		0.086552
	};
	vec x(a.n_elem);
	for (int i = 0; i < x.n_elem; i++)
	{
		x(i) = pow(p, 2 * i + 1);
	}
	return dot(a,x);
}

static double gaussianNoise(double sigma)
{
	double p = (double)rand() / ((double)RAND_MAX / 2) - 1;
	return erfinv(p) * sigma;
}

void sim_robot::move(double vx, double vy, double w)
{
	this->t += w * (1 + gaussianNoise(this->ws));
//			(vx * gaussianNoise(this->vs / 100)) +
//			(vy * gaussianNoise(this->vs / 100)); // arbitrary constant chosen
	double x = this->x + (vy * (1 + gaussianNoise(this->vs))) * cos(deg2rad(this->t)) +
		(vx + gaussianNoise(this->vs)) * sin(deg2rad(this->t));
	double y = this->y + (vy * (1 + gaussianNoise(this->vs))) * sin(deg2rad(this->t)) -
		(vx + gaussianNoise(this->vs)) * cos(deg2rad(this->t));
	if (!this->collided(x, y))
	{
		this->x = x;
		this->y = y;
	}
}

void sim_robot::set_noise(double vs, double ws)
{
	this->vs = vs;
	this->ws = ws;
}

void sim_robot::blit(cube &screen)
{
	int radius = (int)floor(this->r);
	for (int i = 0; i < radius; i++)
	{
		for (int j = 0; j < radius; j++)
		{
			double x_ = (double)j - this->r/2;
			double y_ = (double)i - this->r/2;
			if (!within(eucdist({x_,y_}), 0, this->r/2))
			{
				continue;
			}
			int x = (int)round(x_ + this->x);
			int y = (int)round(y_ + this->y);
			if (!within(x, 0, screen.n_cols-1) || !within(y, 0, screen.n_rows-1))
			{
				continue;
			}
			screen(y, x, 0) = 0;
			screen(y, x, 1) = 1;
			screen(y, x, 2) = 1;
		}
	}
	for (int i = 0; i < radius/2; i++)
	{
		int x = (int)round((double)i * cos(deg2rad(this->t)) + this->x);
		int y = (int)round((double)i * sin(deg2rad(this->t)) + this->y);
		if (!within(x, 0, screen.n_cols-1) || !within(y, 0, screen.n_rows-1))
		{
			continue;
		}
		screen(y, x, 0) = 1;
		screen(y, x, 1) = 0;
		screen(y, x, 2) = 1;
	}
}