#include "robot.h"

using namespace arma;
using namespace std;

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

xbot::xbot(sim_map *map) :
  pos(2, fill::zeros),
  theta(0),
  radius(0),
  sigma(2, fill::zeros),
  health(1) {
  this->map = map;
}

xbot::~xbot(void) {
}

void xbot::set_size(double r) {
  this->radius = r;
}

void xbot::set_apriori(double x, double y, double t) {
  this->pos = { x, y };
  this->theta = t;
}

void xbot::set_noise(double vs, double ws) {
  this->sigma = { vs, ws };
}

static double erfinv(double p) {
  // approximate maclaurin series (refer to http://mathworld.wolfram.com/InverseErf.html)
  double sqrt_pi = sqrt(M_PI);
  vec a = {
    0.88623,
    0.23201,
    0.12756,
    0.086552
  };
  vec x(a.n_elem);
  for (int i = 0; i < x.n_elem; i++) {
    x(i) = pow(p, 2 * i + 1);
  }
  return dot(a,x);
}

static double gaussianNoise(double sigma) {
  double p = (double)rand() / ((double)RAND_MAX / 2) - 1;
  return erfinv(p) * sigma;
}

void xbot::update_belief(double v, double w) {
  // make it simple, just add it
  this->theta += w * (1 + gaussianNoise(this->sigma(1))) +
    v/4 * gaussianNoise(this->sigma(1));
  double x = this->pos(0) + v * (1 + gaussianNoise(this->sigma(0))) * cos(this->theta);
  double y = this->pos(1) + v * (1 + gaussianNoise(this->sigma(0))) * sin(this->theta);
  this->pos = vec({ x, y });
}

void xbot::update_observations(vector<vec> observations) {
  int total_health = 0;
  mat map = this->map->map;
  for (vec &pt : observations) {
    double angle = pt(1) + this->theta;
    double r = pt(0);
    int x = (int)round(this->pos(0) + r * cos(angle));
    int y = (int)round(this->pos(1) + r * sin(angle));
    if (0 > x || x >= (int)map.n_cols ||
        0 > y || y >= (int)map.n_rows) {
      continue;
    }
    int radius = 1; // account for error
    int left = MAX(x - radius, 0);
    int right = MIN(x + radius, map.n_cols - 1);
    int top = MAX(y - radius, 0);
    int bottom = MIN(y + radius, map.n_rows - 1);
    if (accu(map.submat(top, left, bottom, right)) > 0) {
      total_health++;
    }
  }
  this->health = (double)total_health;
}
