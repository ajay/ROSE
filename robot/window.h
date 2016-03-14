#ifndef WINDOW_H
#define WINDOW_H

#include <armadillo>
#include <cstring>
#include <string>

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

// SDL2 things we need to display
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *screen;
static SDL_Texture *texture;

/**
 * Initializes SDL Window, as well as SDL_TTF
 * @return pointer to the screen created in init
 */
SDL_Surface* initSDL();

/**
* Render the message we want to display to a texture for drawing
* @param message
*        The message we want to display
* @param fontFile
*        The font we want to use to render the text
* @param color
*        The color we want the text to be
* @param fontSize
*        The size we want the font to be
* @param renderer
*        The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
*/
SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer);

/*
 * Draw an SDL_Texture to an SDL_Renderer at some destination rect
 * taking a clip of the texture if desired
 * @param tex
 *        The source texture we want to draw
 * @param rend
 *        The renderer we want to draw too
 * @param dst
 *        The destination rectangle to render the texture too
 * @param clip
 *        The sub-section of the texture to draw (clipping rect)
 *        default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr);

/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height and taking a clip of the texture if desired
 * If a clip is passed, the clip's width and height will be used instead of the texture's
 * @param tex
 *        The source texture we want to draw
 * @param rend
 *        The renderer we want to draw too
 * @param x
 *        The x coordinate to draw too
 * @param y
 *        The y coordinate to draw too
 * @param clip
 *        The sub-section of the texture to draw (clipping rect)
 *        default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr);

/**
 * @return pointer to SDL_Window that was created in initSDL()
 */
SDL_Window* get_window();

/**
 * @return pointer to SDL_Renderer that was created in initSDL()
 */
SDL_Renderer* get_renderer();

/**
 * @return pointer to SDL_Texture that was created in initSDL()
 */
SDL_Texture* get_texture();

#endif