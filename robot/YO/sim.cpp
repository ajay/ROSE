#include "sim_map.h"
#include "sim_robot.h"
//#include "sim_window.h"
#include "pfilter.h"
#include "sim_landmark.h"
#include <cstdio>
#include <armadillo>
#include <SDL/SDL.h>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <thread>
#include <signal.h>
#include "draw.h"
#include "sdldef.h"

#include "chili_landmarks.h"
#include "Rose.h"

using namespace std;
using namespace arma;

SDL_Event event;

static chili_landmarks chili;

void update_chili()
{
	chili.update();
}

mat sense() {
  mat sv(3, 10);
  for (int j = 0; j < 10; j++) {
    sv.col(j) = vec({
        chili.tags[j][1],
        chili.tags[j][2],
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


SDL_Surface *initSDL(int w, int h)
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
  SDL_Surface *screen = initSDL(map.n_cols, map.n_rows);
  if (!screen) {
    return 1;
  }
  icube frame(map.n_rows, map.n_cols, 3, fill::zeros), newframe;
  bool turn_left = false;
  bool turn_right = false;
  bool forward = false;
  bool backward = false;
  bool strafe_left = false;
  bool strafe_right = false;
  bool quit = false;

  while (!quit) {
    // see if something is about to quit

    SDL_PumpEvents();
		Uint8 *keystates = SDL_GetKeyState(NULL);

		// Front left, front right, back left, back right
		if (keystates[SDLK_a])			  { drive(-1,  1, -1,  1); }  // left turn
		else if(keystates[SDLK_d])    { drive( 1, -1,  1, -1); }  // right turn
		else if(keystates[SDLK_w]) 	  { drive( 1,  1,  1,  1); }  // up
		else if(keystates[SDLK_s]) 	  { drive(-1, -1, -1, -1); }  // down
		else if(keystates[SDLK_q]) 	  { drive(-1,  1,  1, -1); }  // left strafe
		else if(keystates[SDLK_e]) 	  { drive( 1, -1, -1,  1); }  // right strafe
		else if(keystates[SDLK_1]) 		{ drive( 1,  0,  0,  0); }
		else if(keystates[SDLK_2]) 		{ drive( 0,  1,  0,  0); }
		else if(keystates[SDLK_3]) 		{ drive( 0,  0,  1,  0); }
		else if(keystates[SDLK_4]) 		{ drive( 0,  0,  0,  1); }

		else
		{
			drive(0, 0, 0, 0);
		}

		if(keystates[SDLK_q])
		{
      printf("QUITTING\n");
			quit = true;
			SDL_Quit();
      continue;
		}
    forward = keystates[SDLK_w];
		backward = keystates[SDLK_s];
		turn_left = keystates[SDLK_a];
    turn_right = keystates[SDLK_d];
    strafe_left = keystates[SDLK_q];
    strafe_right = keystates[SDLK_e];

    // update the robot
    //mat sensor_values = lidar.sense();
    //mat tag_landmarks;
    //sim_tag_extract(tag_landmarks, sensor_values, landmarks, robot, map);
    

    mat tag_landmarks = sense();
    pf.observe(tag_landmarks);
    //robot.move((forward - backward), (strafe_right - strafe_left), (turn_left - turn_right) * .1);
    rose.send(motion);
    pf.move((forward - backward), (strafe_right - strafe_left), (turn_left - turn_right) * .1);

    // predict the position
    vec mu;
    vec sigma;
    pf.predict(mu, sigma);
    cout << "position: " << mu(0) << ", " << mu(1) << ", angle: " << mu(2) * 180 / M_PI << ", error: " << sigma << endl;

    // put stuff on the screen
    map.blit(frame);
    for (sim_landmark &lm : landmarks) {
      lm.blit(frame);
    }
    ivec red({255,0,0});
    for (int i = 0; i < landmarks.size(); i++) 
    {
      if (tag_landmarks(2,i) > 0.5) {
        vec pt({(double)landmarks[i].x,(double)landmarks[i].y});
        draw_circle(frame, red, pt, 4);
        draw_circle(frame, red, pt, 5);
        draw_circle(frame, red, pt, 6);
      }
    }
    //lidar.blit(frame);
    pf.blit(frame);
    //robot.blit(frame);
    screenblit(screen, frame);
    SDL_Delay(25);

    // draw the screen
    SDL_Flip(screen);
  }
  rose.disconnect();

  // clean up
  SDL_Quit();
}
