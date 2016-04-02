#include <iostream>
#include <string>
#include <cstdio>
#include <signal.h>
#include <armadillo>
#include "astar.h"
#include "sim_window.h"
#include "draw.h"
#include "maze_imgio.h"

#define t_delay 25
using namespace std;
using namespace arma;

void stopprog(int signo) {
  printf("Time ran out, quitting.\n");
  exit(1);
}

SDL_Surface *screen;
static bool mouseon;

/// OPTIONS ///
static int RANDOM = 1;
static int F = F_FORWARD;
static int H = H_MANHATTAN;
static int G = G_MAX;
static string fname;

ivec getClickedPoint(void) {
  SDL_Event *e;
  for (;;) {
    while ((e = sim_window::get_event())) {
      if (e->type == SDL_QUIT) {
        sim_window::destroy();
        exit(1);
      }
      if (e->type == SDL_MOUSEBUTTONDOWN) {
        if (mouseon == false) {
          mouseon = true;
        }
      }
      if (e->type == SDL_MOUSEBUTTONUP) {
        if (mouseon == true) {
          mouseon = false;
          return ivec({ gridx(e->button.x), gridy(screen->h - e->button.y - 1) });
        }
      }
    }
  }
  return ivec({0, 0});
}

void run_map(imat map) {
  // select two points, for now the corners
  icube pathmap;
  drawGrid(pathmap, map);
  blitRGB(screen, pathmap);
  sim_window::update();
  SDL_Delay(t_delay * 5);

  // grab the start and goal positions
  bool isSame = 1;
  ivec start;
  ivec goal;
  while(isSame) {
    printf("Waiting for point 1\n");
    start = getClickedPoint();

    printf("Waiting for point 2\n");
    goal = getClickedPoint();

    if (start(0) == goal(0) && start(1) == goal(1)) {
      printf("You clicked on the same point twice. Please pick two different points.\n");
    }
    else {
      isSame = 0;
    }
  }

  // compute the path
  AStar astar(map, goal);
  vector<MotionAction> path;
  astar.compute(start, path);
  if (astar.impossible()) {
    printf("It is impossible!\n");
    return;
  }
  vector<ivec> pathpos;
  for (MotionAction &action : path) {
    cout << action << endl;
    pathpos.push_back(ivec({ (int)action.x, (int)action.y }));
  }
  
  // compute the interpolation
  // lookahead requires bounds
  /*vector<ivec> traj;
  for (int i = 2; i < path.size() - 2; i++) {
    ivec p[5];
    p[0] = v[i-2];
    p[1] = v[i-1];
    p[2] = v[i];
    p[3] = v[i+1];
    p[4] = v[i+2];
    traj.push_back((p[1](1)-p[0](1)/p[1](0)-p[0](0))+
    (p[2](1)-p[0](1)/p[2](0)-p[0](0))+
    (p[3](1)-p[0](1)/p[3](0)-p[0](0))+
    (p[4](1)-p[0](1)/p[4](0)-p[0](0)))/4;
  }*/

  // draw it out
  drawPath(pathmap, pathpos);
  drawBot(pathmap, start(0), start(1));
  blitRGB(screen, pathmap);
  sim_window::update();
}

int main(int argc, char *argv[]) {
  //signal(SIGALRM, stopprog);
  //alarm(120);
  srand(getpid());
  // TODO: create argument labels for generating maps and using static maps, as well as options for the simulation
  imat maze;
  int block_prob;

  if (argc < 3) {
    printf("usage: %s [random=<0..100>|file=<filename>] [forward_max|forward_min|backward|euclidean]\n", argv[0]);
    return 1;
  } else {
    string arg1 = argv[1];
    string arg2 = argv[2];
    size_t pos;
    if ((pos = arg1.find("=")) == string::npos) {
      printf("error: format is [random=<block_prob>|file=<filename>]\n");
      return 1;
    }
    if (arg1.substr(0, pos).compare("file") == 0) {
      RANDOM = 0;
      fname = arg1.substr(pos+1, arg1.size()-pos-1);
      maze = load_maze(fname);
    } else if (arg1.substr(0, pos).compare("random") == 0) {
      /*RANDOM = 1;
      block_prob = atoi(arg1.substr(pos+1, arg1.size()-pos-1).c_str());
      if (block_prob < 0 || block_prob > 100) {
        printf("error: probability must be between 0 to 100\n");
        return 1;
      }
      maze = maze_gen(MAZESIZE, block_prob);*/
      return 1;
    } else {
      printf("error: format is [random=<block_prob>|file=<filename>]\n");
      return 1;
    }

    if (arg2.compare("forward_max") == 0) {
      H = H_MANHATTAN;
      F = F_FORWARD;
      G = G_MAX;
    } else if (arg2.compare("forward_min") == 0) {
      H = H_MANHATTAN;
      F = F_FORWARD;
      G = G_MIN;
    } else if (arg2.compare("backward") == 0) {
      H = H_MANHATTAN;
      F = F_BACKWARD;
      G = G_MAX;
    } else if (arg2.compare("euclidean") == 0) {
      H = H_EUCLIDEAN;
      F = F_FORWARD;
      G = G_MAX;
    } else {
      printf("error: format is [forward_max|forward_min|backward|adaptive]\n");
      return 1;
    }
  }
  // create maps
  setBlockSize(2);
  setLineThickness(0);
  screen = sim_window::init(getGridWidth(maze.n_cols), getGridHeight(maze.n_rows));
  run_map(maze);
  sleep(3);
  sim_window::destroy();

  return 0;
}
