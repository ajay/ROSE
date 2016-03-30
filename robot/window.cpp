#include "window.h"

SDL_Event event;

SDL_Surface* initSDL()
{
	SDL_Init(SDL_INIT_VIDEO);

	int width = 640;
	int height = 480;

	window = SDL_CreateWindow("Rose", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	screen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	texture = SDL_CreateTextureFromSurface(renderer, screen);

	if (TTF_Init() != 0)
	{
		SDL_Quit();
	}

	return screen;
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
	// Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr)
	{
		return nullptr;
	}

	// We need to first render to a surface as that's what TTF_RenderText
	// returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr)
	{
		TTF_CloseFont(font);
		return nullptr;
	}

	// printf("past close\n");
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr)
	{
		SDL_Quit();
		return nullptr;
	}

	// Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);

	return texture;
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip)
{
	SDL_RenderCopy(ren, tex, clip, &dst);
}

void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip)
{
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr)
	{
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else
	{
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
}

SDL_Window* get_window()
{
	return window;
}

SDL_Renderer* get_renderer()
{
	return renderer;
}

SDL_Texture* get_texture()
{
	return texture;
}