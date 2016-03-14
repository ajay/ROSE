#ifndef pfilter_h
#define pfilter_h

#include "sim_map.h"
#include "sim_robot.h"
#include "sim_landmark.h"
#include <vector>
#include <armadillo>

class pfilter {
  public:
    pfilter(void);
    pfilter(int nparticles, sim_map *map, std::vector<sim_landmark> &landmarks, int x, int y, int t);
    ~pfilter(void);
    void move(double vx, double vy, double w);
    void observe(arma::mat observations);
    void predict(arma::vec &mu, arma::mat &sigma);
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
