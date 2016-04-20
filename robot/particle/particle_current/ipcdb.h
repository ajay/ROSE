#ifndef IPCDB_H
#define IPCDB_H

#include <armadillo>
#include <vector>

#include "chili_landmarks.h"
#include "Rose.h"
#include "pfilter.h"

// for stopping the robot, no matter what
static int stopsig;

// for markers
static chili_landmarks chili;
static std::mutex chili_lock;
static arma::mat chilitags(3, 20, arma::fill::zeros);

// for sending motion to the robot
static Rose rose;

// for enabling autonomous
static std::mutex autonomous_lock;
static bool auto_enable;
static bool auto_confirmed;
static bool manual_confirmed;

// for getting the position and map
static std::mutex pose_lock;
static arma::vec robot_pose(3, arma::fill::zeros); // x, y, theta
static pfilter pf; // !! takes a long time to blit
static std::mutex map_lock;
static sim_map globalmap;
static std::vector<sim_landmark> landmarks;

// for getting the planned path
static std::mutex path_lock;
static arma::mat pathplan(2, 0, arma::fill::zeros);
static bool dopose;
static arma::vec poseplan(3, arma::fill::zeros);
static double twistplan;
static double grabplan;

// for displaying stuff
static SDL_Surface *screen;

#endif