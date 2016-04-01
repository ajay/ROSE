#include <armadillo>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <string>

#include "Rose.h"
#include "window.h"
#include "dbconn.h"

using namespace std;

static int stopsig;
using namespace arma;
static Rose rose;
static arma::vec motion = zeros<vec>(4);
static bool test_flag = false;
bool pid_kill = true;
bool webapp_control = false;

static dbconn db;

void db_update()
{
	db.db_update();
}

void print_db_data()
{
	while (1)
	{
		printf("State Received: %s\n", db.rose_data_recv.direction.c_str());
		printf("Speed Received: %1.2f\n", db.rose_data_recv.speed);
		usleep(100000); // 1 sec
	}
}

// Takes in four integers and assignments them to motion
void drive(double frontLeft, double frontRight, double backLeft, double backRight)
{
	motion[0] = frontLeft;
	motion[1] = frontRight;
	motion[2] = backLeft;
	motion[3] = backRight;
}

void drive(string direction, double v)
{
	if		(direction == "STOP")						{ drive( 0,  0,  0,  0); }

	else if	(direction == "NORTH")						{ drive( v,  v,  v,  v); }
	else if (direction == "SOUTH")						{ drive(-v, -v, -v, -v); }
	else if (direction == "WEST")						{ drive(-v,  v,  v, -v); }
	else if (direction == "EAST")						{ drive( v, -v, -v,  v); }

	else if (direction == "CLOCKWISE")					{ drive( v, -v,  v, -v); }
	else if	(direction == "COUNTERCLOCKWISE")			{ drive(-v,  v, -v,  v); }

	else if (direction == "NORTHWEST")					{ drive( 0,  v,  v,  0); }
	else if (direction == "NORTHEAST")					{ drive( v,  0,  0,  v); }
	else if (direction == "SOUTHWEST")					{ drive(-v,  0,  0, -v); }
	else if (direction == "SOUTHEAST")					{ drive( 0, -v, -v,  0); }

	else if (direction == "NORTHCLOCKWISE")				{ drive( v,    v/2,    v,  v/2); }
	else if (direction == "NORTHCOUNTERCLOCKWISE")		{ drive( v/2,    v,  v/2,    v); }
	else if (direction == "SOUTHCLOCKWISE")				{ drive(-v/2,   -v, -v/2,   -v); }
	else if (direction == "SOUTHCOUNTERCLOCKWISE")		{ drive(-v,   -v/2,   -v, -v/2); }

	else if (direction == "ONE")						{ drive( v,  0,  0,  0); }
	else if (direction == "TWO")						{ drive( 0,  v,  0,  0); }
	else if (direction == "THREE")						{ drive( 0,  0,  v,  0); }
	else if (direction == "FOUR")						{ drive( 0,  0,  0,  v); }
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
	stopsig = 1;
	rose.startStop = true;
	rose.disconnect();
	exit(1);
}

void print_SDL(std::ostringstream& str, int size, int x, int y)
{
	SDL_Renderer* renderer = get_renderer();
	SDL_Color color = { 255, 255, 255, 255 };
	std::string font = "fonts/roboto.ttf";

	std::string str_string = str.str();
	SDL_Texture *str_image = renderText(str_string, font, color, size, renderer);
	renderTexture(str_image, renderer, x, y);
}

int main(int argc, char *argv[])
{
	std::thread database(db_update);
	// std::thread database_debug(print_db_data);

	std::thread pid(driveStraight);

	rose.startStop = false;
	signal(SIGINT, stop);

	SDL_Surface *screen = initSDL();
	bool quit = false;
	double v = 0.5; // velocity
	string direction = "STOP";
	SDL_Event event;

	SDL_Window* window = get_window();
	SDL_Renderer* renderer = get_renderer();
	SDL_Texture* texture = get_texture();

	while(!quit)
	{
		std::ostringstream speed, voltage, current, db_direction, db_speed, webapp;

		speed  << "speed: " << std::setprecision(2) << v;
		voltage  << "12V Voltage: " << std::setprecision(4) << rose.twelve_volt_voltage << " V";
		current  << "12V Current: " << std::setprecision(4) << rose.twelve_volt_current << "A";
		db_direction << "DB Direction: " << db.rose_data_recv.direction;
		db_speed << "DB Speed: " << std::setprecision(2) << db.rose_data_recv.speed;
        std::string web_on_off = webapp_control ? "On" : "Off";
        webapp << "Webapp Control: " << web_on_off;

		SDL_RenderClear(renderer);
		print_SDL(speed, 32, 10, 10);
		print_SDL(voltage, 32, 10, 50);
		print_SDL(current, 32, 10, 90);
        print_SDL(webapp, 20, 10, 130);
        print_SDL(db_direction, 20, 10, 160);
		print_SDL(db_speed, 20, 10, 190);
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

		else if (keystates[SDL_SCANCODE_PAGEUP])		{ direction = "COUNTERCLOCKWISE"; }
		else if (keystates[SDL_SCANCODE_PAGEDOWN])	    { direction = "CLOCKWISE"; }
		else if (keystates[SDL_SCANCODE_UP])		    { direction = "NORTH"; }
		else if (keystates[SDL_SCANCODE_DOWN])		    { direction = "SOUTH"; }
		else if (keystates[SDL_SCANCODE_LEFT])		    { direction = "WEST"; }
		else if (keystates[SDL_SCANCODE_RIGHT]) 		{ direction = "EAST"; }
		else if (keystates[SDL_SCANCODE_1]) 			{ direction = "ONE"; }
		else if (keystates[SDL_SCANCODE_2]) 			{ direction = "TWO"; }
		else if (keystates[SDL_SCANCODE_3]) 			{ direction = "THREE"; }
		else if (keystates[SDL_SCANCODE_4]) 			{ direction = "FOUR"; }

		else if (keystates[SDL_SCANCODE_O]) 			{ pid_kill = false; }
		else if (keystates[SDL_SCANCODE_P]) 			{ pid_kill = true; }

        else if (keystates[SDL_SCANCODE_Z]) 			{ webapp_control = true; }
        else if (keystates[SDL_SCANCODE_X]) 			{ webapp_control = false; }

		else                                            { direction = "STOP"; }

        if (webapp_control == true)
		{
			direction = db.rose_data_recv.direction;
			v = db.rose_data_recv.speed;
		}

		drive(direction, v);

		if(keystates[SDL_SCANCODE_Q])
		{
			quit = true;
		}

		if (pid_kill)
		{
			rose.send(motion);
		}
	}
	SDL_Quit();
	stop(0);
}
