#ifndef maze_imgio_h
#define maze_imgio_h

#include <armadillo>
#include <string>

arma::imat load_maze(std::string &maze_name);
void save_image(std::string imgname, arma::icube &image);

#endif
