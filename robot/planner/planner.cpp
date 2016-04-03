#include "planner.h"
#include "visual/visual.h"  // for vision
#include "speech/speech.h"  // for audio
#include "robot/robot.h"    // for robot movement and touch
#include "slam/slam.h"      // for localization
#include "manual.h"

int planner::init(BaseRobot *robot, bool en) {
  body = robot;
  enable = en;
  keepRunning = true;
}

void planner::start(void) {
  // one thread to localize and one thread to plan
  planningThread = new thread(planner::run);
}

void planner::stop(void) {
  keepRunning = false;
  if (planningThread) {
    planningThread->join();
    delete planningThread;
    planningThread = NULL;
  }
}

void planner::destroy(void) {
  planner::stop();
}

void planner::run(void) {
  // load a simple action graph
  while (keepRunning) {
    /////////////////////
    //    DANGEROUS    //
    /////////////////////
    if (autonomousEnable) {
      // do nothing for now, this is where the autonomous section is
      // create a motion list later on, but this will have to be strange
    } else {
      // manual control
      manual_input::update(); // autoconnect
      manual_input::control_robot(body);
    }
  }
}
