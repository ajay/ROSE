#include <cstdio>
#include <cassert>
#include "draw.h"

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))
#define LIMIT(x, a, b) (((x)<(a))?(a):(((b)<(x))?(b):(x)))
#define WITHIN(x, a, b) (((x)>=(a))&&((x)<=(b)))
#define SWAP(a, b) {(a)^=(b);(b)^=(a);(a)^=(b);}

using namespace arma;

void draw_rect(mat &I, double v, vec topleft, vec btmright) {
  int width = btmright(1) - topleft(1);
  int height = btmright(0) - topleft(0);
  assert(width >= 0 && height >= 0);
  int y1 = topleft(0);
  int y2 = btmright(0);
  if (y1 > y2) {
    SWAP(y1, y2);
  }
  int i1 = LIMIT(y1, 0, (int)I.n_rows-1);
  int i2 = LIMIT(y2, 0, (int)I.n_rows-1);
  int x1 = topleft(1);
  int x2 = btmright(1);
  if (x1 > x2) {
    SWAP(x1, x2);
  }
  int j1 = LIMIT(x1, 0, (int)I.n_cols-1);
  int j2 = LIMIT(x2, 0, (int)I.n_cols-1);
  if (WITHIN(x1, 0, (int)I.n_cols-1)) {
    for (int i = i1; i <= i2; i++) {
      I(i, x1) = v;
    }
  }
  if (WITHIN(x2, 0, (int)I.n_cols-1)) {
    for (int i = i1; i <= i2; i++) {
      I(i, x2) = v;
    }
  }
  if (WITHIN(y1, 0, (int)I.n_rows-1)) {
    for (int j = j1; j <= j2; j++) {
      I(y1, j) = v;
    }
  }
  if (WITHIN(y2, 0, (int)I.n_rows-1)) {
    for (int j = j1; j <= j2; j++) {
      I(y2, j) = v;
    }
  }
}

void draw_rect(cube &I, const vec &v, vec topleft, vec btmright) {
  for (uword k = 0; k < v.n_elem; k++) {
    draw_rect(I.slice(k), v(k), topleft, btmright);
  }
}

void draw_line(mat &I, double v, vec pt1, vec pt2) {
  int x1 = (int)round(pt1(1));
  int y1 = (int)round(pt1(0));
  int x2 = (int)round(pt2(1));
  int y2 = (int)round(pt2(0));
  if (x1 == x2 && y1 == y2) {
//    printf("same coord\n");
    return;
  }
  int dx = x2 - x1;
  int dy = y2 - y1;
  int d = MAX(abs(dx), abs(dy));
  vec xs = linspace<vec>(x1, x2, d);
  vec ys = linspace<vec>(y1, y2, d);
  for (int i = 0; i < d; i++) {
    if (xs(i) >= 0 && xs(i) <= (int)I.n_cols-1 &&
        ys(i) >= 0 && ys(i) <= (int)I.n_rows-1) {
      I(ys(i), xs(i)) = v;
    }
  }
}

void draw_line(cube &I, const vec &v, vec pt1, vec pt2) {
  for (uword k = 0; k < v.n_elem; k++) {
    draw_line(I.slice(k), v(k), pt1, pt2);
  }
}

// breaks at limiting cases
void draw_circle(mat &I, double v, vec pt, double radius) {
  int x = (int)round(pt(1));
  int y = (int)round(pt(0));
  for (double tr = 0; tr < 2 * M_PI * radius; tr += 1.0) {
    double theta = tr / radius;
    double X = (double)x + cos(theta) * radius;
    double Y = (double)y + sin(theta) * radius;
    if (X >= 0 && X < I.n_cols && Y >= 0 && Y < I.n_rows) {
      I((int)X, (int)Y) = v;
    }
  }
}

void draw_circle(cube &I, const vec &v, vec pt, double radius) {
  for (uword k = 0; k < v.n_elem; k++) {
    draw_circle(I.slice(k), v(k), pt, radius);
  }
}
