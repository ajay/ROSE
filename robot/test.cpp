#include <armadillo>
#include <signal.h>
#include <thread>
#include "SDL/SDL.h"

#include "Rose.h"

SDL_Event event;

static int stopsig;
using namespace arma;
static Rose rose;
static arma::vec motion = zeros<vec>(4);

static bool test_flag = false;

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

bool initSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	SDL_Surface *screen = SDL_SetVideoMode(200, 200, 32, SDL_SWSURFACE);

	if(screen == NULL)
	{
		return false;
	}

	SDL_WM_SetCaption("rose", NULL);

	return true;
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

	if(initSDL() == false)
	{
		return 1;
	}

	bool quit = false;

	while(!quit)
	{
		SDL_PumpEvents();
		Uint8 *keystates = SDL_GetKeyState(NULL);

		// Front left, front right, back left, back right
		if (keystates[SDLK_a])			{ drive(-1,  1, -1,  1); }
		else if(keystates[SDLK_s]) 		{ drive( 1, -1,  1, -1); }
		else if(keystates[SDLK_UP]) 	{ drive( 1,  1,  1,  1); }
		else if(keystates[SDLK_DOWN]) 	{ drive(-1, -1, -1, -1); }
		else if(keystates[SDLK_LEFT]) 	{ drive(-1,  1,  1, -1); }
		else if(keystates[SDLK_RIGHT]) 	{ drive( 1, -1, -1,  1); }
		else if(keystates[SDLK_1]) 		{ drive( 1,  0,  0,  0); }
		else if(keystates[SDLK_2]) 		{ drive( 0,  1,  0,  0); }
		else if(keystates[SDLK_3]) 		{ drive( 0,  0,  1,  0); }
		else if(keystates[SDLK_4]) 		{ drive( 0,  0,  0,  1); }

		else
		{
			drive(0, 0, 0, 0);
		}

		if(keystates[SDLK_q])
		{
			quit = true;
			SDL_Quit();
		}

		rose.send(motion);
	}
	SDL_Quit();
	return 0;
}
