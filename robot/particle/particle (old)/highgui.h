#ifndef __TK_HIGHGUI_H__
#define __TK_HIGHGUI_H__

#include <string>
#include <opencv2/core/core.hpp>

#ifndef __NVCC__

#include <armadillo>

/** Load an image in arma::cube format
 *  @param image_name the name of the image
 *  @return the cube which holds the image data
 */
arma::cube load_image(const std::string &image_name);

/** Save an image to an image name
 *  @param image_name the image name to be saved under
 *  @param image the image to be saved
 */
void save_image(const std::string &image_name, const arma::mat &image);
void save_image(const std::string &image_name, const arma::cube &image);

/** Convert a matrix to a cube
 *  @param m the matrix
 *  @return the cube
 */
arma::cube cvt_mat2cube(const arma::mat &m);

/** Display an image in an OpenCV window
 *  @param window_name the name of the window to display the image
 *  @param image the image to be displayed
 *  @param mouseevent (optional) enable mouse events
 */
void disp_image(const std::string &window_name, const arma::mat &image, bool mouseevent = false);
void disp_image(const std::string &window_name, const arma::cube &image, bool mouseevent = false);

/** Grab the position of a left mouse click
 *  @param window_name the name of the window to get a click from
 *  @return the vector of positions where the mouse was clicked
 */
std::vector<int> disp_get_lclick_pos(const std::string &window_name);

/** See whether or not the mouse was clicked
 *  @param window_name the name of the window to get a click from
 *  @return true if clicked, false otherwise
 */
bool disp_get_lclicked(const std::string &window_name);

/** Grab the position of the right mouse click
 *  @param window_name the name of the window to get a click from
 *  @return the vector of positions where the mouse was clicked
 */
std::vector<int> disp_get_rclick_pos(const std::string &window_name);

/** See whether or not the mouse was clicked
 *  @param window_name the name of the window to get a click from
 *  @return true if clicked, false otherwise
 */
bool disp_get_rclicked(const std::string &window_name);

/** Get the current mouse position in the window
 *  @param window_name the name of the window to grab the mouse position from
 *  @return the vector of positions where the mouse is currently at
 */
std::vector<int> disp_get_mouse_pos(const std::string &window_name);

/** Wait for a key to be pressed before closing the window
 */
void disp_wait(void);

/** Wait for a key to be pressed for 30 fps
 *  @return status of a key pressed
 */
int disp_keyPressed(void);

/** Close a window of an image
 *  @param window_name the name of the window to close
 */
void disp_close(const std::string &window_name);

/** Convert the OpenCV image to an arma::cube
 *  @param cv_image the OpenCV image
 *  @return the arma::cube image
 */
arma::cube cvt_opencv2arma(const cv::Mat &cv_image);

/** Convert the arma::cube image to an OpenCV image
 *  @param image the arma::cube image
 *  @return the OpenCV image
 */
cv::Mat cvt_arma2opencv(const arma::cube &image);

/** Conversion functions (experimental) for arma to opencv
 *  @param mtx the matrix to take in
 *  @param cvtype the image type
 *  @return the opencv matrix
 */
cv::Mat arma2opencv(const arma::mat &mtx, int cvtype = CV_32F);

/** Conversion functions (experimental) for opencv to arma
 *  @param cv_mtx the matrix to take in
 *  @return the armadillo matrix
 */
arma::mat opencv2arma(const cv::Mat &cv_mtx);

/** Convert rgb to grayscale
 *  @param image the rgb image
 *  @return the grayscale image
 */
arma::mat rgb2gray(const arma::cube &image);

/** Convert grayscale to rgb
 *  @param the grayscale image
 *  @return the rgb image
 */
arma::cube gray2rgb(const arma::mat &image);

#else

#include "gcube.h"

/** Load an image in gcube format
 *  @param image_name the name of the image to load
 *  @return the image
 */
gcube load_gcube(const std::string &image_name);

/** Save an image in gcube format
 *  @param image_name the name of the image to save
 *  @param image the image to save
 */
void save_gcube(const std::string &image_name, gcube &image);

/** Print out a gcube to the console
 *  @param image the image to be printed
 */
void print_gcube(gcube &image);

/** Display a gcube onto the screen
 *  @param window_name the name of the window
 *  @param image the gcube to display
 */
void disp_gcube(const std::string &window_name, gcube &image);

/** Wait until the user presses a key to close the window
 */
void disp_wait(void);

/** Wait for a key to be pressed for 30 fps
 *  @return status of a key pressed
 */
int disp_keyPressed(void);

/** Transform an image from RGB to Grayscale
 *  @param image the image to transform into grayscale
 *  @return the transformed image
 */
gcube gpu_rgb2gray(const gcube &image);

/** Transform an image from Grayscale to RGB
 *  @param image the image to transform into RGB
 *  @return the transformed image
 */
gcube gpu_gray2rgb(const gcube &image);

#endif

#endif
