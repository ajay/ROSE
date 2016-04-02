#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include "visual.h"
#include "speech.h"
#include "robot.h"
#include "agent.h"

enum agentstates {
  S_IDLE,
  S_FINDBALL,
  S_GOTOBALL,
  S_PICKBALL,
  S_FINDBASKET,
  S_GOTOBASKET,
  S_DROPBASKET_PHASE1,
  S_DROPBASKET_PHASE2,
  S_DROPBASKET_PHASE3,
  S_DROPBASKET_PHASE4,
  S_DROPBASKET_PHASE5
};
enum objstates {
  S_BALL,
  S_BASKET
};
static bool enable;
static pose3d_t agent_base;
static pose3d_t agent_arm;
// state space (2-level decision resolution)
static const char *task;
static int subtask;
//static int visual_detect_type;
static struct timeval actiontime;
static std::vector<pose3d_t> ballpos;
static int found_ball;
static std::vector<pose3d_t> basketpos;
static int found_basket;
static double find_coeff;

static bool ballfound();
static bool basketfound();

static void conscious_thought(void);
static char *get_speech_command(void);
static double limitf(double x, double min, double max);
static void set_robot(double forward, double left, double raise, double grab);
//static void look_for(int detectingstate);
static void update_object_positions(void);
static int num_balls_in_basket(void);
static std::vector<pose3d_t> get_filtered_ball_positions(void);
static std::vector<pose3d_t> get_filtered_basket_position(void);
static pose3d_t closest_object(std::vector<pose3d_t> locs);
static bool can_pickup(pose3d_t pos);
static bool can_drop(pose3d_t pos);
static double difftime(struct timeval t2, struct timeval t1);
static double dsign(double x);

/** Wake the agent up, start all initial processes
 *  @return 0 on success, -1 on error
 */
int agent::wakeup(void) {
  // start the vision engine(s)
  start_visual();
  // start the speech to text listener
  speech::start();

  task = "fetch";
  subtask = S_IDLE;
  set_enable(true);
  find_coeff = -1.0;
  speech::say("the agent has activated\n");
  return 0;
}

/** Make the agent go to sleep, stop all processes
 */
void agent::gotosleep(void) {
  set_enable(false);
  // stop the vision engine(s)
  stop_visual();
  // stop the speech to text listener
  speech::stop();
}

/** Set the enable for the AI
 *  @param en
 *    true for enable, false for disable
 */
void agent::set_enable(bool en) {
  enable = en;
}

/** Get the poses
 *  @param base
 *    the base struct
 *  @param arm
 *    the arm struct
 *  @return 0
 */
int agent::get_poses(pose3d_t *base, pose3d_t *arm) {
  conscious_thought();
  if (!enable) {
    memset(base, 0, sizeof(pose3d_t));
    memset(arm, 0, sizeof(pose3d_t));
  } else {
    memcpy(base, &agent_base, sizeof(pose3d_t));
    memcpy(arm, &agent_arm, sizeof(pose3d_t));
  }
  return 0;
}

/** Private Functions **/

/** Update the current poses of the robot
 */
static void conscious_thought(void) {
  char *scmd;
  int execute;
  struct timeval currtime;
  // get a speech command, and place that as
  // the priority task
  /*if ((scmd = get_speech_command())) {
    if (strstr(scmd, "fetch")) {
      task = "fetch";
    } else if (strstr(scmd, "stop")) {
      task = "stop";
    }
  }*/

  // FSM (taskgraph)
  execute = 0;
  printf("Updating objects...\n");
  update_object_positions();
  gettimeofday(&currtime, NULL);
  printf("Object positions:\n");
  printf("<ball>\n");
  for (int n = 0; n < (int)ballpos.size(); n++) {
    printf("Ball: %f %f %f\n", ballpos[n].x, ballpos[n].y, ballpos[n].z);
  }
  printf("</ball>\n");
  printf("<basket>\n");
  for (int n = 0; n < (int)basketpos.size(); n++) {
    printf("Basket: %f %f %f\n", basketpos[n].x, basketpos[n].y, basketpos[n].z);
  }
  printf("</basket>\n");

  printf("Updating states...\n");
  while (!execute) {
    if (strcmp(task, "fetch") == 0) {
      // transitions
      // TODO: create a counter resolution for infinite loops
      // TODO: pick up more than 1 ball
      switch (subtask) {
        case S_IDLE:
          printf("State: IDLE\n");
          if (num_balls_in_basket() == 0) {
            subtask = S_FINDBALL;
          } else {
            subtask = S_FINDBASKET;
//              execute = 1;
          }
          break;
        case S_FINDBALL:
          printf("State: FIND BALL\n");
          if (num_balls_in_basket() != 0) {
            subtask = S_IDLE;
          } else if (!ballfound()) {
            execute = 1;
          } else {
            subtask = S_GOTOBALL;
          }
          break;
        case S_GOTOBALL:
          printf("State: GOTO BALL\n");
          if (num_balls_in_basket() != 0) {
            subtask = S_IDLE;
          } else if (!ballfound()) {
            subtask = S_FINDBALL;
          } else if (can_pickup(closest_object(ballpos))) {
            subtask = S_PICKBALL;
          } else {
            execute = 1;
          }
          break;
        case S_PICKBALL:
          printf("State: PICK BALL\n");
          if (num_balls_in_basket() != 0) {
            subtask = S_IDLE;
          } else if (!ballfound()) {
            subtask = S_FINDBALL;
          } else if (!can_pickup(closest_object(ballpos))) {
            subtask = S_GOTOBALL;
          } else {
            execute = 1;
          }
          break;
        case S_FINDBASKET:
          printf("State: FIND BASKET\n");
          if (!basketfound()) {
            execute = 1;
          } else {
            subtask = S_GOTOBASKET;
          }
          break;
        case S_GOTOBASKET:
          printf("State: GOTO BASKET\n");
          if (!basketfound()) {
            subtask = S_FINDBASKET;
          } else if (can_drop(closest_object(basketpos))) {
            // have to do a strange thing here with timers
            gettimeofday(&actiontime, NULL);
            subtask = S_DROPBASKET_PHASE1;
          } else {
            execute = 1;
          }
          break;
        case S_DROPBASKET_PHASE1:
          printf("State: DROP BASKET P1\n");
          if (difftime(currtime, actiontime) < 1.2) {
            execute = 1;
          } else {
            gettimeofday(&actiontime, NULL);
            subtask = S_DROPBASKET_PHASE2;
          }
          break;
        case S_DROPBASKET_PHASE2:
          printf("State: DROP BASKET P2\n");
          if (difftime(currtime, actiontime) < 0.5) {
            execute = 1;
          } else {
            gettimeofday(&actiontime, NULL);
            subtask = S_DROPBASKET_PHASE3;
          }
          break;
        case S_DROPBASKET_PHASE3:
          printf("State: DROP BASKET P3\n");
          if (difftime(currtime, actiontime) < 1.0) {
            execute = 1;
          } else {
            gettimeofday(&actiontime, NULL);
            subtask = S_DROPBASKET_PHASE4;
          }
          break;
        case S_DROPBASKET_PHASE4:
          printf("State: DROP BASKET P4\n");
          if (difftime(currtime, actiontime) < 1.0) {
            execute = 1;
          } else {
            gettimeofday(&actiontime, NULL);
            subtask = S_DROPBASKET_PHASE5;
          }
          break;
        case S_DROPBASKET_PHASE5:
          printf("State: DROP BASKET P5\n");
          if (difftime(currtime, actiontime) < 1.2) {
            execute = 1;
          } else {
            subtask = S_IDLE;
          }
          break;
        default:
          subtask = S_IDLE;
      }
    } else if (strcmp(task, "stop") == 0) {
      subtask = S_IDLE;
      printf("State: IDLE\n");
      execute = 1;
    }
  }

  printf("Executing...\n");

  // move accordingly to the state
  // TODO: do collision detection, stop the robot from moving
  switch (subtask) {
    case S_IDLE:
      set_robot(0.0, 0.0, 0.0, 0.0);
      break;
    case S_FINDBALL:
      set_robot(0.0, 0.30 * find_coeff, 0.0, 0.0);
      break;
    case S_GOTOBALL:
      set_robot(0.32, -ballpos[0].x / 1000.0, -1.0, 0.0);
      find_coeff = dsign(-ballpos[0].x);
      break;
    case S_PICKBALL:
      set_robot(0.30, -ballpos[0].x / 1000.0, -1.0, 1.0);
      find_coeff = dsign(-ballpos[0].x);
      break;
    case S_FINDBASKET:
      set_robot(0.0, -0.30 * find_coeff, 0.0, 0.0);
      break;
    case S_GOTOBASKET:
      set_robot(0.32, -basketpos[0].x / 1200.0, 0.0, 0.0);
      find_coeff = dsign(-basketpos[0].x);
      break;
    case S_DROPBASKET_PHASE1:
      set_robot(0.0, 0.0, 1.0, 0.0);
      break;
    case S_DROPBASKET_PHASE2:
      set_robot(0.40, -basketpos[0].x / 1200.0, 0.0, 0.0);
      find_coeff = dsign(-basketpos[0].x);
      break;
    case S_DROPBASKET_PHASE3:
      set_robot(0.0, 0.0, 0.0, -1.0);
      break;
    case S_DROPBASKET_PHASE4:
      set_robot(-0.40, 0.0, 0.0, 0.0);
      break;
    case S_DROPBASKET_PHASE5:
      set_robot(0.0, 0.0, -1.0, 0.0);
      break;
    default:
      set_robot(0.0, 0.0, 0.0, 0.0);
  }
}

/** Try to get a speech command from a hypothesis string
 *  @param hyp
 *    the hypothesis string
 *  @return the command string
 */
static char *get_speech_command(void) {
  /*const char *commands[] = { "fetch", "return", "stop", NULL };
  char *hyp;
  size_t lastptr;
  int cmdindex;
  int i;
  hyp = speech::listen();
  lastptr = -1;
  for (i = 0; commands[i] != NULL; i++) {
    char *ptr;
    // TODO: resolve last element bug
    ptr = strstr(hyp, commands[i]);
    if ((size_t)ptr - (size_t)hyp > lastptr) {
      lastptr = (size_t)ptr - (size_t)hyp;
      cmdindex = i;
    }
  }
  if (lastptr == (size_t)-1) {
    return NULL;
  } else {
    return (char *)commands[cmdindex];
  }*/
  return speech::listen();
}

/** Limit a value between min and max
 *  @param x
 *    the value
 *  @param min
 *    the lower bound
 *  @param max
 *    the upper bound
 *  @return the limited value
 */
static double limitf(double x, double min, double max) {
  if (x < min) {
    return min;
  } else if (x > max) {
    return max;
  } else {
    return x;
  }
}

/** Set the robot pose in the global space,
 *  where all the values are in the range (-1.0, 1.0)
 *  @param forward
 *    the forward percentage
 *  @param left
 *    the left percentage
 *  @param raise
 *    the raise percentage
 *  @param grab
 *    the grab percentage
 */
static void set_robot(double forward, double left, double raise, double grab) {
  agent_base.y = limitf(forward, -1.0, 1.0);
  agent_base.yaw = limitf(left, -1.0, 1.0);
  agent_arm.pitch = limitf(raise, -1.0, 1.0);
  agent_arm.yaw = limitf(grab, -1.0, 1.0);
}

/** Set the state that the robot is supposed to be looking for
 *  @param detectingstate
 *    either S_BALL or S_BASKET
 */
/*static void look_for(int detectingstate) {
  if (detectingstate == S_BALL && visual_detect_type != S_BALL) {
    visual_detect_type = S_BALL;
    set_detection(DETECT_BALL);
    printf("Detection type set to BALL\n");
  } else if (detectingstate == S_BASKET && visual_detect_type != S_BASKET) {
    visual_detect_type = S_BASKET;
    set_detection(DETECT_BASKET);
    printf("Detection type set to BASKET\n");
  }
}*/

/** Grab all positions in the image
 */
static void update_object_positions(void) {
  pose3d_t *loc;
  int type;
  int found;
  int readdata;
  loc = get_position(&found, &type, &readdata);
  if (!readdata) {
    return;
  }
  switch (type) {
    case 1: //ball
      if (ballpos.size() == 0) {
        ballpos.push_back(*loc);
      } else {
        ballpos[0] = *loc;
      }
      found_ball = found;
      break;

    case 2: //basket
      if (basketpos.size() == 0) {
        basketpos.push_back(*loc);
      } else {
        basketpos[0] = *loc;
      }
      found_basket = found;
      break;
  }
}

/** Get the number of balls in the basket
 *  @return the number of balls
 */
static int num_balls_in_basket(void) {
  const double in_basket_baseline = 100.0; // TODO: CONFIGURE ME!
  int numballs;
  numballs = 0;
  for (int i = 0; i < (int)ballpos.size(); i++) {
    numballs += (ballpos[i].y > in_basket_baseline) ? 1 : 0;
  }
  return numballs;
}

static bool ballfound() {
  return found_ball == 1;
}

static bool basketfound() {
  return found_basket == 1;
}

/** Get all possible ball locations
 *  @return the ball locations
 */
static std::vector<pose3d_t> get_filtered_ball_positions(void) {
  const double in_basket_baseline = 100.0; // TODO: CONFIGURE ME!
  std::vector<pose3d_t> flocs;
  for (int i = 0; i < (int)ballpos.size(); i++) {
    if (ballpos[i].y <= in_basket_baseline) {
      flocs.push_back(ballpos[i]);
    }
  }
  return flocs;
}

/** Get all possible locations of the basket
 *  @return the most likely basket location
 */
static std::vector<pose3d_t> get_filtered_basket_position(void) {
  return basketpos;
}

/** Get the closest object in a location list
 *  @param locs
 *    the location list
 *  @return the closest location
 */
static pose3d_t closest_object(std::vector<pose3d_t> locs) {
  pose3d_t nil;
  memset(&nil, 0, sizeof(pose3d_t));
  if (locs.size() > 0) {
    return locs[0];
  } else {
    return nil;
  }
}

/** Queries if a ball at a position can be picked up
 *  @param pos
 *    the position of the object
 *  @return true for now
 */
static bool can_pickup(pose3d_t pos) {
  return true; // for now, keep running
}

/** Queries if a ball can be dropped into the basket
 *  @param pos
 *    the likely position of the basket
 *  @return true if close enough to drop, otherwise false
 */
static bool can_drop(pose3d_t pos) {
  //pose3d_t *sonar = robot::sense();
  const double collision_baseline = 38.0; // TODO: CONFIGURE ME!
  // dont really need position, just the ultrasonic sensors
//  if (basketpos.size() > 0) {
    //bool left_collision = sonar[0].y <= collision_baseline;
    //bool right_collision = sonar[1].y <= collision_baseline;
    //free(sonar); // TODO: REMOVE WORKAROUND
    return pos.z <= collision_baseline;
//  } else {
//    return false;
//  }
}

/** Get the difference of time in seconds
 *  @param t2
 *    the end time
 *  @param t1
 *    the start time
 *  @return the difference in seconds
 */
static double difftime(struct timeval t2, struct timeval t1) {
  double diff;
  diff = (double)(t2.tv_sec - t1.tv_sec);
  diff += ((double)(t2.tv_usec - t1.tv_usec)) / 1000000;
  return diff;
}

static double dsign(double x) {
  if (x < 0.0) {
    return -1.0;
  } else {
    return 1.0;
  }
}
