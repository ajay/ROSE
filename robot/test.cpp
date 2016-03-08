#include <armadillo>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <iomanip>

#include "Rose.h"
#include "window.h"

static int stopsig;
using namespace arma;
static Rose rose;
static arma::vec motion = zeros<vec>(4);
static bool test_flag = false;
bool pid_kill = true;

// Kendrick:
// Test flag is updated here
// Only add code here for mongo connection
void update_flag()
{
	// test_flag = <something>
	// printf("test_flag is: %d\n", test_flag);
}

void drive(double frontLeft, double frontRight, double backLeft, double backRight)
{
	motion[0] = frontLeft;
	motion[1] = frontRight;
	motion[2] = backLeft;
	motion[3] = backRight;
}

// Takes in a value between -1 and 1, and drives straight
// until a pid_kill flag is tripped
void driveStraight()
{
	double k_p = 0.005;
	double k_i = 0.001;
	double k_d = 0.001;

	while (1)
	{
		while (pid_kill)
		{
			sleep(0.1);
		}

		double speed = 0.9;
		rose.reset_encoders();
		int average = 0;

		arma::vec error = zeros<vec>(4);
		arma::vec error_sum = zeros<vec>(4);
		arma::vec prev_error = zeros<vec>(4);
		arma::vec error_diff = zeros<vec>(4);

		while (!pid_kill)
		{
			// Average is now the master
			int average = (rose.encoder[0] + rose.encoder[1] + rose.encoder[2] + rose.encoder[3]) / 4;

			if (average >= 360)
			{
				pid_kill = true;
			}

			for (unsigned int i = 0; i < 4; i++)
			{
				error[i] = average - rose.encoder[i];
				error_sum[i] += error[i];
				error_diff[i] = error[i] - prev_error[i];
				prev_error[i] = error[i];
			}

			// Slaves
			motion[0] = speed + (error[1] * k_p) + (error_sum[1] * k_i) + (error_diff[1] * k_d);
			motion[1] = speed + (error[1] * k_p) + (error_sum[1] * k_i) + (error_diff[1] * k_d);
			motion[2] = speed + (error[2] * k_p) + (error_sum[2] * k_i) + (error_diff[2] * k_d);
			motion[3] = speed + (error[3] * k_p) + (error_sum[3] * k_i) + (error_diff[3] * k_d);

			printf("average: [%d]\n", average);
			printf("rose_encoder: [%d %d %d %d]\n", (int)rose.encoder[0], (int)rose.encoder[1], (int)rose.encoder[2], (int)rose.encoder[3]);
			printf("error: [%d %d %d %d]\n", (int)error[0], (int)error[1], (int)error[2], (int)error[3]);
			printf("error_sum: [%d %d %d %d]\n", (int)error_sum[0], (int)error_sum[1], (int)error_sum[2], (int)error_sum[3]);
			printf("error_diff: [%d %d %d %d]\n", (int)error_diff[0], (int)error_diff[1], (int)error_diff[2], (int)error_diff[3]);
			printf("motion: [%2.5f %2.5f %2.5f %2.5f]\n\n", motion[0], motion[1], motion[2], motion[3]);

			rose.send(motion);
			usleep(100000); // 100ms
		}
	}
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
	std::thread pid(driveStraight);

	rose.startStop = false;
	signal(SIGINT, stop);

	SDL_Surface *screen = initSDL();
	bool quit = false;
	double v = 0.5; // velocity
	SDL_Event event;

	SDL_Window* window = get_window();
	SDL_Renderer* renderer = get_renderer();
	SDL_Texture* texture = get_texture();

	while(!quit)
	{
		SDL_Color color = { 255, 255, 255, 255 };

		std::ostringstream stringStream;
		stringStream  << "speed: " << std::setprecision(2) << v;
		std::string test = stringStream.str();

		SDL_Texture *image = renderText(test, "fonts/roboto.ttf", color, 32, renderer);

		if (image == nullptr)
		{
			TTF_Quit();
			SDL_Quit();
			return 1;
		}

		// SDL Stuff
		int iW, iH;
		SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
		int x = 200 / 2 - iW / 2;
		int y = 200 / 2 - iH / 2;

		SDL_RenderClear(renderer);
		renderTexture(image, renderer, 10, 10);
		SDL_RenderPresent(renderer);

		SDL_PollEvent(&event);
		const Uint8 *keystates = SDL_GetKeyboardState(NULL);

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

		if(keystates[SDL_SCANCODE_R])
		{
			rose.reset_encoders();
			drive(0.0001, -0.001, 0.0001, 0.001);
		}

		else if(keystates[SDL_SCANCODE_PAGEUP])		{ drive(-v,  v, -v,  v); }
		else if(keystates[SDL_SCANCODE_PAGEDOWN])	{ drive( v, -v,  v, -v); }
		else if(keystates[SDL_SCANCODE_UP]) 		{ drive( v,  v,  v,  v); }
		else if(keystates[SDL_SCANCODE_DOWN]) 		{ drive(-v, -v, -v, -v); }
		else if(keystates[SDL_SCANCODE_LEFT]) 		{ drive(-v,  v,  v, -v); }
		else if(keystates[SDL_SCANCODE_RIGHT]) 		{ drive( v, -v, -v,  v); }
		else if(keystates[SDL_SCANCODE_1]) 			{ drive( v,  0,  0,  0); }
		else if(keystates[SDL_SCANCODE_2]) 			{ drive( 0,  v,  0,  0); }
		else if(keystates[SDL_SCANCODE_3]) 			{ drive( 0,  0,  v,  0); }
		else if(keystates[SDL_SCANCODE_4]) 			{ drive( 0,  0,  0,  v); }

		else if(keystates[SDL_SCANCODE_O]) 			{ pid_kill = false; }
		else if(keystates[SDL_SCANCODE_P]) 			{ pid_kill = true; }

		else
		{
			drive(0, 0, 0, 0);
		}

		if(keystates[SDL_SCANCODE_Q])
		{
			quit = true;
			// SDL_Quit();
		}
		if (pid_kill)
		{
			rose.send(motion);
		}
	}

	rose.startStop = true;
	rose.disconnect();
	stopsig = 1;
	SDL_Quit();
	return 0;
}