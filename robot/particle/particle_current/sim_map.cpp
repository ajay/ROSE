#include "highgui.h"
#include "sim_map.h"

using namespace arma;

sim_map::sim_map(void)
{
	this->n_rows = 0;
	this->n_cols = 0;
}

sim_map::~sim_map(void)
{
}

void sim_map::load(const std::string &map_name)
{
	this->map = flipud(rgb2gray(load_image(map_name)));
	this->map = (this->map < 0.5) % ones<mat>(this->map.n_rows, this->map.n_cols);
	this->n_rows = this->map.n_rows;
	this->n_cols = this->map.n_cols;
}

void sim_map::blit(cube &screen, int x, int y)
{
	for (int i = 0; i < (int)screen.n_rows; i++)
	{
		for (int j = 0; j < (int)screen.n_cols; j++)
		{
			int x_ = x - (int)screen.n_cols/2 + j;
			int y_ = y - (int)screen.n_rows/2 + i;
			if (x_ < 0 || x_ >= (int)this->map.n_cols || y_ < 0 || y_ >= (int)this->map.n_rows)
			{
				continue;
			}
			screen(i, j, 0) = -(this->map(y_, x_) * 0.25) + 0.5;
			screen(i, j, 1) = -(this->map(y_, x_) * 0.25) + 0.5;
			screen(i, j, 2) = -(this->map(y_, x_) * 0.25) + 0.5;
		}
	}
}