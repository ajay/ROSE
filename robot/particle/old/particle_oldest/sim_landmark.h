#ifndef sim_landmark_h
#define sim_landmark_h

#include <armadillo>
#include <SDL2/SDL.h>
#include "sim_map.h"
#include "sim_robot.h"

class sim_landmark {
  public:
    sim_landmark(double x, double y);
    double collision(sim_map *map, arma::vec pos);
    arma::vec sense(sim_robot &robot, arma::mat lidarvals = arma::mat(), int flags = 0);
    void blit(arma::icube &screen);

    double x;
    double y;
};

#endif
