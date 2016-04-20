#ifndef SIM_WINDOW_H
#define SIM_WINDOW_H

#include <armadillo>
#include <SDL2/SDL.h>

namespace sim_window
{
	SDL_Surface *init(int width, int height);
	SDL_Event *get_event(void);
	void blit(SDL_Surface *s, arma::cube &frame);
	void update(void);
	void destroy(void);
}

#endif