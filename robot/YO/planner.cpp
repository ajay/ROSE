#include "robot.h"
#include "pfilter.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

static waypoint *target;
static waypoint *last;
static thread *planning_thread;

void planner::init(planning_constraints constraints, planning_routines routines) {
	// set up the particle filter for localization extraction
  perception::init();
  planner::init_planning_constraints(constraints);
  planner::init_motion_routines(routines);
}

/** The purpose of the planning constraints is to
 *  specify the initial constraints which apply
 *  to the robot
 */
void planner::init_planning_constraints() {
  // target needs to be set to something
  // start the map here
}

/** Construct static initial waypoints for the planner
 *  to invoke
 */
void planner::load_waypoints(vector<waypoint *> &pts) {
  for (waypoint *pt : pts) {
    if (!target) {
      target = new waypoint(*pt);
      last = target;
    } else {
      // for now only make it a linked list, no decision making needs to be done for now
      last->next.push_back(waypoint(*pt));
      last = last->next[0];
    }
  }
}

void planner::
