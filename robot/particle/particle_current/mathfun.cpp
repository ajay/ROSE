#include "mathfun.h"
#include <cmath>
#include <cassert>

using namespace arma;

double limit_value(double x, double a, double b) {
  return ((x < a) ? a : ((x > b) ? b : x));
}

/** This maps a value from one domain to another
 */
double map_value(double x, double a1, double b1, double a2, double b2) {
  assert(a1 != b1);
  double ratio = (b2 - a2) / (b1 - a1);
  return a2 + ratio * (x - a1);
}

double wrap_value(double x, double a, double b) {
  assert(a < b);
  double diff = b - a;
  while (x < a) {
    x += diff;
  }
  while (x > b) {
    x -= diff;
  }
  return x;
}

int within_value(double x, double a, double b) {
  return a <= x && x <= b;
}

double cos_rule_angle(double A, double B, double C) {
  return acos((A * A + B * B - C * C) / (2.0 * A * B)) * 180 / M_PI;
}

arma::mat genRotateMat(double x, double y, double z) {
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
