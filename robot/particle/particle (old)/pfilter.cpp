#include "pfilter.h"
#include "sim_robot.h"
#include <cmath>
#include <random>

using namespace arma;
using namespace std;

pfilter::pfilter(void) {
  // DOES NOTHING
}

/** This is the constructor for the particle filter (think of it as your init)
 *  @param nparticles the number of particles to create
 *  @param map the pointer to the map (just store it)
 *  @param landmarks a list of landmarks, where each landmark is described in sim_landmark.h
 */
pfilter::pfilter(int nparticles, sim_map *map, vector<sim_landmark> &landmarks, int x, int y, int t) {
  // STEP 1: store the map and landmark variables
  this->map = map;
  this->landmarks = landmarks;

  // STEP 2: create a bunch of particles, place them into this->particles
  default_random_engine generator;
  uniform_int_distribution<> xd(0, map->n_cols);
  uniform_int_distribution<> yd(0, map->n_rows);
  uniform_real_distribution<> td(0, 2 * M_PI);
  //normal_distribution<double> xd(x, 50.0);
  //normal_distribution<double> yd(y, 50.0);
  //normal_distribution<double> td(t, 10.0);

  vector<sim_robot> new_particles;
  for (int i = 0; i < nparticles; i++) {
    //x = round(xd(generator));
    //y = round(yd(generator));
    //t = ((int)round(td(generator)) % 360) * M_PI / 180;
    x = xd(generator);
    y = yd(generator);
    t = td(generator);
    sim_robot newbot; 
    newbot.set_pose(x,y,t);
    new_particles.push_back(newbot);
  }
  particles.clear();
  particles = new_particles;

  // STEP 3: initialize the health as a vec of ones
  health = ones<vec>(nparticles);
}

pfilter::~pfilter(void) {
}

/** Set the noise for each robot
 *  @param vs the velocity sigma2
 *  @param ws the angular velocity sigma2
 */
void pfilter::set_noise(double vs, double ws) {
  // Set the noise for each robot
  for (sim_robot &bot : this->particles) {
    bot.set_noise(vs, ws);
  }
  this->vs = vs;
  this->ws = ws;
}

/** Set the size for each robot
 *  @param r the radius of the robot
 */
void pfilter::set_size(double r) {
  for (sim_robot &bot : this->particles) {
    bot.set_size(r);
  }
}

/** Move each robot by some velocity and angular velocity
 *  In here, also detect if the robot's position goes out of range,
 *  and handle it
 *  Pretend that the world is circular
 *  @param v the velocity
 *  @param w the angular velocity
 */
void pfilter::move(double vx, double vy, double w) {
  for (int i = 0; i < particles.size(); i++){
    particles[i].move(vx,vy,w);
    // circular world!!!!
    sim_robot &bot = particles[i];
    if (bot.x < 0) {
      bot.x = 0;
    }
    if (bot.x >= (double)this->map->n_cols) {
      bot.x = (double)this->map->n_cols-1;
    }
    while (bot.y < 0) {
      bot.y = 0;
    }
    while (bot.y >= (double)this->map->n_rows) {
      bot.y = (double)this->map->n_rows-1;
    }
  }
}

double gauss(double mu, double sigma2) {
  return 1 / sqrt(2 * M_PI * sigma2) * exp(-0.5 * (mu * mu) / sigma2);
}

/** Weigh the "health" of each particle using gaussian error
 *  @param observations a 3xn matrix, where the first row is the x row,
 *                      and the second row is the y row
 *  @param health the health vector
 */
void pfilter::weigh(mat &observations) {
  vec theta = vec(observations.size());
  vec radius(observations.size());
  vec R(observations.n_cols);
  vec T(observations.n_cols);
  cout << "[pfilter.cpp] obs recvd:\n" << observations << endl;
  for (int i = 0; i < (int)landmarks.size(); i++) {
    vec sensor(2);
    sensor(0) = observations(0, i);
    sensor(1) = observations(1, i);
    R[i] = sensor(1);//sqrt(dot(sensor, sensor));
    T[i] = atan2(sensor(1), sensor(0));
  }
  for (int i = 0; i < particles.size(); i++){
    for(int j = 0; j < landmarks.size(); j++){
      double dx = landmarks[j].x - particles[i].x;
      double dy = landmarks[j].y - particles[i].y;
      radius[j] = sqrt(pow(dx, 2) + pow(dy, 2)); // radius of the robot
      theta[j] = atan2(dy, dx) - particles[i].t; // theta of the robot
      this->health[i] *= gauss(R[j]-radius[j],vs);
      //this->health[i] *= gauss(T[j]-theta[j],ws);
    }
    int x_ = (int)round(particles[i].x);
    int y_ = (int)round(particles[i].y);
    if (x_ < 0 || x_ >= (int)this->map->n_cols ||
        y_ < 0 || y_ >= (int)this->map->n_rows ||
        this->map->map(y_,x_) > 0.5) {
      this->health[i] = 0;
    }
  }
}

/** Resample all the particles based on the health using the resample wheel
 *  @param health the health of all the particles
 */
void pfilter::resample(void) {
  int N = particles.size();
  vector<sim_robot> p2;
  int index = (int)rand() % N;
  double beta=0;
  double mw = (max(this->health));
  for (int i = 0; i < N; i++){
    beta += ((double)rand() / (double)RAND_MAX) * 2 * mw;
    while (beta >= this->health[index]){
      if (beta == 0) {
        break;
      }
      beta -= this->health[index];
      index=(index+1)%N;
    }
    p2.push_back(particles[index]);
  }
  particles.clear();
  particles = p2;
}

/** Call the weigh and resample functions from here
 *  @param observations the observations of the landmarks
 */
void pfilter::observe(mat observations) {
  printf("[pfilter.cpp] received observation!\n");
  // each column of obs matches to each col of landmarks
  health=ones<vec>(particles.size());
  weigh(observations);
  resample();
}

/** Predict the position and calculate the error of the particle set
 *  @param mu (output) the position ( x, y, theta )
 *  @param sigma (output) the error
 */
void pfilter::predict(vec &mu, mat &sigma) {
  mu = zeros<vec>(3);
  for (int i = 0; i < this->particles.size(); i++) {
    mu += vec({ this->particles[i].x, this->particles[i].y, this->particles[i].t });
  }
  mu /= this->particles.size();
  sigma = zeros<mat>(3, 3);
  // var = (Particle's x_i-Mean)^2 / N 
  for (int i = 0; i < this->particles.size(); i++) {
    vec pvec({ this->particles[i].x, this->particles[i].y, this->particles[i].t });
    sigma += pvec * pvec.t();
  }
  sigma /= this->particles.size();
}

void pfilter::blit(icube &screen) {
  // DO NOT TOUCH THIS FUNCTION
  vec health;
  if (this->health.n_elem != 0) {
    this->health / max(this->health);
  } else {
    this->health = 0.00001;
  }
  int i = 0;
  for (sim_robot &bot : this->particles) {
    int x = (int)round(bot.x);
    int y = (int)round(bot.y);
    if (x >= 0 && x < (int)screen.n_cols &&
        y >= 0 && y < (int)screen.n_rows) {
      screen(y, x, 0) = 255;
      screen(y, x, 1) = 255;
      screen(y, x, 2) = 0;
    }
    i++;
  }
}
