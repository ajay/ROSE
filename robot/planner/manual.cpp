#include "interface/xboxctrl.h" // input device O_O

// ROBOTS
#include "robot/tachikoma/tachikoma.h"
#include "robot/armv1/armv1.h"

// this is a manual interface for the robot to be controlled
// change as necessary I guess...

// Sometimes people wonder why we can't use a keyboard
// WELL TOO BAD

// this is an xbox controller! ^_^
static xboxctrl_t xboxjoy;

// What?! Connect to the XBOX Controller?!
void manual_input::connect(void) {
  xboxctrl_connect(&xboxjoy);
}

// Nope.
void manual_input::disconnect(void) {
  xboxctrl_disconnect(&xboxjoy);
}

// Look at the controller
void manual_input::update(void) {
  // connect if not connected
  if (!xboxjoy.connected) {
    xboxctrl_connect(&xboxjoy);
  }
  // update the stupid thingy
  xboxctrl_update(&xboxjoy);
}

// USE IT!!!
void manual_input::control_robot(BaseRobot *robot) {
  if (robot->robotid == TACHIKOMA) {
    Tachikoma *bot = (Tachikoma *)robot;
  } else if (robot->roboid == ARMV1) {
    Armv1 *bot = (Armv1 *)robot;
  }
}
