#ifndef sim_window_h
#define sim_window_h

#include <SDL2/SDL.h>
#include <armadillo>

namespace sim_window {
  SDL_Surface *init(int width, int height);
  SDL_Event *get_event(void);
  void blit(SDL_Surface *s, arma::cube &frame);
  void update(void);
  void destroy(void);
}

#endif
