#ifndef sim_map_h
#define sim_map_h

#include <string>
#include <armadillo>
#include <SDL2/SDL.h>
#include "sdldef.h"

class sim_map {
  public:
    sim_map(void);
    ~sim_map(void);
    void load(const std::string &map_name);
    void blit(arma::icube &screen);

    arma::mat map;
    arma::uword n_rows;
    arma::uword n_cols;
};

#endif
