#ifndef __TK_ACTIONS_H__
#define __TK_ACTIONS_H__

#include <armadillo>

enum ActionId { STARTING_ACTION = 1, WAIT,
                MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT,
                TURN_LEFT, TURN_RIGHT, GRAB, RELEASE,
                PULL, PUSH, LIFT, DROP };

class BaseAction {
  public:
    double x;
    double y;
    arma::vec pos;
    double t;
    double cost;
    double gcost;
    double hcost;
    enum ActionId id;
    BaseAction(double x, double y, double t, enum ActionId id = STARTING_ACTION);
    ~BaseAction(void);
};

class MotionAction : public BaseAction {
  public:
    MotionAction(double x, double y, enum ActionId id = STARTING_ACTION);
    ~MotionAction(void);
};

std::ostream &operator<<(std::ostream &out, BaseAction &action);

#endif
