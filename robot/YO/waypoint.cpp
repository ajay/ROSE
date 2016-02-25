#include "waypoint.h"

waypoint::waypoint(double x, double y) {
  this->x = x;
  this->y = y;
}

bool waypoint::completed(vec pos) {
  // static definition for now
  const double tolerance = 1.5; // 1.5 inches

  vec diff = pos - vec({ this->x, this->y });
  double dist = sqrt(dot(diff, diff));
  if (dist < tolerance) { // we good dawg!
    return true;
  } else {
    return false;
  }
}
