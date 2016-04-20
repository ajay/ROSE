#include "sim_landmark.h"
#include "mathfun.h"
#include "draw.h"

using namespace arma;

sim_landmark::sim_landmark(double x, double y)
{
	this->x = x;
	this->y = y;
}

double sim_landmark::collision(sim_map *map, vec pos)
{
	if (this->y == pos(1) && this->x == pos(0))
	{
		return 0;
	}
	vec unit({ this->x - pos(0), this->y - pos(1) });
	int maxr = (int)(map->n_rows + map->n_cols);
	double radius = eucdist(unit);
	unit /= radius;

	for (int r = 0; r < maxr; r++)
	{
		vec trajectory = unit * r + pos;
		ivec t({ (sword)round(trajectory(0)), (sword)round(trajectory(1)) });
		ivec xpos({ t(0)-1, t(0), t(0), t(0), t(0)+1 });
		ivec ypos({ t(1), t(1), t(1)+1, t(1)-1, t(1) });
		int total = 0;
		int nelem = 0;
		for (int i = 0; i < 5; i++)
		{
			if (!within_value(xpos(i), 0, (int)map->n_cols-1) || !within_value(ypos(i), 0, (int)map->n_rows-1))
			{
				continue;
			}
			total += map->map(ypos(i), xpos(i));
			nelem++;
		}
		if (nelem == 0)
		{
			break;
		}
		if (total > 0)
		{
			return r;
		}
	}
	return 10000;
}

vec sim_landmark::sense(sim_robot &robot, mat lidarvals, int flags)
{
	if (flags & 0x01)
	{
		return vec({ -1, -1 }); // TODO: make better analysis later
	}
	else
	{
		vec diff = vec({ this->x - robot.x, this->y - robot.y });
		double radius = eucdist(diff);// + gauss(0, 1.0);
		double theta = wrap_value(angle(diff) - robot.t, -180, 180);// + gauss(0, 2.0);
		return vec({ radius, theta });
	}
}

void sim_landmark::blit(cube &screen, int mux, int muy, vec place_circle)
{
	vec white({ 1, 1, 1 });
	int x_ = (int)round(this->x) - mux + (int)screen.n_cols / 2;
	int y_ = (int)round(this->y) - muy + (int)screen.n_rows / 2;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int x = x_ + j;
			int y = y_ + i;
			if (!within_value(x, 0, (int)screen.n_cols-1) || !within_value(y, 0, (int)screen.n_rows-1))
			{
				continue;
			}
			screen(y, x, 0) = 0;
			screen(y, x, 1) = 0;
			screen(y, x, 2) = 1;
		}
	}
	if (place_circle(2) > 0.5)
	{
		draw_circle(screen, white, vec({ (double)x_, (double)y_ }), eucdist(place_circle.subvec(0,1)));
	}
}