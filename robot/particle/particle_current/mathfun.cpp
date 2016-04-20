#include "mathfun.h"
#include <cmath>
#include <cassert>

using namespace arma;

double limit_value(double x, double a, double b)
{
	return ((x < a) ? a : ((x > b) ? b : x));
}

/** This maps a value from one domain to another
 */
double map_value(double x, double a1, double b1, double a2, double b2)
{
	assert(a1 != b1);
	double ratio = (b2 - a2) / (b1 - a1);
	return a2 + ratio * (x - a1);
}

double wrap_value(double x, double a, double b)
{
	assert(a < b);
	double diff = b - a;
	// TODO: inefficient, fix later
	int ratio = (int)(x / diff);
	x -= (double)ratio * diff;
	while (x < a)
	{
		x += diff;
	}
	while (x > b)
	{
		x -= diff;
	}
	return x;
}

int within_value(double x, double a, double b)
{
	return a <= x && x <= b;
}

double rad2deg(double x)
{
	return x * 180 / M_PI;
}

double deg2rad(double x)
{
	return x * M_PI / 180;
}

double eucdist(vec v)
{
	return sqrt(dot(v, v));
}

double angle(vec v)
{
	assert(v.n_elem == 2);
	return rad2deg(atan2(v(1), v(0)));
}

double cos_rule_angle(double A, double B, double C)
{
	return rad2deg(acos((A * A + B * B - C * C) / (2.0 * A * B)));
}

arma::mat rotationMat(double x, double y, double z)
{
	// convert the degrees into radians
	x = deg2rad(x);
	y = deg2rad(y);
	z = deg2rad(z);

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