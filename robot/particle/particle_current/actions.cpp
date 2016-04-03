#include "actions.h"
#include <iostream>
#include <string>

using namespace std;

vector<string> names = { "STARTING_ACTION", "WAIT",
  "MOVE_FORWARD", "MOVE_BACKWARD", "MOVE_LEFT", "MOVE_RIGHT",
  "TURN_LEFT", "TURN_RIGHT", "GRAB", "RELEASE",
  "PULL", "PUSH", "LIFT", "DROP" };

BaseAction::BaseAction(double x, double y, double t, enum ActionId id) {
  this->x = x;
  this->y = y;
  this->pos = arma::vec({ x, y });
  this->t = t;
  this->id = id;
  this->cost = 0;
  this->gcost = 0;
  this->hcost = 0;
}

BaseAction::~BaseAction(void) {
}

MotionAction::MotionAction(double x, double y, enum ActionId id) :
    BaseAction(x, y, 0, id) {
}

MotionAction::~MotionAction(void) {
}

ostream &operator<<(ostream &out, BaseAction &action) {
  out << "{ " << names[(int)action.id-1] <<
    ": [x: " << action.x << ", y: " << action.y << ", t: " << action.t <<
    ", c(x): " << action.cost << ", g(x): " << action.gcost << ", h(x): " << action.hcost << " }\n";
}
