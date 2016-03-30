#ifndef sim_robot_h
#define sim_robot_h

#include "sim_map.h"
#include <SDL2/SDL.h>
#include <string>
#include <armadillo>

class sim_robot {
  public:
    sim_robot(sim_map *map = NULL);
    ~sim_robot(void);
    void set_size(double r);
    void set_pose(double x, double y, double t);
    void set_noise(double vs, double ws);
    void attach_lidar(void *lidar);
    void move(double vx, double vy, double w);
    void blit(arma::icube &screen);

    double x;
    double y;
    double t;
    double r;
    double vs;
    double ws;
    sim_map *map;
    void *lidar;

  private:
    bool collided(double x, double y);
};

#endif
