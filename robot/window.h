#ifndef WINDOW_H
#define WINDOW_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <armadillo>
#include <string>
#include <cstring>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *screen;
static SDL_Texture *texture;

SDL_Surface* initSDL();
SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr);

SDL_Window* get_window();
SDL_Renderer* get_renderer();
SDL_Texture* get_texture();

#endif