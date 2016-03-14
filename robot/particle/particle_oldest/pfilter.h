#ifndef pfilter_h
#define pfilter_h

#include "sim_map.h"
#include "sim_robot.h"
#include "sim_landmark.h"
#include <vector>
#include <armadillo>
#include <SDL2/SDL.h>

class pfilter {
  public:
    pfilter(int nparticles, sim_map *map, std::vector<sim_landmark> &landmarks);
    ~pfilter(void);
    void move(double v, double w);
    void observe(arma::mat observations);
    void predict(arma::vec &mu, double &sigma);
    void set_noise(double vs, double ws);
    void set_size(double r);
    void blit(arma::icube &screen);

    std::vector<sim_robot> particles;
    sim_map *map;

  private:
    void weigh(arma::mat &observations);
    void resample(void);

    double vs;
    double ws;
    arma::vec health;
    std::vector<sim_landmark> landmarks;
};

#endif
