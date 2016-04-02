#ifndef draw_h
#define draw_h

#include <armadillo>
#include <SDL2/SDL.h>
#include <vector>

void setBlockSize(int bs);
void setLineThickness(int lt);
int getGridWidth(int n_cols);
int getGridHeight(int n_rows);
int gridx(int x);
int gridy(int y);
void blitRGB(SDL_Surface *screen, arma::icube &image);
void drawGrid(arma::icube &grid, arma::imat &map);
void drawPath(arma::icube &grid, std::vector<arma::ivec> &path);
void drawBot(arma::icube &grid, int x, int y);

#endif
