#ifndef xbot_h
#define xbot_h

#include <vector>
#include <armadillo>
#include "sim_map.h"

class xbot {
  public:
    xbot(sim_map *map);
    ~xbot(void);
    void set_size(double r);
    void set_apriori(double x, double y, double t);
    void set_noise(double vs, double ws);
    void update_belief(double v, double w);
    void update_observations(std::vector<arma::vec> observations);

    arma::vec pos;
    double theta;
    double radius;
    arma::vec sigma;
    sim_map *map;
    double health;
};

#endif
