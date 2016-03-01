#include <armadillo>
#include <signal.h>
#include <thread>
#include "SDL2/SDL.h"
// #include "SDL_ttf.h"

#include "Rose.h"

SDL_Event event;

static int stopsig;
using namespace arma;
static Rose rose;
static arma::vec motion = zeros<vec>(4);
bool drive_kill = true;

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

	static SDL_Window *window = SDL_CreateWindow("simulation",
	      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	      width, height, SDL_WINDOW_SHOWN);
	static SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,	      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	static SDL_Surface *screen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	static SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, screen);

	return screen;
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

	while(!quit)
	{
		SDL_PumpEvents();
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);

		// Front left, front right, back left, back right

		if (keystates[SDL_SCANCODE_A])
		{
			if (!(keystates[SDL_SCANCODE_A]))
			{
				v += 0.1;
			}
		}
		else if(keystates[SDL_SCANCODE_S])
		{
			if (!(keystates[SDL_SCANCODE_S]))
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
