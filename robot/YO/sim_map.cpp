#include "sim_map.h"
#include "highgui.h"

using namespace arma;

sim_map::sim_map(void) {
  this->n_rows = 0;
  this->n_cols = 0;
}

sim_map::~sim_map(void) {
}

void sim_map::load(const std::string &map_name) {
  this->map = flipud(rgb2gray(load_image(map_name)));
  this->map = (this->map < 0.5) % ones<mat>(this->map.n_rows, this->map.n_cols);
  this->n_rows = this->map.n_rows;
  this->n_cols = this->map.n_cols;
}

void sim_map::blit(icube &screen) {
  assert(screen.n_rows == this->n_rows && screen.n_cols == this->n_cols && screen.n_slices == 3);
  for (int i = 0; i < (int)screen.n_rows; i++) {
    for (int j = 0; j < (int)screen.n_cols; j++) {
      screen(i, j, 0) = (int)(this->map(i, j) * 255);
      screen(i, j, 1) = (int)(this->map(i, j) * 255);
      screen(i, j, 2) = (int)(this->map(i, j) * 255);
    }
  }
}
