#include "sim_map.h"
#include "sim_robot.h"
#include "sim_window.h"
#include "pfilter.h"
#include "sim_landmark.h"
#include <cstdio>
#include <armadillo>
//#include <SDL/SDL.h>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <signal.h>
#include "draw.h"
#include "sdldef.h"
#include <mutex>
#include "astar.h"


#include "chili_landmarks.h"
#include "Rose.h"

#define NUMLANDMARKS 20

using namespace std;
using namespace arma;

static SDL_Event *event;
static chili_landmarks chili;
static int stopsig;
static arma::vec motion = zeros<vec>(4);
static Rose rose;
static bool auto_enable;
static double delta_theta;
static double calculate_distance(vec &a, vec &b);
static double calculate_target_angle(vec &curr, vec &target);
static void draw_things();

static double calculate_target_angle(vec &curr, vec &target) {

  // MODIFY ATAN2 FOR CPPa
  vec diff = target - curr;
  double angle = atan2(diff(1), diff(0)) * 180 / M_PI;

  if (angle < 0){ 
    angle = angle + 360;
  }

  return angle;
}

static double calculate_distance(vec &a, vec &b) {
  vec diff = b - a;
  return sqrt(dot(diff, diff));
}

mat sense() {
  mat sv(3, NUMLANDMARKS);
  for (int j = 0; j < NUMLANDMARKS; j++) {
    sv.col(j) = vec({
        chili.tags[j][1],
        chili.tags[j][2],
        chili.tags[j][0]});
  }
  return sv;
}

void drive(double frontLeft, double frontRight, double backLeft, double backRight)
{
  motion[0] = frontLeft;
  motion[1] = frontRight;
  motion[2] = backLeft;
  motion[3] = backRight;
}

void stop(int signo)
{
  //printf("Exiting yo >>>>\n");
  rose.startStop = true;
  stopsig = 1;
  exit(1);
}

void screenblit(SDL_Surface *s, icube &frame) {
  for (int i = 0; i < (int)frame.n_rows; i++) {
    for (int j = 0; j < (int)frame.n_cols; j++) {
      uint32_t color =
        (((uint8_t)frame(i,j,0)) << 16) |
        (((uint8_t)frame(i,j,1)) << 8) |
        (((uint8_t)frame(i,j,2)) << 0);
      ((uint32_t *)s->pixels)[XY2P(j, i, s->w, s->h)] = color;
    }
  }
}

icube partial_frame(icube frame, int x, int y, int width, int height) {
  icube partial_frame(height, width, 3, fill::zeros);
  for (int i = 0; i < (int)partial_frame.n_rows; i++) {
    for (int j = 0; j < (int)partial_frame.n_cols; j++) {
      int x_ = x - width/2 + j;
      int y_ = y - height/2 + i;
      if (x_ < 0 || x_ >= (int)frame.n_cols ||
          y_ < 0 || y_ >= (int)frame.n_rows) {
        continue;
      }
      partial_frame(i, j, 0) = frame(y_, x_, 0);
      partial_frame(i, j, 1) = frame(y_, x_, 1);
      partial_frame(i, j, 2) = frame(y_, x_, 2);
    }
  }
  return partial_frame;
}


SDL_Surface *screen;
sim_map map;
icube frame, newframe;
vector<sim_landmark> landmarks;
mat tag_landmarks;
pfilter pf;

vec mu(2);
mat sigma;

int main() {
  srand(getpid());

  // load the map (custom)
  sim_map map;
  map.load("map_engineering_b_wing.jpg");
  frame = icube(map.n_rows, map.n_cols, 3, fill::zeros);

  // start up the A* planner to create an initial path for the robot to start moving
  vec goal = vec({76,94});
  AStar astar(map.map, goal);
  vector<MotionAction> path;

  astar.compute(vec({ 20, 20 }), path);
  if (astar.impossible()) {
    printf("AStar cannot find a path!\n");
    drive(0, 0, 0, 0);
  }

  // create the landmarks (custom)
  vector<sim_landmark> landmarks;
  landmarks.push_back(sim_landmark(90, 202));						// 0
  landmarks.push_back(sim_landmark(601, 617));					// 1
  landmarks.push_back(sim_landmark(0, 416));						// 2
  landmarks.push_back(sim_landmark(98, 627));						// 3
  landmarks.push_back(sim_landmark(384, 627));					// 4
  landmarks.push_back(sim_landmark(1225, 627));					// 5
  landmarks.push_back(sim_landmark(3400, 513));					// 6
  landmarks.push_back(sim_landmark(3442, 583));					// 7
  landmarks.push_back(sim_landmark(1961, 539));					// 8
  landmarks.push_back(sim_landmark(3442-460, 539+80));	// 9
  landmarks.push_back(sim_landmark(0, 517));						// 10
  landmarks.push_back(sim_landmark(78, 202));						// 11
  landmarks.push_back(sim_landmark(111, 627));					// 12
  landmarks.push_back(sim_landmark(384, 707));					// 13
  landmarks.push_back(sim_landmark(601, 537));					// 14
  landmarks.push_back(sim_landmark(1165, 540));					// 15
  landmarks.push_back(sim_landmark(1961, 539+80));			// 16
  landmarks.push_back(sim_landmark(3442, 583-37));			// 17
  landmarks.push_back(sim_landmark(3400+14, 513));			// 18
  landmarks.push_back(sim_landmark(3442-460, 539));			// 19

  // load the robot
  rose.startStop = false;
  signal(SIGINT, stop);

  /////////////////////////
  //	START OF THE BOT
  /////////////////////////

  // create the particle filter
  pf = pfilter(500, &map, landmarks, 20, 20, M_PI_2);
  pf.set_size(12);
  pf.set_noise(1.5, 0.002);

  // start up the window
  screen = sim_window::init(500, 500);
  if (!screen) {
    return 1;
  }

  // load the chilitag detector
  std::thread detect(chili.update);

  //icube frame(map.n_rows, map.n_cols, 3, fill::zeros), newframe;
  bool forward = false;
  bool backward = false;
  bool turn_left = false;
  bool turn_right = false;
  bool strafe_left = false;
  bool strafe_right = false;
  bool quit = false;

  while (!quit) {
    // see if something is about to quit

   // Make sure all points are evenly spaced out
    if (auto_enable){
      int len = path.size();
      for (int i = 0; i < len-1; i++){
        // Cycle through pairs...if the second is too near the first, then it is removed from the path
        if (calculate_distance(vec({path[i].x, path[i].y}), vec({path[i+1].x, path[i+1].y})) <= 30){
          path.erase(path.begin() + i);
          i--;
        }
      }

      /****************
      // for assigning left side and right side motor duty cycles.
       ***************/

      // K_i & K_d
      double theta_k_i = 0;
      double theta_k_d = 2.5;

        // Assign v_l & v_r
      double v_l, v_r;
      v_l = (100 - theta_k_p * delta_theta
          - theta_k_i * r.integral_error
          - theta_k_d * r.delta_theta_diff);

      v_r = (100 + r.theta_k_p * r.delta_theta
          + r.theta_k_i * r.integral_error
          + r.theta_k_d * r.delta_theta_diff);

      drive(v_l, v_r, v_l, v_r);
    }

    SDL_PumpEvents();
    const Uint8 *keystates = SDL_GetKeyState(NULL);
    if (keystates[SDLK_q])					{ drive(-1,  1, -1,  1); }	// left turn
    else if (keystates[SDLK_e])			{ drive( 1, -1,  1, -1); }	// right turn
    else if (keystates[SDLK_w]) 		{ drive( 1,  1,  1,  1); }	// up
    else if (keystates[SDLK_s]) 		{ drive(-1, -1, -1, -1); }	// down
    else if (keystates[SDLK_a]) 		{ drive(-1,  1,  1, -1); }	// left strafe
    else if (keystates[SDLK_d]) 		{ drive( 1, -1, -1,  1); }	// right strafe
    else if (keystates[SDLK_1]) 		{ drive( 1,  0,  0,  0); }
    else if (keystates[SDLK_2]) 		{ drive( 0,  1,  0,  0); }
    else if (keystates[SDLK_3]) 		{ drive( 0,  0,  1,  0); }
    else if (keystates[SDLK_4]) 		{ drive( 0,  0,  0,  1); }
    else if (keystates[SDLK_n]) 		{ auto_enable = true; }	// 
    else if (keystates[SDLK_m]) 		{ auto_enable = false; }	// 
    else if (!auto_enable) 				  { drive( 0,  0,  0,  0); }	// 

    if (keystates[SDLK_x]) {
      drive(0, 0, 0, 0);
      quit = true;
      SDL_Quit();
      break;
    }
    forward = keystates[SDLK_w];
    backward = keystates[SDLK_s];
    turn_left = keystates[SDLK_q];
    turn_right = keystates[SDLK_e];
    strafe_left = keystates[SDLK_a];
    strafe_right = keystates[SDLK_d];

    rose.send(motion);
    pf.move(forward - backward, turn_left - turn_right, rose.encoder);

    tag_landmarks = sense();
    cout << "sensed: \n" << tag_landmarks << "\n";
    pf.observe(tag_landmarks);

    // predict the position
    //printf("[sim.cpp] predicting\n");
    pf.predict(mu, sigma);
    cout << "[sim.cpp] position: " << mu(0) << ", " << mu(1) << ", angle: " << mu(2) * 180 / M_PI << "\n[sim.cpp] error: \n" << sigma << endl;

    // recompute the planner in order to get the most optimal path
    astar.compute(mu.subvec(0,1), path);
    if (astar.impossible()) {
      // do nothing for now
      printf("AStar cannot find a path!\n");
      drive(0, 0, 0, 0);
    }

    draw_things();

  }
  rose.disconnect();

  // clean up
  sim_window::destroy();
}

static void draw_things()
{
  // put stuff on the screen
  int mux = (int)round(mu(0));
  int muy = (int)round(mu(1));
  icube newframe;
  if (map.map.n_elem > 0) {
    printf("[sim.cpp] [draw] blit map\n");
    map.blit(frame, mux, muy, screen->w, screen->h);
    printf("[sim.cpp] [draw] blit landmark\n");
    //frame.zeros();
    for (sim_landmark &lm : landmarks) {
      lm.blit(frame);
    }
    printf("[sim.cpp] [draw] blit landmark detect\n");
    ivec cyan({255,0, 0});
    mat lms = tag_landmarks;

    /* for (int i = 0; i < landmarks.size(); i++)
       {
       if (lms.n_cols != landmarks.size()) {
       printf("[sim.cpp] [draw] not enough landmarks!\n");
       break;
       }

       printf("[sim.cpp] [landmark loop] examime: %d/%d\n", i, landmarks.size()); 

       if (lms(2,i) > 0.5) {
       vec pt({(double)landmarks[i].x,(double)landmarks[i].y});
       draw_circle(frame, cyan, pt, lms(1,i)-1);
       draw_circle(frame, cyan, pt, lms(1,i));
       draw_circle(frame, cyan, pt, lms(1,i)+1);
       }
       } */
    printf("[sim.cpp] [draw] blit particle filter\n");
    pf.blit(frame);
    printf("[sim.cpp] [draw] blit robot\n");
    for (int _i = -5; _i <= 5; _i++) {
      for (int _j = -5; _j <= 5; _j++) {
        int xx = (int)round(mu(0)) + _j;
        int yy = (int)round(mu(1)) + _i;
        if (xx < 0 || xx >= (int)frame.n_cols ||
            yy < 0 || yy >= (int)frame.n_rows) {
          continue;
        }
        frame(yy,xx,0) = 255;
        frame(yy,xx,1) = 0;
        frame(yy,xx,2) = 0;
      }
    }
    //robot.blit(frame);
    //printf("[sim.cpp] [draw] blit partial frame\n");
    newframe = partial_frame(frame, mux, muy, screen->w, screen->h);
    printf("[sim.cpp] [draw] screenblit\n");
    //memset(screen->pixels, 0, sizeof(uint32_t) * screen->w * screen->h);
    screenblit(screen, newframe);
    //screenblit(screen, frame);

    // draw the screen
    printf("[sim.cpp] [draw] flipping screen\n");
    //SDL_Flip(screen);
    sim_window::update();
    SDL_Delay(25);
  }
}
