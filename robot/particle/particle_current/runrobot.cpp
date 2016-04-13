#include <cstdio>
#include <armadillo>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <signal.h>
#include <unistd.h>
#include "SDL/SDL.h"

#include "chili_landmarks.h"
#include "Rose.h"
#include "sim_map.h"
#include "sim_robot.h"
#include "sim_landmark.h"
#include "sdldef.h"
#include "draw.h"
#include "pfilter.h"
#include "astar.h"
#include "mathfun.h"
#include "ipcdb.h"

using namespace std;
using namespace arma;

// threads in this file
void manual_input(void);
void chilitag_detect(void);
void localize_pose(void);
void robot_calcmotion(void);
void motion_plan(void);
void display_interface(void);

static void stoprunning(int signum) {
  stopsig = 1;
  rose.startStop = true;
  alarm(1); // hack to get this to work
}

static void forcequit(int signum) {
  printf("Force quitting...\n");
  exit(1);
}

static SDL_Surface *initSDL(int w, int h)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return NULL;
	}

	SDL_Surface *screen = SDL_SetVideoMode(w,h, 32, SDL_SWSURFACE);

	if(screen == NULL)
	{
		return NULL;
	}

	SDL_WM_SetCaption("rose", NULL);

	return screen;
}

static void screenblit(SDL_Surface *s, cube &frame) {
  for (int i = 0; i < (int)frame.n_rows; i++) {
    for (int j = 0; j < (int)frame.n_cols; j++) {
      uint32_t color =
        ((uint8_t)((int)round(frame(i,j,0) * 255)) << 16) |
        ((uint8_t)((int)round(frame(i,j,1) * 255)) << 8) |
        ((uint8_t)((int)round(frame(i,j,2) * 255)) << 0);
      ((uint32_t *)s->pixels)[XY2P(j, i, s->w, s->h)] = color;
    }
  }
}

int main() {
  // preemptive init
  printf("[main] preemptive init\n");
  signal(SIGINT, stoprunning);
  signal(SIGALRM, forcequit);
  screen = initSDL(500, 500);
  if (!screen) {
    printf("No screen found, please check your SDL configurations\n");
    return 1;
  }
  robot_pose = vec({ 40, 40, 90 * M_PI / 180 });
  globalmap.load("map_engineering_b_wing.jpg");

  // start up the threads
  printf("[main] start up the threads\n");
  rose.startStop = false;
  thread manual_thread(manual_input);
  thread chili_thread(chilitag_detect);
  thread pose_thread(localize_pose);
  thread path_thread(motion_plan);
  thread robot_thread(robot_calcmotion);
  thread display_thread(display_interface);

  // wait until program closes
  printf("[main] wait until program closes\n");
  while (!stopsig);
  alarm(1);

  // close all threads
  printf("[main] close all threads\n");
  rose.set_wheels(0, 0, 0, 0);
  rose.stop_arm();
  rose.startStop = true;
  rose.disconnect();
  chili_thread.join();
  pose_thread.join();
  path_thread.join();
  robot_thread.join();
  display_thread.join();
  SDL_Quit();

  printf("Closed successfully.\n");
  return 0;
}

void manual_input(void) {
#define posedge(x) (keystates[(x)] && !prevstates[(x)])
#define negedge(x) (!keystates[(x)] && prevstates[(x)])
#define removekey(x) { for (int i = 0; i < 256; i++) { \
  if (keyQueue[i] == (x)) { \
    keyQueue.erase(keyQueue.begin() + i); \
    break; }}}
  vector<Uint8> keyQueue;
  Uint8 prevstates[256];
  memset(prevstates, 0, sizeof(prevstates));

  while (!stopsig) {
    SDL_PumpEvents();
    Uint8 *keystates = SDL_GetKeyState(NULL);

    // detect for exit
    if (keystates[SDLK_x]) {
      auto_enable = false;
      auto_confirmed = false;
      manual_confirmed = true;
      rose.set_wheels(0, 0, 0, 0);
      rose.stop_arm();
      rose.startStop = true;
      kill(getpid(), SIGINT);
      continue;
    }

    // detect for autonomous enable or disable
    if (keystates[SDLK_m]) {
      autonomous_lock.lock();
      auto_enable = false;
      auto_confirmed = false;
      manual_confirmed = true;
      autonomous_lock.unlock();
    } else if (keystates[SDLK_n]) {
      autonomous_lock.lock();
      auto_enable = true;
      auto_confirmed = false;
      manual_confirmed = true;
      autonomous_lock.unlock();
    }

    // if not manual en, then continue
    if (auto_enable || !auto_confirmed) {
      continue;
    }

    for (int i=0; i<keyQueue.size(); i++)
    {
      //printf("Queue: %c\n", keyQueue[i]);
    }

    // input manual feedback
        if (keystates[SDLK_q]) rose.set_wheels(-1, 1, -1, 1); 
        else if (keystates[SDLK_e]) rose.set_wheels(1, -1, 1, -1); 
        else if (keystates[SDLK_a]) rose.set_wheels(-1, 1, 1, -1); 
        else if (keystates[SDLK_d]) rose.set_wheels(1, -1, -1, 1); 
        else if (keystates[SDLK_s]) rose.set_wheels(-1, -1, -1, -1); 
        else if (keystates[SDLK_w]) rose.set_wheels(1, 1, 1, 1); 
        else if (keystates[SDLK_1]) rose.set_wheels(1, 0, 0, 0); 
        else if (keystates[SDLK_2]) rose.set_wheels(0, 1, 0, 0); 
        else if (keystates[SDLK_3]) rose.set_wheels(0, 0, 1, 0); 
        else if (keystates[SDLK_4]) rose.set_wheels(0, 0, 0, 1); 
        else rose.set_wheels(0, 0, 0, 0);

/*
    if (posedge(SDLK_q)) keyQueue.push_back(SDLK_q);
    if (posedge(SDLK_e)) keyQueue.push_back(SDLK_e);
    if (posedge(SDLK_a)) keyQueue.push_back(SDLK_a);
    if (posedge(SDLK_d)) keyQueue.push_back(SDLK_d);
    if (posedge(SDLK_s)) keyQueue.push_back(SDLK_s);
    if (posedge(SDLK_w)) keyQueue.push_back(SDLK_w);
    if (posedge(SDLK_1)) keyQueue.push_back(SDLK_1);
    if (posedge(SDLK_2)) keyQueue.push_back(SDLK_2);
    if (posedge(SDLK_3)) keyQueue.push_back(SDLK_3);
    if (posedge(SDLK_4)) keyQueue.push_back(SDLK_4);
    if (negedge(SDLK_q)) removekey(SDLK_q);
    if (negedge(SDLK_e)) removekey(SDLK_e);
    if (negedge(SDLK_a)) removekey(SDLK_a);
    if (negedge(SDLK_d)) removekey(SDLK_d);
    if (negedge(SDLK_s)) removekey(SDLK_s);
    if (negedge(SDLK_w)) removekey(SDLK_w);
    if (negedge(SDLK_1)) removekey(SDLK_1);
    if (negedge(SDLK_2)) removekey(SDLK_2);
    if (negedge(SDLK_3)) removekey(SDLK_3);
    if (negedge(SDLK_4)) removekey(SDLK_4);

    if (keyQueue.size() > 0) {
      switch (keyQueue[0]) {
        case SDLK_q: rose.set_wheels(-1, 1, -1, 1); break;
        case SDLK_e: rose.set_wheels(1, -1, 1, -1); break;
        case SDLK_a: rose.set_wheels(-1, 1, 1, -1); break;
        case SDLK_d: rose.set_wheels(1, -1, -1, 1); break;
        case SDLK_s: rose.set_wheels(-1, -1, -1, -1); break;
        case SDLK_w: rose.set_wheels(1, 1, 1, 1); break;
        case SDLK_1: rose.set_wheels(1, 0, 0, 0); break;
        case SDLK_2: rose.set_wheels(0, 1, 0, 0); break;
        case SDLK_3: rose.set_wheels(0, 0, 1, 0); break;
        case SDLK_4: rose.set_wheels(0, 0, 0, 1); break;
        default: rose.set_wheels(0, 0, 0, 0); break;
      }
    } else {
      rose.set_wheels(0, 0, 0, 0);
    }*/

    memcpy(prevstates, keystates, sizeof(prevstates));
  }
}

void chilitag_detect(void) {
  // create a chilitag object
  chili_landmarks chili;

  while (!stopsig) {
    // update the chilitags
    chili.update();

    // place the chilitags' positions into a matrix
    mat sv(3, 20, fill::zeros);
    for (int j = 0; j < 20; j++) {
      if (chili.tags[j][0] != 0.0) {
        sv.col(j) = vec({ chili.tags[j][1], chili.tags[j][2], chili.tags[j][0] });
      }
    }

    // store the matrix
    chili_lock.lock();
    chilitags = sv;
    chili_lock.unlock();
  }
}

void localize_pose(void) {
  // create the landmarks (custom)
  landmarks.push_back(sim_landmark(90, 202));           // 00
  landmarks.push_back(sim_landmark(601, 617));          // 01
  landmarks.push_back(sim_landmark(0, 416));            // 02
  landmarks.push_back(sim_landmark(98, 627));           // 03
  landmarks.push_back(sim_landmark(384, 627));          // 04
  landmarks.push_back(sim_landmark(1225, 627));         // 05
  landmarks.push_back(sim_landmark(3400, 513));         // 06
  landmarks.push_back(sim_landmark(3442, 583));         // 07
  landmarks.push_back(sim_landmark(1961, 539));         // 08
  landmarks.push_back(sim_landmark(3442-460, 539+80));  // 09
  landmarks.push_back(sim_landmark(0, 517));            // 10
  landmarks.push_back(sim_landmark(78, 202));           // 11
  landmarks.push_back(sim_landmark(111, 627));          // 12
  landmarks.push_back(sim_landmark(384, 707));          // 13
  landmarks.push_back(sim_landmark(601, 537));          // 14
  landmarks.push_back(sim_landmark(1165, 540));         // 15
  landmarks.push_back(sim_landmark(1961, 539+80));      // 16
  landmarks.push_back(sim_landmark(3442, 583-37));      // 17
  landmarks.push_back(sim_landmark(3400+14, 513));      // 18
  landmarks.push_back(sim_landmark(3442-460, 539));     // 19

  // start the particle filter
  pose_lock.lock();
  pf = pfilter(500, &globalmap, landmarks, robot_pose(0), robot_pose(1), robot_pose(2), 10);
  pose_lock.unlock();

  // loop on the particle filter for updates on the location
  vec mu;
  mat sigma;
  while (!stopsig) {
    // move the robot
    vec enc = rose.recv();
    pf.move(enc);

    // get the chilitags
    chili_lock.lock();
    mat landmarks = chilitags;
    chili_lock.unlock();

    // observe and predict the robot's new location
    pf.observe(landmarks);
    pf.predict(mu, sigma);

    // store the new location
    pose_lock.lock();
    robot_pose = mu;
    pose_lock.unlock();
  }
}

void robot_calcmotion(void) {
  while (!stopsig) {

    autonomous_lock.lock();
    if (!auto_enable)
    {
      autonomous_lock.unlock();
      continue;
    }
    autonomous_lock.unlock();

    // get the current position of the robot
    pose_lock.lock();
    vec pose = robot_pose;
    pose_lock.unlock();
    vec pos = pose(span(0,1));

    // get the current plan
    path_lock.lock();
    mat path_plan = pathplan;
    vec pose_plan = poseplan;
    bool do_pose = dopose;
    double twist = twistplan;
    double grab = grabplan;
    path_lock.unlock();
    if (path_plan.n_cols == 0) {
      // if there is no path, stop the robot for now
      rose.set_wheels(0, 0, 0, 0);
      continue;
    }

    if (path_plan.n_cols == 2) {
      printf("FUCK THIS SHIT, I'M OUT\n");
      rose.set_wheels(0, 0, 0, 0);
      continue;
    }

    vec distance(path_plan.n_cols);

    // do calculation of the wheels
    // first find the closest waypoint as the "start"
    mat diffs = path_plan - repmat(pos, 1, path_plan.n_cols);
    uword target_index = 0;
    // only if the planner is too slow do the following:
    /*for (int j = 0; j < path_plan.n_cols; j++) {
      vec a = diffs.col(j);
      vec b = diffs.col(target_index);
      distance[j] = sqrt(dot(a, a));
      if (sqrt(dot(a, a)) < sqrt(dot(b, b))) {
        target_index = j;
      }
    }*/
    for (int j = 0; j < path_plan.n_cols; j++) {
      vec a = diffs.col(j);
      distance[j] = sqrt(dot(a, a));
    }

    // acceptance radius (just for the end, stop)
    if (distance[target_index] < 20)
    {
      //if (target_index >= (int)distance.n_elem) {
      if (distance.n_elem == 2) {
        printf("target reached\n");
        target_index--;
        rose.set_wheels(0, 0, 0, 0);
        continue;
      } else {
        target_index++;
      }
    }

    // angle check
    vec diff = path_plan(target_index) - pos;
    if (!within_value(atan2(diff(1), diff(0)) * 180 / M_PI, -45, 45) && within_value(sqrt(dot(diff, diff)), 0, 40)) {
      target_index++;
      if (target_index >= (int)distance.n_elem) {
        printf("target reached\n");
        target_index--;
        rose.set_wheels(0, 0, 0, 0);
        continue;
      }
    }

    vec target = path_plan.col(target_index);

    // once the target is found, then calculate the trajectory
    double theta_k_p = 0.025;
    diff = target - pos;
    double delta_theta = atan2(diff(1), diff(0)) * 180 / M_PI - 180 *pose(2) / M_PI;
    double v_l, v_r;
    v_l = (1.0 - theta_k_p * delta_theta);
    // - theta_k_i * r.integral_error
    // - theta_k_d * r.delta_theta_diff);
    v_r = (1.0 + theta_k_p * delta_theta);
    // + r.theta_k_i * r.integral_error
    // + r.theta_k_d * r.delta_theta_diff);

    cout << "current angle: " << pose(2) << endl;
    cout << "delta theta: " << delta_theta << endl;
    cout << "left vel: " << v_l << endl << "right vel: " << v_r << endl;
    cout << "waypoint: " << endl << path_plan.col(path_plan.n_cols-1) << endl;
    cout << "current position: " << endl << pose << endl;
    cout << endl;

    // send the trajectory to the robot's wheels
    rose.set_wheels(v_l, v_r, v_l, v_r);

    // only if we want to do a pose do we activate the pose solver
    if (!do_pose) {
      rose.stop_arm();
      continue;
    }

    // get the list of all possible poses
    bool feasible_found = false;
    double baseangle = atan2(poseplan(1), poseplan(0)) * 180 / M_PI - 90.0;
    vec enc(6);
    for (int i = 0; i <= 90; i += 5) {
      mat R = genRotateMat(0, 0, baseangle);
      // if a negative pose proves to find a solution, stop
      vec negpose({ 0, cos(-i * M_PI / 180), sin(-i * M_PI / 180) });
      negpose = R * negpose;
      if (rose.get_arm_position_placement(poseplan, negpose, twist, grab, enc)) {
        feasible_found = true;
        break;
      }
      // if a positive pose proves to find a solution, stop
      vec pospose({ 0, cos(i * M_PI / 180), sin(i * M_PI / 180) });
      pospose = R * pospose;
      if (rose.get_arm_position_placement(poseplan, negpose, twist, grab, enc)) {
        feasible_found = true;
        break;
      }
    }

    // send poses to the robot arm
    if (feasible_found) {
      rose.set_arm(enc(0), enc(1), enc(2), enc(3), enc(4), enc(5));
    } else {
      rose.stop_arm();
    }
  }

  rose.send(zeros<vec>(10));
}

void motion_plan(void) {
  vec goal = vec({ 40, 300 }); // this will have to change later somehow
  // grab the map
  mat localmap = globalmap.map;

  AStar astar(localmap, goal);
  while (!stopsig) {
    // try and see if we are allowed to go autonomous
    bool en = false;
    autonomous_lock.lock();
    en = auto_enable && manual_confirmed;
    auto_confirmed = true;
    autonomous_lock.unlock();

    // if we are indeed enabled, then we can compute the path to take
    if (!en) {
      continue;
    } else {
      printf("Autonomous enabled!\n");
    }

    // grab the current position
    pose_lock.lock();
    vec pose = robot_pose;
    pose_lock.unlock();

    // compute the new path
    vector<MotionAction> actionpath;
    vec curr = pose(span(0,1));
    astar.compute(curr, actionpath);
    if (astar.impossible()) {
      // store an empty path
      path_lock.lock();
      pathplan = mat(2, 0);
      dopose = false; // shut off the pose just in case
      path_lock.unlock();
      continue;
    }

    // prune bad motion vectors
    vector<vec> prunedpath;
    vec origin;
    for (int i = 0; i < actionpath.size(); i++) {
      if (i == 0) {
        origin = vec({ actionpath[i].x, actionpath[i].y });
        prunedpath.push_back(origin);
      } else if (i == actionpath.size() - 1) {
        prunedpath.push_back(vec({ actionpath[i].x, actionpath[i].y }));
      } else {
        vec target({ actionpath[i].x, actionpath[i].y });
        vec diff = target - origin;
        if (sqrt(dot(diff, diff)) >= 20) {
          prunedpath.push_back(target);
          origin = target;
        }
      }
    }

    // store the pruned path
    mat coalescedpath(2, prunedpath.size());
    for (int i = 0; i < prunedpath.size(); i++) {
      coalescedpath.col(i) = vec({ prunedpath[i](0), prunedpath[i](1) });
    }
    path_lock.lock();
    pathplan = coalescedpath;
    dopose = false;
    path_lock.unlock();
  }
}

void display_interface(void) {
  cube frame(500, 500, 3, fill::zeros);

  while (!stopsig) {
    frame.zeros();

    // get the position of the robot
    pose_lock.lock();
    vec pose = robot_pose;
    pose_lock.unlock();

    // create a window around the pose
    int mux = (int)round(pose(0));
    int muy = (int)round(pose(1));
    double mut = pose(2);
    int sw2 = screen->w / 2;
    int sh2 = screen->h / 2;

    // draw the map
    globalmap.blit(frame, mux, muy);

    // draw the landmarks
    for (sim_landmark &lm : landmarks) {
      lm.blit(frame, mux, muy);
    }

    // draw the particle filter
    pf.blit(frame, mux, muy);

    // draw the robot's position and pose
    for (int _i = -5; _i <= 5; _i++) {
      for (int _j = -5; _j <= 5; _j++) {
        int xx = sw2 + _j;
        int yy = sh2 + _i;
        if (xx < 0 || xx >= (int)frame.n_cols ||
            yy < 0 || yy >= (int)frame.n_rows) {
          continue;
        }
        frame(yy,xx,0) = 1;
        frame(yy,xx,1) = 0;
        frame(yy,xx,2) = 0;
      }
    }
    int _xx = (int)round(sw2 + (10 * cos(mut)));
    int _yy = (int)round(sh2 + (10 * sin(mut)));
    vec color({ 0, 1, 1 });
    draw_line(frame, color, vec({(double)sh2,(double)sw2-1}), vec({(double)_yy,(double)_xx-1}));
    draw_line(frame, color, vec({(double)sh2,(double)sw2+1}), vec({(double)_yy,(double)_xx+1}));
    draw_line(frame, color, vec({(double)sh2-1,(double)sw2}), vec({(double)_yy-1,(double)_xx}));
    draw_line(frame, color, vec({(double)sh2+1,(double)sw2}), vec({(double)_yy+1,(double)_xx}));
    draw_line(frame, color, vec({(double)sh2,(double)sw2}), vec({(double)_yy,(double)_xx}));

    // draw A*
    path_lock.lock();
    mat path_plan = pathplan;
    path_lock.unlock();
    if (auto_enable) {
      vec purple({ 1, 0, 1 });
      for (int j = 1; j < (int)path_plan.n_cols; j++) {
        vec action1 = path_plan.col(j-1) + vec({ (double)(sw2 - mux), (double)(sh2 - muy) });
        vec action2 = path_plan.col(j) + vec({ (double)(sw2 - mux), (double)(sh2 - muy) });
        draw_line(frame, purple, vec({ action1(1), action1(0) }), vec({ action2(1), action2(0) }));
      }
    }

    // push onto the screen
    screenblit(screen, frame);
    SDL_Flip(screen);
    SDL_Delay(25);
  }
}
