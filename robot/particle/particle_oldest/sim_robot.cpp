#include "sim_robot.h"
#include "highgui.h"

using namespace arma;

sim_robot::sim_robot(sim_map *map) {
  this->map = map;
  this->x = 0;
  this->y = 0;
  this->t = 0;
  this->r = 0;
  this->vs = 0;
  this->ws = 0;
  this->lidar = NULL;
}

sim_robot::~sim_robot(void) {
  this->vs = 0;
  this->ws = 0;
}

void sim_robot::set_size(double r) {
  this->r = r;
}

void sim_robot::set_pose(double x, double y, double t) {
  this->x = x;
  this->y = y;
  this->t = t;
}

void sim_robot::attach_lidar(void *lidar) {
  this->lidar = lidar;
}

double limitf(double x, double a, double b) {
  return (x < a) ? a : (x > b ? b : x);
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

bool sim_robot::collided(double x, double y) {
  if (this->map == NULL) { // don't care about collisions
    return false;
  }
  if (x < 0 || x >= (double)this->map->n_cols ||
      y < 0 || y >= (double)this->map->n_rows) {
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

static bool within(double x, double a, double b) {
  return a <= x && x <= b;
}

void sim_robot::move(double v, double w) {
  v = limitf(v, -2, 2); // limit movement for PHYSICAL reasons
  this->t += w + gaussianNoise(this->ws) + v/4 * gaussianNoise(this->ws);
  double x = this->x + (v + gaussianNoise(this->vs)) * cos(this->t);
  double y = this->y + (v + gaussianNoise(this->vs)) * sin(this->t);
  if (!this->collided(x, y)) {
    this->x = x;
    this->y = y;
  }
}

void sim_robot::set_noise(double vs, double ws) {
  this->vs = vs;
  this->ws = ws;
}

void sim_robot::blit(icube &screen) {
  int radius = (int)floor(this->r);
  for (int i = 0; i < radius; i++) {
    for (int j = 0; j < radius; j++) {
      double x_ = (double)j - this->r/2;
      double y_ = (double)i - this->r/2;
      if (!within(sqrt(x_*x_+y_*y_), 0, this->r/2)) {
        continue;
      }
      int x = (int)round(x_ + this->x);
      int y = (int)round(y_ + this->y);
      if (!within(x, 0, screen.n_cols-1) || !within(y, 0, screen.n_rows-1)) {
        continue;
      }
      screen(y, x, 0) = 0;
      screen(y, x, 1) = 255;
      screen(y, x, 2) = 255;
    }
  }
  for (int i = 0; i < radius/2; i++) {
    int x = (int)round((double)i * cos(this->t) + this->x);
    int y = (int)round((double)i * sin(this->t) + this->y);
    if (!within(x, 0, screen.n_cols-1) || !within(y, 0, screen.n_rows-1)) {
      continue;
    }
    screen(y, x, 0) = 255;
    screen(y, x, 1) = 0;
    screen(y, x, 2) = 255;
  }
}
