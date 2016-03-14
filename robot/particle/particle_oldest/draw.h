#ifndef __TK_DRAW_H__
#define __TK_DRAW_H__

#include <armadillo>

/** Draw a rectangle
 *  @param I the image to draw a rectangle in (gray)
 *  @param v the grayscale intensity
 *  @param topleft the top-left corner of the rectangle
 *  @param btmright the bottom-right corner of the rectangle
 */
void draw_rect(arma::mat &I, double v, arma::vec topleft, arma::vec btmright);

/** Draw a rectangle
 *  @param I the image to draw a rectangle in (rgb)
 *  @param v the rgb intensity vector
 *  @param topleft the top-left corner of the rectangle
 *  @param btmright the bottom-right corner of the rectangle
 */
void draw_rect(arma::cube &I, const arma::vec &v, arma::vec topleft, arma::vec btmright);

/** Draw a line
 *  @param I the image to draw a line in (gray)
 *  @param v the grayscale intensity
 *  @param pt1 the first point of the line
 *  @param pt2 the second point of the line
 */
void draw_line(arma::mat &I, double v, arma::vec pt1, arma::vec pt2);

/** Draw a line
 *  @param I the image to draw a line in (rgb)
 *  @param v the rgb intensity vector
 *  @param x1 the first coord x
 *  @param y1 the first coord y
 *  @param x2 the second coord x
 *  @param y2 the second coord y
 */
void draw_line(arma::cube &I, const arma::vec &v, arma::vec pt1, arma::vec pt2);

/** Draw a circle
 *  @param I the image to draw a circle in (gray)
 *  @param v the grayscale intensity
 *  @param x the center coord x
 *  @param y the center coord y
 *  @param radius the radius of the circle
 */
void draw_circle(arma::mat &I, double v, arma::vec pt, double radius);

/** Draw a circle
 *  @param I the image to draw a circle in (gray)
 *  @param v the rgb intensity
 *  @param x the center coord x
 *  @param y the center coord y
 *  @param radius the radius of the circle
 */
void draw_circle(arma::cube &I, const arma::vec &v, arma::vec pt, double radius);

#endif
