#ifndef sim_window_h
#define sim_window_h

#include <SDL2/SDL.h>

#define XY2P(x, y, width, height) ((((height) - (y) - 1) * (width)) + (x))

namespace sim_window {
  SDL_Surface *init(int width, int height);
  SDL_Event *get_event(void);
  void update(void);
  void destroy(void);
}

#endif
