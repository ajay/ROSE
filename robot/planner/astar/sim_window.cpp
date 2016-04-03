#include "sim_window.h"

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *screen;
static SDL_Texture *texture;
static SDL_Event event;

SDL_Surface *sim_window::init(int width, int height) {
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("simulation",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      width, height, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
  texture = SDL_CreateTextureFromSurface(renderer, screen);
  return screen;
}

SDL_Event *sim_window::get_event(void) {
  if (SDL_PollEvent(&event)) {
    return &event;
  } else {
    return NULL;
  }
}

void sim_window::update(void) {
  SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void sim_window::destroy(void) {
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(screen);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
