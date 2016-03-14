#include "sim_landmark.h"

using namespace arma;

static bool within(int x, int a, int b) {
  return a <= x && x <= b;
}

sim_landmark::sim_landmark(double x, double y) {
  this->x = x;
  this->y = y;
}

double sim_landmark::collision(sim_map *map, vec pos) {
  if (this->y == pos(1) && this->x == pos(0)) {
    return 0;
  }
  vec unit({ this->x - pos(0), this->y - pos(1) });
  int maxr = (int)(map->n_rows + map->n_cols);
  double radius = sqrt(dot(unit, unit));
  unit /= radius;

  for (int r = 0; r < maxr; r++) {
    vec trajectory = unit * r + pos;
    ivec t({ (sword)round(trajectory(0)), (sword)round(trajectory(1)) });
    ivec xpos({ t(0)-1, t(0), t(0), t(0), t(0)+1 });
    ivec ypos({ t(1), t(1), t(1)+1, t(1)-1, t(1) });
    int total = 0;
    int nelem = 0;
    for (int i = 0; i < 5; i++) {
      if (!within(xpos(i), 0, map->n_cols-1) ||
          !within(ypos(i), 0, map->n_rows-1)) {
        continue;
      }
      total += map->map(ypos(i), xpos(i));
      nelem++;
    }
    if (nelem == 0) {
      break;
    }
    if (total > 0) {
      return r;
    }
  }
  return 10000;
}

vec sim_landmark::sense(sim_robot &robot, mat lidarvals, int flags) {
  if (flags & 0x01) {
    return vec({ -1, -1 }); // TODO: make better analysis later
  } else {
    //double radius = landmarks[lid].collision(&map, vec({ robot.x, robot.y })); // add this in for observability test
    vec diff = vec({ robot.x, robot.y }) - vec({ this->x, this->y });
    double radius = sqrt(dot(diff, diff));
    double theta = atan2(diff(1), diff(0)) - robot.t;
    return vec({ radius * cos(theta), radius * sin(theta) });
  }
}

void sim_landmark::blit(icube &screen) {
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int x = (int)round(this->x) + j;
      int y = (int)round(this->y) + i;
      if (!within(x, 0, screen.n_cols-1) || !within(y, 0, screen.n_rows-1)) {
        continue;
      }
      screen(y, x, 0) = 0;
      screen(y, x, 1) = 0;
      screen(y, x, 2) = 255;
    }
  }
}
