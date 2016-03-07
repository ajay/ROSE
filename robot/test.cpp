#include <armadillo>
#include <signal.h>
#include <thread>
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#include "Rose.h"

SDL_Event event;

static int stopsig;
using namespace arma;
static Rose rose;
static arma::vec motion = zeros<vec>(4);
bool drive_kill = true;

static bool test_flag = false;


static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *screen;
static SDL_Texture *texture;




// Kendrick:
// Test flag is updated here
// Only add code here for mongo connection
void update_flag()
{
	// test_flag = <something>
	printf("test_flag is: %d\n", test_flag);
}

void drive(double frontLeft, double frontRight, double backLeft, double backRight)
{
	motion[0] = frontLeft;
	motion[1] = frontRight;
	motion[2] = backLeft;
	motion[3] = backRight;
}


// Takes in a value between -1 and 1, and drives straight
// until a drive_kill flag is tripped
void driveStraight(double speed)
{
    speed = 0.5;
    drive_kill = false;
    while (!drive_kill)
    {
        // Put pid stuff here

        // 4 Wheels
        // motion[0] --> rightFront wheel
        // takes in value between -1 and 1 (1 is full speed forward)
        // Ignore speed parameter for now, and go at 0.5
        // base_velocity = 0.5
        // read encoders through array (rose->encoders[0])
        // leftFront, rightFront, leftBack, rightBack- encoders and motors


    }
}

SDL_Surface* initSDL()
{
	SDL_Init(SDL_INIT_VIDEO);

	int width = 200;
	int height = 200;

	window = SDL_CreateWindow("simulation",
	      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	      width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1,	      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	screen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	texture = SDL_CreateTextureFromSurface(renderer, screen);

	if (TTF_Init() != 0)
	{
		// logSDLError(std::cout, "TTF_Init");
		SDL_Quit();
		// return 1;
	}

	return screen;
}

/**
* Render the message we want to display to a texture for drawing
* @param message The message we want to display
* @param fontFile The font we want to use to render the text
* @param color The color we want the text to be
* @param fontSize The size we want the font to be
* @param renderer The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
*/
SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr){
		// logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr){
		TTF_CloseFont(font);
		// logSDLError(std::cout, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr){
		// logSDLError(std::cout, "CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

/*
 * Draw an SDL_Texture to an SDL_Renderer at some destination rect
 * taking a clip of the texture if desired
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param dst The destination rectangle to render the texture too
 * @param clip The sub-section of the texture to draw (clipping rect)
 *		default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr){
	SDL_RenderCopy(ren, tex, clip, &dst);
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height and taking a clip of the texture if desired
 * If a clip is passed, the clip's width and height will be used instead of the texture's
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param clip The sub-section of the texture to draw (clipping rect)
 *		default of nullptr draws the entire texture
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr){
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr){
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
}


void stop(int signo)
{
	printf("Exiting yo >>>>\n");
	rose.startStop = true;
	stopsig = 1;
	exit(1);
}

int main(int argc, char *argv[])
{
	std::thread db_update(update_flag);

	rose.startStop = false;
	signal(SIGINT, stop);

	SDL_Surface *screen = initSDL();

	bool quit = false;

	double v = 0.5; // velocity


	SDL_Event event;



	while(!quit)
	{

		// SDL TTF //
		// const std::string resPath = getResourcePath("Lesson6");
		//We'll render the string "TTF fonts are cool!" in white
		//Color is in RGBA format
		SDL_Color color = { 255, 255, 255, 255 };

		std::ostringstream stringStream;
		stringStream << "speed: " << v;
		std::string test = stringStream.str();

		SDL_Texture *image = renderText(test, "sketchy.ttf",
			color, 32, renderer);
		if (image == nullptr){
			// cleanup(renderer, window);
			TTF_Quit();
			SDL_Quit();
			return 1;
		}
		//Get the texture w/h so we can center it in the screen
		int iW, iH;
		SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
		int x = 200 / 2 - iW / 2;
		int y = 200 / 2 - iH / 2;

		//Note: This is within the program's main loop
		SDL_RenderClear(renderer);
		//We can draw our message as we do any other texture, since it's been
		//rendered to a texture
		renderTexture(image, renderer, 10, 10);
		SDL_RenderPresent(renderer);


		SDL_PollEvent(&event);
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);

		// Front left, front right, back left, back right

		if (event.type == SDL_KEYDOWN)
		{
			if (keystates[SDL_SCANCODE_A] && (v < 0.9))
			{
				v += 0.1;
			}
			else if(keystates[SDL_SCANCODE_S] && (v > -0.9))
			{
				v -= 0.1;
			}
		}



		if (keystates[SDL_SCANCODE_PAGEUP])			{ drive(-v,  v, -v,  v); }
		else if(keystates[SDL_SCANCODE_PAGEDOWN])	{ drive( v, -v,  v, -v); }
		else if(keystates[SDL_SCANCODE_UP]) 		{ drive( v,  v,  v,  v); }
		else if(keystates[SDL_SCANCODE_DOWN]) 		{ drive(-v, -v, -v, -v); }
		else if(keystates[SDL_SCANCODE_LEFT]) 		{ drive(-v,  v,  v, -v); }
		else if(keystates[SDL_SCANCODE_RIGHT]) 		{ drive( v, -v, -v,  v); }
		else if(keystates[SDL_SCANCODE_1]) 			{ drive( v,  0,  0,  0); }
		else if(keystates[SDL_SCANCODE_2]) 			{ drive( 0,  v,  0,  0); }
		else if(keystates[SDL_SCANCODE_3]) 			{ drive( 0,  0,  v,  0); }
		else if(keystates[SDL_SCANCODE_4]) 			{ drive( 0,  0,  0,  v); }

		else
		{
			drive(0, 0, 0, 0);
		}

		// printf("[test.cpp] speed = %f\n\n", v);

		if(keystates[SDL_SCANCODE_Q])
		{
			quit = true;
			SDL_Quit();
		}

		rose.send(motion);
	}
	SDL_Quit();
	return 0;
}
