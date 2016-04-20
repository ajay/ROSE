#ifndef pfilter_h
#define pfilter_h

#include <armadillo>
#include <vector>

#include "sim_landmark.h"
#include "sim_map.h"
#include "sim_robot.h"

class pfilter
{
	public:
		pfilter(void);
		pfilter(int nparticles, sim_map *map, std::vector<sim_landmark> &landmarks, double x, double y, double t, double initial_sigma);
		~pfilter(void);
		void move(arma::vec sensors);
		void observe(arma::mat observations);
		void predict(arma::vec &mu, arma::mat &sigma);
		void set_noise(double vs, double ws);
		void set_size(double r);
		void blit(arma::cube &screen, int mux, int muy);

		std::vector<sim_robot> particles;
		sim_map *map;

	private:
		void weigh(arma::mat &observations);
		void resample(void);

		double vs;
		double ws;
		arma::vec health;
		std::vector<sim_landmark> landmarks;
};
#endif