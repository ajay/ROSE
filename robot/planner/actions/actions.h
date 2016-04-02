#ifndef __TK_ACTIONS_H__
#define __TK_ACTIONS_H__

#include <armadillo>

enum ActionId { WAIT = 1, MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT,
                TURN_LEFT, TURN_RIGHT, BUTTON_PRESS, PICK_UP_OBJECT };

class BaseAction {
  public:
    double x;
    double y;
    double theta;
    double cost;
    double gcost;
    double hcost;
    enum ActionId id;
    BaseAction(double x, double y, double theta);
    ~BaseAction(void);
};

class MotionAction : BaseAction {
  public:
    MotionAction(double x, double y);
    ~MotionAction(void);
};

#endif
