#include "sim_map.h"
#include "sim_robot.h"
//#include "sim_window.h"
#include "pfilter.h"
#include "sim_landmark.h"
#include <cstdio>
#include <armadillo>
#include "SDL/SDL.h"
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

icube bilinint(icube frame) {
  icube f((frame.n_rows - 1) / 2 + 1, (frame.n_cols - 1) / 2 + 1, 3, fill::zeros);
  for (int i = 0; i < (int)frame.n_rows; i++) {
    for (int j = 0; j < (int)frame.n_cols; j++) {
      f(i / 2, j / 2, 0) += frame(i, j, 0) / 4;
      f(i / 2, j / 2, 1) += frame(i, j, 1) / 4;
      f(i / 2, j / 2, 2) += frame(i, j, 2) / 4;
    }
  }
  return f;
}

void update_chili()
{
	chili.update();
}

void draw_things();

mat sense() {
  mat sv(3, 10);
  for (int j = 0; j < 10; j++) {
    sv.col(j) = vec({
        -chili.tags[j][1],
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
  printf("screen dims: %d %d\n", s->w, s->h);
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

SDL_Surface *screen;
sim_map map;
icube frame, newframe;
vector<sim_landmark> landmarks;
mat tag_landmarks;
pfilter pf;

int main() {
  srand(getpid());

  // load the map (custom)

  //map.load("map_engineering_b_wing.jpg");
  map.load("map.jpg");
  frame = icube(map.n_rows, map.n_cols, 3, fill::zeros);

  // 2/24/16 Engineering Building
  // 0 (88, 202)
  // 1 (601, 617)
  // 2 (0, 171)
  // 3 (98, 627)
  // 4 (384, 627)
  // 5 (1225, 627)
  // 6 (3400, 513)
  // 7 (3442, 583)
  // 8 (1961, 539)
  // 9 (762, 539)

  // create the landmarks (custom)

  /*landmarks.push_back(sim_landmark(88, 202));
  landmarks.push_back(sim_landmark(601, 617));
  landmarks.push_back(sim_landmark(0, 171));
  landmarks.push_back(sim_landmark(98, 627));
  landmarks.push_back(sim_landmark(384, 627));
  landmarks.push_back(sim_landmark(1225, 627));
  landmarks.push_back(sim_landmark(3400, 513));
  landmarks.push_back(sim_landmark(3442, 583));
  landmarks.push_back(sim_landmark(1961, 539));
  landmarks.push_back(sim_landmark(762, 539));*/
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0, 171));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));
  landmarks.push_back(sim_landmark(0,0));

  // load the robot
  //sim_robot robot(&map);
  //robot.set_size(8);
  //robot.set_pose(70, 48, 0);
  //robot.set_noise(0.3, 0.02);
  rose.startStop = false;
	signal(SIGINT, stop);

/////////////////////////
//  START OF THE BOT
/////////////////////////

  // create the particle filter
  pf = pfilter(1000, &map, landmarks, 24, 24, M_PI_2);
  pf.set_size(12);
  pf.set_noise(8.0, 2.0);

  // start up the window
  screen = initSDL(map.n_cols, map.n_rows);
  if (!screen) {
    return 1;
  }
  bool forward = false;
  bool backward = false;
  bool turn_left = false;
  bool turn_right = false;
  bool strafe_left = false;
  bool strafe_right = false;
  bool quit = false;

  // load the lidar
  //sim_lidar lidar(&robot);
  //lidar.set_noise(1.0, 0.1);
  std::thread detect(update_chili);
//  std::thread draw(draw_things);

  while (!quit) {
    // see if something is about to quit

    SDL_PumpEvents();
		Uint8 *keystates = SDL_GetKeyState(NULL);

		// Front left, front right, back left, back right
		if (keystates[SDLK_q])			  { drive(-1,  1, -1,  1); }  // left turn
		else if(keystates[SDLK_e])    { drive( 1, -1,  1, -1); }  // right turn
		else if(keystates[SDLK_w]) 	  { drive( 1,  1,  1,  1); }  // up
		else if(keystates[SDLK_s]) 	  { drive(-1, -1, -1, -1); }  // down
		else if(keystates[SDLK_a]) 	  { drive(-1,  1,  1, -1); }  // left strafe
		else if(keystates[SDLK_d]) 	  { drive( 1, -1, -1,  1); }  // right strafe
		else if(keystates[SDLK_1]) 		{ drive( 1,  0,  0,  0); }
		else if(keystates[SDLK_2]) 		{ drive( 0,  1,  0,  0); }
		else if(keystates[SDLK_3]) 		{ drive( 0,  0,  1,  0); }
		else if(keystates[SDLK_4]) 		{ drive( 0,  0,  0,  1); }

		else
		{
			drive(0, 0, 0, 0);
		}

		if(keystates[SDLK_x])
		{
      printf("QUITTING\n");
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
    printf("[sim.cpp] drive: %d %d %d %d %d %d\n", forward, backward, turn_left, turn_right, strafe_left, strafe_right);

    // update the robot
    //mat sensor_values = lidar.sense();
    //mat tag_landmarks;
    //sim_tag_extract(tag_landmarks, sensor_values, landmarks, robot, map);

    // update movement
    //robot.move((forward - backward), (strafe_right - strafe_left), (turn_left - turn_right) * .1);
    rose.send(motion);
    pf.move((strafe_right - strafe_left), (forward - backward), (turn_left - turn_right) * .1);

    // observe
    tag_landmarks = sense();
    cout << "sensed:\n" << tag_landmarks << endl;
    pf.observe(tag_landmarks);

    // predict the position
    vec mu;
    mat sigma;
    pf.predict(mu, sigma);
    cout << "position: " << mu(0) << ", " << mu(1) << ", angle: " << mu(2) * 180 / M_PI << "\nerror: \n" << sigma << endl;

    draw_things();
  }

  rose.disconnect();

  // clean up
  SDL_Quit();
}

void draw_things()
{
    sleep(1);
    while (1)
    {
        // put stuff on the screen
        map.blit(frame);
        for (sim_landmark &lm : landmarks) {
          lm.blit(frame);
        }
        ivec cyan({0,255,255});
        for (int i = 0; i < landmarks.size(); i++)
        {
          if (tag_landmarks.n_cols > landmarks.size()) {
            break;
          }
          if (tag_landmarks(2,i) > 0.5) {
            vec pt({(double)landmarks[i].x,(double)landmarks[i].y});
            draw_circle(frame, cyan, pt, tag_landmarks(1,i)-1);
            draw_circle(frame, cyan, pt, tag_landmarks(1,i));
            draw_circle(frame, cyan, pt, tag_landmarks(1,i)+1);
          }
        }
        //lidar.blit(frame);
        pf.blit(frame);
        //robot.blit(frame);
        screenblit(screen, frame);

        // draw the screen
        SDL_Flip(screen);
        SDL_Delay(25);
    }
}
