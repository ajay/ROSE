#include "sim_map.h"
#include "sim_robot.h"
#include "sim_lidar.h"
#include "sim_window.h"
#include "pfilter.h"
#include "sim_landmark.h"
#include <cstdio>
#include <armadillo>
#include <SDL2/SDL.h>
#include <cassert>
#include <iostream>
#include <cstdlib>

using namespace std;
using namespace arma;

// This is a simpler landmark extractor with the goal of extracting the location of the landmarks easily
void sim_tag_extract(mat &tag_landmarks,
    mat &sensor_vals,
    vector<sim_landmark> &landmarks,
    sim_robot &robot,
    sim_map &map) {
  tag_landmarks = mat(2, landmarks.size());
  for (int lid = 0; lid < landmarks.size(); lid++) {
    tag_landmarks.col(lid) = landmarks[lid].sense(robot);
  }
}

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
  map.load("maze1.png");

  // create the landmarks (custom)
  vector<sim_landmark> landmarks;
  landmarks.push_back(sim_landmark(40,167));
  landmarks.push_back(sim_landmark(100, 115));
  landmarks.push_back(sim_landmark(153, 160));
  landmarks.push_back(sim_landmark(84, 78));
  landmarks.push_back(sim_landmark(128, 75));
  landmarks.push_back(sim_landmark(45, 36));

  // load the robot
  sim_robot robot(&map);
  robot.set_size(8);
  robot.set_pose(70, 48, 0);
  robot.set_noise(0.3, 0.02);

  // load the lidar
  sim_lidar lidar(&robot);
  lidar.set_noise(1.0, 0.1);

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

  for (;;) {
    // see if something is about to quit
    SDL_Event *e;
    bool done = false;
    while ((e = sim_window::get_event())) {
      if (e->type == SDL_QUIT) {
        done = true;
      } else if (e->type == SDL_KEYDOWN) {
        // attempt to see if the user wants to make the robot move
        if (e->key.keysym.sym == SDLK_LEFT)  { left = true; }
        if (e->key.keysym.sym == SDLK_RIGHT) { right = true; }
        if (e->key.keysym.sym == SDLK_UP)    { forward = true; }
      } else if (e->type == SDL_KEYUP) {
        if (e->key.keysym.sym == SDLK_LEFT)  { left = false; }
        if (e->key.keysym.sym == SDLK_RIGHT) { right = false; }
        if (e->key.keysym.sym == SDLK_UP)    { forward = false; }
      }
    }
    if (done) {
      break;
    }

    // update the robot
    mat sensor_values = lidar.sense();
    mat tag_landmarks;
    sim_tag_extract(tag_landmarks, sensor_values, landmarks, robot, map);
    pf.observe(tag_landmarks);
    robot.move(forward * 2, (left - right) * .1);
    pf.move(forward * 2, (left - right) * .1);

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
    newframe = doubleImageSize(frame);
    sim_window::blit(screen, newframe);
    SDL_Delay(25);

    // draw the screen
    sim_window::update();
  }

  // clean up
  sim_window::destroy();
}
