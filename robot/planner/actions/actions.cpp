#include "actions.h"

BaseAction::BaseAction(double x, double y, double theta) {
  this->x = x;
  this->y = y;
  this->theta = theta;
}

BaseAction::~BaseAction(void) {
}

MotionAction::MotionAction(double x, double y) :
    BaseAction(x, y, 0) {
}

MotionAction::~MotionAction(void) {
}
