#include "robot.h"
#include "pfilter.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

void planner::init(void) {
	// set up the particle filter for localization extraction
	pfilter localizer;
}