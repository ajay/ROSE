#ifndef SIM_MAP_H
#define SIM_MAP_H

#include <armadillo>
#include <string>

#include "sdldef.h"

class sim_map
{
	public:
		sim_map(void);
		~sim_map(void);
		void load(const std::string &map_name);
		void blit(arma::cube &screen, int x, int y);

		arma::mat map;
		arma::uword n_rows;
		arma::uword n_cols;
};

#endif