#include "sim_map.h"
#include "sim_robot.h"
//#include "sim_lidar.h"
#include "sim_window.h"
#include "pfilter.h"
#include "sim_landmark.h"
#include <cstdio>
#include <armadillo>
#include <SDL2/SDL.h>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <thread>

#include "chilitag_landmark.h"
#include "Rose.h"

using namespace std;
using namespace arma;


static chili_landmarks chili;

void update_chili()
{
	chili.update();
}

mat sense() {
  mat sv(3, 10);
  for (int j = 0; j < 10; j++) {
    sv.col(j) = vec({
        chili.tags[j][1]*2,
        chili.tags[j][2]*2,
        chili.tags[j][0]});
  }
  return sv;
}

static int stopsig;
static arma::vec motion = zeros<vec>(4);
static Rose rose;
void drive(double frontLeft, double frontRight, double backLeft, double backRight)
{
	motion[0] = frontLeft;
	motion[1] = frontRight;
	motion[2] = backLeft;
	motion[3] = backRight;
}


void stop(int signo)
{
	printf("Exiting yo >>>>\n");
	rose.startStop = true;
	stopsig = 1;
	exit(1);
}


// This is a simpler landmark extractor with the goal of extracting the location of the landmarks easily
/*void sim_tag_extract(mat &tag_landmarks,
    mat &sensor_vals,
    vector<sim_landmark> &landmarks,
    sim_robot &robot,
    sim_map &map) {
  tag_landmarks = mat(2, landmarks.size());
  for (int lid = 0; lid < landmarks.size(); lid++) {
    tag_landmarks.col(lid) = landmarks[lid].sense(robot);
  }
}*/

icube doubleImageSize(icube &image) {
  icube newframe(image.n_rows * 2, image.n_cols * 2, 3);
  for (uword i = 0; i < image.n_rows * 2; i++) {
    for (uword j = 0; j < image.n_cols * 2; j++) {
      newframe(i, j, 0) = image(i / 2, j / 2, 0);
      newframe(i, j, 1) = image(i / 2, j / 2, 1);
      newframe(i, j, 2) = image(i / 2, j / 2, 2);
    }
  }
  return newframe;
}

int main() {
  srand(getpid());

  // load the map (custom)
  sim_map map;
  map.load("map.jpg");

  // create the landmarks (custom)
  vector<sim_landmark> landmarks;
  landmarks.push_back(sim_landmark(54, 32));
  landmarks.push_back(sim_landmark(256, 150));
  landmarks.push_back(sim_landmark(274, 302));
  landmarks.push_back(sim_landmark(450, 374));
  landmarks.push_back(sim_landmark(202, 206));
  landmarks.push_back(sim_landmark(576, 24));
  landmarks.push_back(sim_landmark(0, 230));
  landmarks.push_back(sim_landmark(522, 168));
  landmarks.push_back(sim_landmark(158, 134));
  landmarks.push_back(sim_landmark(116, 374));

  // load the robot
  //sim_robot robot(&map);
  //robot.set_size(8);
  //robot.set_pose(70, 48, 0);
  //robot.set_noise(0.3, 0.02);
  rose.startStop = false;
	signal(SIGINT, stop);

  // load the lidar
  //sim_lidar lidar(&robot);
  //lidar.set_noise(1.0, 0.1);
  std::thread detect(update_chili);

/////////////////////////
//  START OF THE BOT
/////////////////////////

  // create the particle filter
  pfilter pf(1000, &map, landmarks);
  pf.set_size(12);
  pf.set_noise(6.0, 1.5);
  
  // start up the window
  SDL_Surface *screen = sim_window::init(map.n_cols * 2, map.n_rows * 2);
  icube frame(map.n_rows, map.n_cols, 3, fill::zeros), newframe;
  bool left = false;
  bool right = false;
  bool forward = false;
  bool backward = false;
  bool quit = false;

  while (!quit) {
    // see if something is about to quit
    SDL_PumpEvents();
		Uint8 *keystates = SDL_GetKeyState(NULL);

		// Front left, front right, back left, back right
		if (keystates[SDLK_a])			{ drive(-1,  1, -1,  1); }
		else if(keystates[SDLK_s]) 		{ drive( 1, -1,  1, -1); }
		else if(keystates[SDLK_UP]) 	{ drive( 1,  1,  1,  1); }
		else if(keystates[SDLK_DOWN]) 	{ drive(-1, -1, -1, -1); }
		else if(keystates[SDLK_LEFT]) 	{ drive(-1,  1,  1, -1); }
		else if(keystates[SDLK_RIGHT]) 	{ drive( 1, -1, -1,  1); }
		else if(keystates[SDLK_1]) 		{ drive( 1,  0,  0,  0); }
		else if(keystates[SDLK_2]) 		{ drive( 0,  1,  0,  0); }
		else if(keystates[SDLK_3]) 		{ drive( 0,  0,  1,  0); }
		else if(keystates[SDLK_4]) 		{ drive( 0,  0,  0,  1); }

		else
		{
			drive(0, 0, 0, 0);
		}

		if(keystates[SDLK_q] || rose.startStop)
		{
			quit = true;
			SDL_Quit();
		}
    forward = keystates[SDLK_UP];
		backward = keystates[SDLK_DOWN];
		left = keystates[SDLK_LEFT];
    right = keystates[SDLK_RIGHT];
    
    

    // update the robot
    //mat sensor_values = lidar.sense();
    //mat tag_landmarks;
    //sim_tag_extract(tag_landmarks, sensor_values, landmarks, robot, map);
    mat tag_landmarks = sense();
    pf.observe(tag_landmarks);
    //robot.move(forward * 2, (left - right) * .1);
    rose.send(motion);
    pf.move((forward - backward) * 2, (left - right) * .1);

    // predict the position
    vec mu;
    double sigma;
    pf.predict(mu, sigma);
    cout << "position: " << mu(0) << ", " << mu(1) << ", angle: " << mu(2) * 180 / M_PI << ", error: " << sigma << endl;

    // put stuff on the screen
    map.blit(frame);
    for (sim_landmark &lm : landmarks) {
      lm.blit(frame);
    }
    //lidar.blit(frame);
    pf.blit(frame);
    //robot.blit(frame);
    sim_window::blit(screen, frame);
    SDL_Delay(25);

    // draw the screen
    sim_window::update();
  }
  rose.disconnect();

  // clean up
  sim_window::destroy();
}
