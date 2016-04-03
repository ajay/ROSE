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
static void autonomous_decide_trajectory(void);
static mutex autonomous_lock;
static vec robot_pos; // make sure this is a 2vec
static vec robot_auton_drive; // this is an output 2vec
// K_i & K_d
static  double theta_k_p = 1;
static  double theta_k_i = 0;
static  double theta_k_d = 1;

static double calculate_target_angle(vec &curr, vec &target) {

  vec diff = target - curr;
  double delta_theta = atan2(diff(1), diff(0)) * 180 / M_PI;

  if (delta_theta < 0){
    delta_theta += 360;
  }
  return delta_theta;
}

static double calculate_distance(vec &a, vec &b) {
  vec diff = b - a;
  return sqrt(dot(diff, diff));
}

void detect_thread()
{
  chili.update();
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
  map.load("map_engineering_b_wing.jpg");
  frame = icube(map.n_rows, map.n_cols, 3, fill::zeros);

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
  robot_pos = vec({ 20, 20 });
  signal(SIGINT, stop);

  /////////////////////////
  //	START OF THE BOT
  /////////////////////////

  // create the particle filter
  pf = pfilter(500, &map, landmarks, robot_pos(0), robot_pos(1), M_PI_2);
  pf.set_size(12);
  pf.set_noise(1.5, 0.002);

  // start up the planner
  //thread planner(autonomous_decide_trajectory);
    
  // start up the window
  screen = sim_window::init(500, 500);
  if (!screen) {
    return 1;
  }

  // load the chilitag detector
  std::thread detect(detect_thread);

  bool forward = false;
  bool backward = false;
  bool turn_left = false;
  bool turn_right = false;
  bool strafe_left = false;
  bool strafe_right = false;
  bool quit = false;
  while (!quit) {
    // see if something is about to quit

    SDL_PumpEvents();
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    if (keystates[SDL_SCANCODE_Q])				{ drive(-1,  1, -1,  1); }	// left turn
    else if (keystates[SDL_SCANCODE_E])			{ drive( 1, -1,  1, -1); }	// right turn
    else if (keystates[SDL_SCANCODE_W]) 		{ drive( 1,  1,  1,  1); }	// up
    else if (keystates[SDL_SCANCODE_S]) 		{ drive(-1, -1, -1, -1); }	// down
    else if (keystates[SDL_SCANCODE_A]) 		{ drive(-1,  1,  1, -1); }	// left strafe
    else if (keystates[SDL_SCANCODE_D]) 		{ drive( 1, -1, -1,  1); }	// right strafe
    else if (keystates[SDL_SCANCODE_1]) 		{ drive( 1,  0,  0,  0); }
    else if (keystates[SDL_SCANCODE_2]) 		{ drive( 0,  1,  0,  0); }
    else if (keystates[SDL_SCANCODE_3]) 		{ drive( 0,  0,  1,  0); }
    else if (keystates[SDL_SCANCODE_4]) 		{ drive( 0,  0,  0,  1); }
    else if (keystates[SDL_SCANCODE_N]) 		{ auto_enable = true; }	//
    else if (keystates[SDL_SCANCODE_M]) 		{ auto_enable = false; }	//
    else if (!auto_enable) 				  { drive( 0,  0,  0,  0); }	//

    if (keystates[SDL_SCANCODE_X]) {
      drive(0, 0, 0, 0);
      quit = true;
      SDL_Quit();
      break;
    }
    forward = keystates[SDL_SCANCODE_W];
    backward = keystates[SDL_SCANCODE_S];
    turn_left = keystates[SDL_SCANCODE_Q];
    turn_right = keystates[SDL_SCANCODE_E];
    strafe_left = keystates[SDL_SCANCODE_A];
    strafe_right = keystates[SDL_SCANCODE_D];
    rose.send(motion);
    printf("encoder values: \n");
    for (int i = 0; i < 4; i++) {
      printf("%d\n", rose.encoder[i]);
    }
    pf.move(forward - backward, turn_left - turn_right, rose.encoder);
    tag_landmarks = sense();
    cout << "sensed: \n" << tag_landmarks << "\n";
    pf.observe(tag_landmarks);
    // predict the position
    //printf("[sim.cpp] predicting\n");
    pf.predict(mu, sigma);
    cout << "[sim.cpp] position: " << mu(0) << ", " << mu(1) << ", angle: " << mu(2) * 180 / M_PI << "\n[sim.cpp] error: \n" << sigma << endl;
    // recompute the planner in order to get the most optimal path
    if (auto_enable) {
      autonomous_lock.lock();
      robot_pos = mu(span(0,1));
      vec speeds = robot_auton_drive;
      autonomous_lock.unlock();
      drive(speeds(0), speeds(1), speeds(0), speeds(1));
    }
    draw_things();

  }
  rose.disconnect();

  // clean up
  sim_window::destroy();
}

static void autonomous_decide_trajectory(void) {
  // start up the A* planner to create an initial path for the robot to start moving
  vec goal = vec({76,94});
  printf("goal of A*:\n");
  cout << goal << endl;
  printf("map size: %u %u\n", map.map.n_rows, map.map.n_cols);
  AStar astar(map.map, goal);
  vector<MotionAction> path;
  for (;;) {
    autonomous_lock.lock();
    vec currPos = robot_pos;
    autonomous_lock.unlock();
    printf("A* currpos:\n");
    cout << currPos << endl;

    // compute the new path
    astar.compute(currPos, path);
    if (astar.impossible()) {
      // do nothing for now
      drive(0, 0, 0, 0);
    }

    int len = path.size();
    for (int i = 0; i < len-1; i++){
      // Cycle through pairs...if the second is too near the first, then it is removed from the path
      vec origin = vec({path[i].x, path[i].y});
      vec target = vec({path[i+1].x, path[i+1].y});
      if (calculate_distance(origin, target) <= 30){
        path.erase(path.begin() + i);
        i--;
      }
    }

    /****************
    // for assigning left side and right side motor duty cycles.
     ***************/

    // update delta_theta for assigning motor velocities
    vec curr = vec({path[0].x, path[0].y});
    delta_theta = calculate_target_angle(robot_pos, curr); 
    // popped off of stack
    path.erase(path.begin());  
    // Assign v_l & v_r
    double v_l, v_r;
    v_l = (100 - theta_k_p * delta_theta);
    // - theta_k_i * r.integral_error
    // - theta_k_d * r.delta_theta_diff);

    v_r = (100 + theta_k_p * delta_theta);
    // + r.theta_k_i * r.integral_error
    // + r.theta_k_d * r.delta_theta_diff);

    autonomous_lock.lock();
    robot_auton_drive = vec({ v_l, v_r });
    autonomous_lock.unlock();
  }

}

static void draw_things()
{
  // put stuff on the screen
  int mux = (int)round(mu(0));
  int muy = (int)round(mu(1));
  double mut;
  const ivec color({ 0, 0, 255 });
  if (map.map.n_elem > 0) {
    printf("[sim.cpp] [draw] blit map\n");
    map.blit(frame, mux, muy, screen->w, screen->h);
    printf("[sim.cpp] [draw] blit landmark\n");
    //frame.zeros();
    for (sim_landmark &lm : landmarks) {
      lm.blit(frame);
    }
    /*printf("[sim.cpp] [draw] blit landmark detect\n");
    ivec cyan({255,0, 0});
    mat lms = tag_landmarks;

     for (int i = 0; i < landmarks.size(); i++)
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
    mux = (int)round(mu(0));
    muy = (int)round(mu(1));
    mut = mu(2);
    int _xx = (int)round(mux + 10 * cos(mut));
    int _yy = (int)round(muy + 10 * sin(mut));
    draw_line(frame, color, vec({(double)muy,(double)mux-1}), vec({(double)_yy,(double)_xx-1}));
    draw_line(frame, color, vec({(double)muy,(double)mux}), vec({(double)_yy,(double)_xx}));
    draw_line(frame, color, vec({(double)muy,(double)mux+1}), vec({(double)_yy,(double)_xx+1}));
    draw_line(frame, color, vec({(double)muy-1,(double)mux}), vec({(double)_yy-1,(double)_xx}));
    draw_line(frame, color, vec({(double)muy+1,(double)mux}), vec({(double)_yy+1,(double)_xx}));
    // get a subset of the frame
    newframe = partial_frame(frame, mux, muy, screen->w, screen->h);
    screenblit(screen, newframe);

    // draw the screen
    sim_window::update();
    SDL_Delay(25);
  }
}
