#include "draw.h"
#include "sim_window.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

using namespace arma;
using namespace std;

static int blocksize = 4;
static int linethickness = 1;

void setBlockSize(int bs) {
  blocksize = bs;
}

void setLineThickness(int lt) {
  linethickness = lt;
}

int getGridWidth(int n_cols) {
  int truewidth = (int)(n_cols + 3) * linethickness + (int)n_cols * blocksize;
  return truewidth;
}

int getGridHeight(int n_rows) {
  int trueheight = (int)(n_rows + 3) * linethickness + (int)n_rows * blocksize;
  return trueheight;
}

int gridx(int x) {
  return (x - linethickness) / (blocksize + linethickness);
}

int gridy(int y) {
  return (y - linethickness) / (blocksize + linethickness);
}

void blitRGB(SDL_Surface *screen, icube &image) {
  int w = getGridWidth(image.n_cols);
  int h = getGridHeight(image.n_rows);
  uint32_t *pixels = (uint32_t *)screen->pixels;
  uint32_t color_black = SDL_MapRGB(screen->format,0,0,0);
  SDL_FillRect(screen, NULL, color_black);
  for (int i = 0; i < (int)image.n_rows; i++) {
    for (int j = 0; j < (int)image.n_cols; j++) {
      // this might be a bit slow
      uint32_t color = SDL_MapRGB(screen->format,
          image(i,j,0),image(i,j,1),image(i,j,2));
      pixels[XY2P(j, i, screen->w, screen->h)] = color;
    }
  }
}

void drawGrid(icube &grid, imat &map) {
  int width = getGridWidth(map.n_cols);
  int height = getGridHeight(map.n_rows);
  if (grid.n_rows != height || grid.n_cols != width) {
    grid = icube(height, width, 3, fill::zeros);
  } else {
    grid.zeros();
  }
  // white fill
  grid.ones();
  grid *= 255;
  icube greensquare = zeros<icube>(blocksize, blocksize, 3);
  greensquare.slice(1) = ones<imat>(blocksize, blocksize) * 164;
  icube whitesquare = ones<icube>(blocksize, blocksize, 3) * 255;
  for (int i = 0; i < (int)map.n_rows; i++) {
    for (int j = 0; j < (int)map.n_cols; j++) {
      int startrow = linethickness * 2 + i * (blocksize + linethickness);
      int endrow = linethickness + (i + 1) * (blocksize + linethickness);
      int startcol = linethickness * 2 + j * (blocksize + linethickness);
      int endcol = linethickness + (j + 1) * (blocksize + linethickness);
      if (map(i, j)) {
        grid(span(startrow, endrow-1), span(startcol, endcol-1), span::all) = greensquare;
      } else {
        grid(span(startrow, endrow-1), span(startcol, endcol-1), span::all) = whitesquare;
      }
    }
  }
}

void drawPath(icube &grid, vector<ivec> &path) {
  // start drawing the path
  icube cyansquare = zeros<icube>(blocksize, blocksize, 3);
  cyansquare.slice(1) = ones<imat>(blocksize, blocksize) * 255;
  cyansquare.slice(2) = ones<imat>(blocksize, blocksize) * 255;
  for (ivec &node : path) {
    int x = node(0);
    int y = node(1);
    int startrow = linethickness * 2 + y * (blocksize + linethickness);
    int endrow = linethickness + (y + 1) * (blocksize + linethickness);
    int startcol = linethickness * 2 + x * (blocksize + linethickness);
    int endcol = linethickness + (x + 1) * (blocksize + linethickness);
    grid(span(startrow, endrow-1), span(startcol, endcol-1), span::all) = cyansquare;
  }
}

void drawBot(icube &grid, int x, int y) {
  icube redsquare = zeros<icube>(blocksize, blocksize, 3);
  redsquare.slice(0) = ones<imat>(blocksize, blocksize) * 255;
  int startrow = linethickness * 2 + y * (blocksize + linethickness);
  int endrow = linethickness + (y + 1) * (blocksize + linethickness);
  int startcol = linethickness * 2 + x * (blocksize + linethickness);
  int endcol = linethickness + (x + 1) * (blocksize + linethickness);
  grid(span(startrow, endrow-1), span(startcol, endcol-1), span::all) = redsquare;
}
