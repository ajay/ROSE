#include <armadillo>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include "SDL/SDL.h"

#include "astar.h"
#include "chili_landmarks.h"
#include "draw.h"
#include "ipcdb.h"
#include "mathfun.h"
#include "pfilter.h"
#include "Rose.h"
#include "sdldef.h"
#include "sim_landmark.h"
#include "sim_map.h"
#include "sim_robot.h"

using namespace arma;
using namespace std;

// threads in this file
void manual_input(void);
void chilitag_detect(void);
void localize_pose(void);
void robot_calcmotion(void);
void motion_plan(void);
void display_interface(void);

static double secdiff(struct timeval &t1, struct timeval &t2);

static void stoprunning(int signum)
{
	stopsig = 1;
	rose.startStop = true;
	alarm(1); // hack to get this to force quit
}

static void forcequit(int signum)
{
	printf("Force quitting...\n");
	exit(1);
}

static SDL_Surface *initSDL(int w, int h)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return NULL;
	}

	SDL_Surface *screen = SDL_SetVideoMode(w,h, 32, SDL_SWSURFACE);
	if (screen == NULL)
	{
		return NULL;
	}

	SDL_WM_SetCaption("rose", NULL);

	return screen;
}

static void screenblit(SDL_Surface *s, cube &frame)
{
	for (int i = 0; i < (int)frame.n_rows; i++)
	{
		for (int j = 0; j < (int)frame.n_cols; j++)
		{
			uint32_t color =
				((uint8_t)((int)round(frame(i,j,0) * 255)) << 16) |
				((uint8_t)((int)round(frame(i,j,1) * 255)) << 8) |
				((uint8_t)((int)round(frame(i,j,2) * 255)) << 0);
			((uint32_t *)s->pixels)[XY2P(j, i, s->w, s->h)] = color;
		}
	}
}

static void chilicamdetect_thread(void)
{
	chili.update();
}

int main()
{
	// preemptive init
	printf("[main] preemptive init\n");
	signal(SIGINT, stoprunning);
	signal(SIGALRM, forcequit);
	screen = initSDL(500, 500);

	if (!screen)
	{
		printf("No screen found, please check your SDL configurations\n");
		return 1;
	}

	double initial_x = 75;
	double initial_y = 60;
	double initial_t = 90;
	robot_pose = vec({ initial_x, initial_y, initial_t });
	globalmap.load("ece_hallway_partial.jpg"); // lower corner is (0,0)

	// start up the threads
	printf("[main] start up the threads\n");
	rose.startStop = false;
	thread manual_thread(manual_input);
	thread chilicamthread(chilicamdetect_thread);
	thread chili_thread(chilitag_detect);
	thread pose_thread(localize_pose);
	thread path_thread(motion_plan);
	thread robot_thread(robot_calcmotion);
	thread display_thread(display_interface);

	// wait until program closes
	printf("[main] all threads started\n");
	while (!stopsig);
	alarm(1);

	// close all threads
	printf("[main] close all threads\n");
	rose.set_wheels(0, 0, 0, 0);
	rose.stop_arm();
	rose.startStop = true;
	rose.disconnect();
	chili_thread.join();
	chilicamthread.join();
	pose_thread.join();
	path_thread.join();
	robot_thread.join();
	display_thread.join();
	SDL_Quit();

	printf("Closed successfully.\n");
	return 0;
}

static bool arm_enabled = false;
static vec garm 	 ({ 0, -15, 90, 20, 0, 0 }); // degrees
static vec garm_init ({ 0, -15, 90, 20, 0, 0 });
static vec grab_pos	 ({ 0, 23, 114, -58, 0, 0 });
static vec rest_pos	 ({ 0, -21, 108, 90, 0, 70 });

void manual_input(void)
{
	struct timeval starttime;
	gettimeofday(&starttime, NULL);
	while (!stopsig)
	{
		SDL_PumpEvents();
		Uint8 *keystates = SDL_GetKeyState(NULL);

		// detect for exit
		if (keystates[SDLK_x])
		{
			auto_enable = false;
			auto_confirmed = false;
			manual_confirmed = true;
			rose.set_wheels(0, 0, 0, 0);
			rose.stop_arm();
			rose.startStop = true;
			kill(getpid(), SIGINT);
			continue;
		}

		// detect for autonomous enable or disable
		if (keystates[SDLK_m])
		{
			autonomous_lock.lock();
			auto_enable = false;
			auto_confirmed = false;
			manual_confirmed = true;
			autonomous_lock.unlock();
		}
		else if (keystates[SDLK_n])
		{
			autonomous_lock.lock();
			auto_enable = false;//true;
			auto_confirmed = false;
			manual_confirmed = true;
			autonomous_lock.unlock();
		}

		// input arm manual feedback
		struct timeval currtime;
		gettimeofday(&currtime, NULL);
		if (secdiff(starttime, currtime) > 0.05)
		{
			if (keystates[SDLK_p]) { garm(0) += 2; }
			if (keystates[SDLK_l]) { garm(0) -= 2; }
			if (keystates[SDLK_o]) { garm(1) += 2; }
			if (keystates[SDLK_k]) { garm(1) -= 2; }
			if (keystates[SDLK_i]) { garm(2) += 2; }
			if (keystates[SDLK_j]) { garm(2) -= 2; }
			if (keystates[SDLK_u]) { garm(3) += 2; }
			if (keystates[SDLK_h]) { garm(3) -= 2; }
			if (keystates[SDLK_y]) { garm(4) += 2; }
			if (keystates[SDLK_g]) { garm(4) -= 2; }
			if (keystates[SDLK_t]) { garm(5) += 10; }
			if (keystates[SDLK_f]) { garm(5) -= 10; }
			memcpy(&starttime, &currtime, sizeof(struct timeval));
		}

		// limit garm values
		for (int i = 0; i < garm.size(); i++)
		{
			garm(i) = limit_value(garm(i), rose.arm_mint(i), rose.arm_maxt(i));
		}

		if (keystates[SDLK_v])
		{
			arm_enabled = true;
			garm = garm_init;
		}

		if (keystates[SDLK_c])
		{
			garm = grab_pos;
		}

		if (keystates[SDLK_z])
		{
			garm = rest_pos;
		}

		if (keystates[SDLK_b])
		{
			arm_enabled = false;
		}

		if (arm_enabled)
		{
			rose.set_arm(garm(0), garm(1), garm(2), garm(3), garm(4), garm(5));
			cout << garm << endl;
		}
		else
		{
			rose.stop_arm();
		}

		// if not manual en, then continue
		if (auto_enable || !auto_confirmed)
		{
			continue;
		}

		// input manual feedback
		if 		(keystates[SDLK_q]) rose.set_wheels(-1,1,-1,1);
		else if (keystates[SDLK_e]) rose.set_wheels(1,-1,1,-1);
		else if (keystates[SDLK_a]) rose.set_wheels(-1,1,1,-1);
		else if (keystates[SDLK_d]) rose.set_wheels(1,-1,-1,1);
		else if (keystates[SDLK_s]) rose.set_wheels(-1,-1,-1,-1);
		else if (keystates[SDLK_w]) rose.set_wheels(1,1,1,1);
		else if (keystates[SDLK_1]) rose.set_wheels(1,0,0,0);
		else if (keystates[SDLK_2]) rose.set_wheels(0,1,0,0);
		else if (keystates[SDLK_3]) rose.set_wheels(0,0,1,0);
		else if (keystates[SDLK_4]) rose.set_wheels(0,0,0,1);
		else if (keystates[SDLK_r]) rose.reset_encoders();
		else rose.set_wheels(0,0,0,0);

		cout << rose.encoder[0] << " " << rose.encoder[1] << " " << rose.encoder[2] << " " << rose.encoder[3] << endl;

	}
}

void chilitag_detect(void)
{
	while (!stopsig)
	{
		// place the chilitags' positions into a matrix
		mat sv(3, 20, fill::zeros);
		for (int j = 0; j < 20; j++)
		{
			if (chili.tags[j][0] != 0.0)
			{
				sv.col(j) = vec({ chili.tags[j][1], chili.tags[j][2], chili.tags[j][0] });
			}
		}

		for (int j = 0; j < 20; j++)
		{
			vec pt({ sv(0, j), sv(1, j) });
			sv(0, j) = eucdist(pt);
			sv(1, j) = angle(pt);
		}

		// store the matrix
		chili_lock.lock();
		chilitags = sv;
		chili_lock.unlock();
	}
}

void localize_pose(void)
{
	// create the landmarks (custom)
	landmarks.push_back(sim_landmark(8, 240-24));				// 00
	landmarks.push_back(sim_landmark(8, 480-24));				// 01
	landmarks.push_back(sim_landmark(8, 720-24));				// 02
	landmarks.push_back(sim_landmark(8, 960-24));				// 03
	landmarks.push_back(sim_landmark(8, 1200-24));				// 04
	landmarks.push_back(sim_landmark(146, 1200-24));			// 05
	landmarks.push_back(sim_landmark(146, 960-24));				// 06
	landmarks.push_back(sim_landmark(146, 720-24));				// 07
	landmarks.push_back(sim_landmark(146, 480-24));				// 08
	landmarks.push_back(sim_landmark(146, 240-24));				// 09
	landmarks.push_back(sim_landmark(8, 240));					// 10
	landmarks.push_back(sim_landmark(8, 480));					// 11
	landmarks.push_back(sim_landmark(8, 720));					// 12
	landmarks.push_back(sim_landmark(8, 960));					// 13
	landmarks.push_back(sim_landmark(8, 1200));					// 14
	landmarks.push_back(sim_landmark(146, 1200));				// 15
	landmarks.push_back(sim_landmark(146, 960));				// 16
	landmarks.push_back(sim_landmark(146, 720));				// 17
	landmarks.push_back(sim_landmark(146, 480));				// 18
	landmarks.push_back(sim_landmark(146, 240));				// 19

	// start the particle filter
	pose_lock.lock();
	int nparticles = 500;
	double initial_sigma = 100;
	double x = robot_pose(0);
	double y = robot_pose(1);
	double t = robot_pose(2);
	double vs = 0.1;
	double ws = 0.2;
	pf = pfilter(nparticles, &globalmap, landmarks, x, y, t, initial_sigma);
	pf.set_noise(vs, ws);
	pose_lock.unlock();

	// loop on the particle filter for updates on the location
	vec mu;
	mat sigma;
	while (!stopsig)
	{
		// move the robot
		pf.move(rose.recv());

		// get the chilitags
		chili_lock.lock();
		mat landmarks = chilitags;
		chili_lock.unlock();

		// observe and predict the robot's new location
		pf.observe(landmarks);
		pf.predict(mu, sigma);

		// store the new location
		pose_lock.lock();
		robot_pose = mu;
		pose_lock.unlock();
	}
}

void robot_calcmotion(void)
{
	while (!stopsig)
	{
		autonomous_lock.lock();
		if (!auto_enable)
		{
			autonomous_lock.unlock();
			continue;
		}
		autonomous_lock.unlock();

		// get the current position of the robot
		pose_lock.lock();
		vec pose = robot_pose;
		pose_lock.unlock();
		vec pos = pose(span(0,1));
		double theta = pose(2);

		// get the current plan
		path_lock.lock();
		mat path_plan = pathplan;
		vec pose_plan = poseplan;
		bool do_pose = dopose;
		double twist = twistplan;
		double grab = grabplan;
		path_lock.unlock();
		int nwaypoints = (int)path_plan.n_cols;
		if (nwaypoints < 2)
		{
			// if there is no path, stop the robot for now
			rose.set_wheels(0, 0, 0, 0);
			continue;
		}
		else if (nwaypoints == 2)
		{
			printf("Reached goal, autonomous stopping\n");
			rose.set_wheels(0, 0, 0, 0);
			continue;
		}

		// do calculation of the wheels
		// first find the closest waypoint as the "start"
		vec distance(nwaypoints);
		mat diffs = path_plan - repmat(pos, 1, nwaypoints);
		uword target_index = 0;
		vec target = path_plan.col(0);
		for (int j = 0; j < nwaypoints; j++)
		{
			vec a = diffs.col(j);
			vec b = diffs.col(target_index);
			distance[j] = eucdist(a);
			if (distance[j] < eucdist(b))
			{
				target_index = j;
				target = path_plan.col(target_index);
			}
		}

		// acceptance radius && sector check
		if (distance[target_index] < 20 || (distance[target_index] < 40 && !within_value(angle(target - pos) - theta, -45, 45)))
		{
			if (target_index >= nwaypoints-1)
			{
				printf("target reached\n");
				rose.set_wheels(0, 0, 0, 0);
			}
			else
			{
				target_index++;
				target = path_plan.col(target_index);
			}
		}

		// once the target is found, then calculate the trajectory
		double theta_k_p = 0.025;
		double delta_theta = angle(target - pos) - theta;
		double v_l, v_r;
		v_l = (1.0 - theta_k_p * delta_theta);
		// - theta_k_i * r.integral_error
		// - theta_k_d * r.delta_theta_diff);
		v_r = (1.0 + theta_k_p * delta_theta);
		// + r.theta_k_i * r.integral_error
		// + r.theta_k_d * r.delta_theta_diff);

		cout << "current angle: " << theta << endl;
		cout << "delta theta: " << delta_theta << endl;
		cout << "left vel: " << v_l << endl << "right vel: " << v_r << endl;
		cout << "waypoint: " << endl << path_plan.col(path_plan.n_cols-1) << endl;
		cout << "current position: " << endl << pose << endl;
		cout << endl;

		// send the trajectory to the robot's wheels
		rose.set_wheels(v_l, v_r, v_l, v_r);

		// only if we want to do a pose do we activate the pose solver
		if (!do_pose)
		{
			rose.stop_arm();
			continue;
		}

		// get the list of all possible poses
		bool feasible_found = false;
		double baseangle = angle(poseplan.col(0).subvec(0,1));
		vec enc(6);
		for (int i = 0; i <= 90; i += 5)
		{ // 5 degree granularity
			mat R = rotationMat(0, 0, baseangle);
			// if a negative pose proves to find a solution, stop
			vec negpose({ 0, cos(deg2rad(-i)), sin(deg2rad(-i)) });
			negpose = R * negpose;
			if (rose.get_arm_position_placement(poseplan, negpose, twist, grab, enc))
			{
				feasible_found = true;
				break;
			}
			// if a positive pose proves to find a solution, stop
			vec pospose({ 0, cos(deg2rad(i)), sin(deg2rad(i)) });
			pospose = R * pospose;
			if (rose.get_arm_position_placement(poseplan, negpose, twist, grab, enc))
			{
				feasible_found = true;
				break;
			}
		}

		// send poses to the robot arm
		if (feasible_found)
		{
			rose.set_arm(enc(0), enc(1), enc(2), enc(3), enc(4), enc(5));
		}
		else
		{
			rose.stop_arm();
		}
	}

	// clean the motion
	rose.send(zeros<vec>(10));
}

void motion_plan(void)
{
	vec goal = vec({ 73, 1000 }); // this will have to change later somehow
	// grab the map
	mat localmap = globalmap.map;

	AStar astar(localmap, goal);
	while (!stopsig)
	{
		// try and see if we are allowed to go autonomous
		bool en = false;
		autonomous_lock.lock();
		en = auto_enable && manual_confirmed;
		auto_confirmed = true;
		autonomous_lock.unlock();

		// if we are indeed enabled, then we can compute the path to take
		if (!en)
		{
			continue;
		}

		// grab the current position
		pose_lock.lock();
		vec pose = robot_pose;
		pose_lock.unlock();

		// compute the new path
		vector<MotionAction> actionpath;
		vec curr = pose(span(0,1));
		astar.compute(curr, actionpath);
		if (astar.impossible())
		{
			// store an empty path
			path_lock.lock();
			pathplan = mat(2, 0);
			dopose = false; // shut off the pose just in case
			path_lock.unlock();
			continue;
		}

		// prune bad motion vectors
		vector<vec> prunedpath;
		vec origin;
		for (int i = 0; i < actionpath.size(); i++)
		{
			if (i == 0)
			{
				origin = vec({ actionpath[i].x, actionpath[i].y });
				prunedpath.push_back(origin);
			}
			else if (i == actionpath.size() - 1)
			{
				prunedpath.push_back(vec({ actionpath[i].x, actionpath[i].y }));
			}
			else
			{
				vec target({ actionpath[i].x, actionpath[i].y });
				if (eucdist(target - origin) >= 30)
				{
					prunedpath.push_back(target);
					origin = target;
				}
			}
		}

		// store the pruned path
		mat coalescedpath(2, prunedpath.size());
		for (int i = 0; i < prunedpath.size(); i++)
		{
			coalescedpath.col(i) = vec({ prunedpath[i](0), prunedpath[i](1) });
		}
		path_lock.lock();
		pathplan = coalescedpath;
		dopose = false;
		path_lock.unlock();
	}
}

void display_interface(void)
{
	cube frame(500, 500, 3, fill::zeros);

	while (!stopsig)
	{
		frame.zeros();

		// get the position of the robot
		pose_lock.lock();
		vec pose = robot_pose;
		pose_lock.unlock();

		// create a window around the pose
		int mux = (int)round(pose(0));
		int muy = (int)round(pose(1));
		double mut = pose(2);
		int sw2 = screen->w / 2;
		int sh2 = screen->h / 2;

		// draw the map
		globalmap.blit(frame, mux, muy);

		// draw the landmarks
		vec white({ 1, 1, 1 });
		for (int i = 0; i < landmarks.size(); i++)
		{
			sim_landmark &lm = landmarks[i];
			lm.blit(frame, mux, muy, chilitags.col(i));
		}

		// draw the particle filter
		pf.blit(frame, mux, muy);

		// draw the robot's position and pose
		int x, y;
		vec yellow({ 1, 1, 0 });
		draw_circle(frame, yellow, vec({ (double)sw2, (double)sh2 }), 20);
		for (int _i = -5; _i <= 5; _i++)
		{
			for (int _j = -5; _j <= 5; _j++)
			{
				x = sw2 + _j;
				y = sh2 + _i;
				if (x < 0 || x >= (int)frame.n_cols || y < 0 || y >= (int)frame.n_rows)
				{
					continue;
				}
				frame(y,x,0) = 1;
				frame(y,x,1) = 0;
				frame(y,x,2) = 0;
			}
		}
		x = (int)round(sw2 + (10 * cos(deg2rad(mut))));
		y = (int)round(sh2 + (10 * sin(deg2rad(mut))));
		vec color({ 0, 1, 1 });
		draw_line(frame, color, vec({(double)sw2,(double)sh2-1}), vec({(double)x,(double)y-1}));
		draw_line(frame, color, vec({(double)sw2,(double)sh2+1}), vec({(double)x,(double)y+1}));
		draw_line(frame, color, vec({(double)sw2-1,(double)sh2}), vec({(double)x-1,(double)y}));
		draw_line(frame, color, vec({(double)sw2+1,(double)sh2}), vec({(double)x+1,(double)y}));
		draw_line(frame, color, vec({(double)sw2,(double)sh2}), vec({(double)x,(double)y}));

		// draw A*
		path_lock.lock();
		mat path_plan = pathplan;
		path_lock.unlock();
		if (auto_enable)
		{
			vec purple({ 1, 0, 1 });
			for (int j = 0; j < (int)path_plan.n_cols; j++)
			{
				vec action = path_plan.col(j) + vec({ (double)(sw2 - mux), (double)(sh2 - muy) });
				draw_circle(frame, purple, vec({ action(0), action(1) }), 2.0);
			}
		}

		// push onto the screen
		screenblit(screen, frame);
		SDL_Flip(screen);
		SDL_Delay(25);
	}
}

static double secdiff(struct timeval &t1, struct timeval &t2)
{
	double usec = (double)(t2.tv_usec - t1.tv_usec) / 1000000.0;
	double sec = (double)(t2.tv_sec - t1.tv_sec);
	return sec + usec;
}