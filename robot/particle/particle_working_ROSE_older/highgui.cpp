#include <cmath>
#include <cstdint>
#include <opencv2/highgui/highgui.hpp>
#include "highgui.h"

static int limit(int x, int minv, int maxv);

arma::cube load_image(const std::string &image_name) {
  cv::Mat cv_image = cv::imread(image_name, CV_LOAD_IMAGE_COLOR);
  if (!cv_image.data) {
    printf("Warning: No image data!\n");
  }
  return cvt_opencv2arma(cv_image) / 255.0;
}

void save_image(const std::string &image_name, const arma::cube &image) {
  cv::Mat cv_image = cvt_arma2opencv(image * 255.0);
  cv::imwrite(image_name, cv_image);
}

arma::cube cvt_mat2cube(const arma::mat &m) {
  arma::cube c_image(m.n_rows, m.n_cols, 1);
  c_image.slice(0) = m;
  return c_image;
}

void save_image(const std::string &image_name, const arma::mat &image) {
  arma::cube c_image = cvt_mat2cube(image) * 255.0;
  cv::Mat cv_image = cvt_arma2opencv(c_image);
  cv::imwrite(image_name, cv_image);
}

arma::cube cvt_opencv2arma(const cv::Mat &cv_image) {
  arma::cube image(cv_image.rows, cv_image.cols, cv_image.channels());
  switch (image.n_slices) {
    case 1:
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          switch (cv_image.type()) {
            case CV_16S:
              image(i, j, 0) = (double)(cv_image.at<int16_t>(i, j));
              break;
            case CV_32F:
              image(i, j, 0) = (double)(cv_image.at<float>(i, j));
              break;
            case CV_64F:
              image(i, j, 0) = cv_image.at<double>(i, j);
              break;
            case CV_8UC1:
              image(i, j, 0) = (double)(cv_image.at<uint8_t>(i, j));
              break;
            default:
              fprintf(stderr, "ERROR: bad format!\n");
              return image;
          }
        }
      }
      break;
    case 3:
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          // set the red pixel
          image(i, j, 0) = (double)(cv_image.at<cv::Vec3b>(i, j)[2]);
          // set the green pixel
          image(i, j, 1) = (double)(cv_image.at<cv::Vec3b>(i, j)[1]);
          // set the blue pixel
          image(i, j, 2) = (double)(cv_image.at<cv::Vec3b>(i, j)[0]);
        }
      }
      break;
    case 4:
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          // set the red pixel
          image(i, j, 0) = (double)(cv_image.at<cv::Vec4b>(i, j)[2]);
          // set the green pixel
          image(i, j, 1) = (double)(cv_image.at<cv::Vec4b>(i, j)[1]);
          // set the blue pixel
          image(i, j, 2) = (double)(cv_image.at<cv::Vec4b>(i, j)[0]);
          // set the alpha pixel
          image(i, j, 3) = (double)(cv_image.at<cv::Vec4b>(i, j)[3]);
        }
      }
      break;
    default:
      image.zeros();
      break;
  }
  return image;
}

static int event_lbuttondown_sig;
static char *event_lbuttondown_name;
static int event_lbuttondown_posx;
static int event_lbuttondown_posy;
static int event_rbuttondown_sig;
static char *event_rbuttondown_name;
static int event_rbuttondown_posx;
static int event_rbuttondown_posy;
static char *event_mousemove_name;
static int event_mousemove_posx;
static int event_mousemove_posy;

static void MouseEventCallback(int event, int x, int y, int flags, void *userdata) {
  switch (event) {
    case cv::EVENT_LBUTTONDOWN:
      event_lbuttondown_sig = 1;
      event_lbuttondown_name = (char *)userdata;
      event_lbuttondown_posx = x;
      event_lbuttondown_posy = y;
      break;
    case cv::EVENT_RBUTTONDOWN:
      event_rbuttondown_sig = 1;
      event_rbuttondown_name = (char *)userdata;
      event_rbuttondown_posx = x;
      event_rbuttondown_posy = y;
      break;
    case cv::EVENT_MOUSEMOVE:
      event_mousemove_name = (char *)userdata;
      event_mousemove_posx = x;
      event_mousemove_posx = y;
      break;
  }
}

void disp_image(const std::string &window_name, const arma::mat &image, bool mouseevent) {
  cv::namedWindow(window_name);
  if (mouseevent) {
    char *name = new char[window_name.length() + 1];
    strcpy(name, window_name.c_str());
    cv::setMouseCallback(window_name, MouseEventCallback, name);
  }
  arma::cube new_image = gray2rgb(image);
  cv::imshow(window_name, cvt_arma2opencv(new_image * 255.0));
}

void disp_image(const std::string &window_name, const arma::cube &image, bool mouseevent) {
  cv::namedWindow(window_name);
  if (mouseevent) {
    char *name = new char[window_name.length() + 1];
    strcpy(name, window_name.c_str());
    cv::setMouseCallback(window_name, MouseEventCallback, name);
  }
  cv::imshow(window_name, cvt_arma2opencv(image * 255.0));
}

std::vector<int> disp_get_lclick_pos(const std::string &window_name) {
  if (event_lbuttondown_name && window_name == std::string(event_lbuttondown_name)) {
    return std::vector<int>({ event_lbuttondown_posx, event_lbuttondown_posy });
  } else {
    return std::vector<int>({ 0, 0 });
  }
}

bool disp_get_lclicked(const std::string &window_name) {
  if (event_lbuttondown_name && window_name == std::string(event_lbuttondown_name) && event_lbuttondown_sig) {
    event_lbuttondown_sig = 0;
    return true;
  } else {
    return false;
  }
}

std::vector<int> disp_get_rclick_pos(const std::string &window_name) {
  if (event_rbuttondown_name && window_name == std::string(event_rbuttondown_name)) {
    return std::vector<int>({ event_rbuttondown_posx, event_rbuttondown_posy });
  } else {
    return std::vector<int>({ 0, 0 });
  }
}

bool disp_get_rclicked(const std::string &window_name) {
  if (event_rbuttondown_name && window_name == std::string(event_rbuttondown_name) && event_rbuttondown_sig) {
    event_rbuttondown_sig = 0;
    return true;
  } else {
    return false;
  }
}

std::vector<int> disp_get_mouse_pos(const std::string &window_name) {
  if (event_mousemove_name && window_name == std::string(event_mousemove_name)) {
    return std::vector<int>({ event_mousemove_posx, event_mousemove_posy });
  } else {
    return std::vector<int>({ 0, 0 });
  }
}

void disp_wait(void) {
  cv::waitKey(0);
}

int disp_keyPressed(void) {
  return cv::waitKey(30);
}

void disp_close(const std::string &window_name) {
  cv::destroyWindow(window_name);
}

// ARMADILLO

cv::Mat cvt_arma2opencv(const arma::cube &image) {
  cv::Mat cv_image;
  switch (image.n_slices) {
    case 1:
      cv_image = cv::Mat(image.n_rows, image.n_cols, CV_8UC1);
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          uint8_t gray = (uint8_t)limit((int)round(image(i, j, 0)), 0, 255);
          cv_image.at<uint8_t>(i, j) = gray;
        }
      }
      break;
    case 3:
      cv_image = cv::Mat(image.n_rows, image.n_cols, CV_8UC3);
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          uint8_t red   = (uint8_t)limit((int)round(image(i, j, 0)), 0, 255);
          uint8_t green = (uint8_t)limit((int)round(image(i, j, 1)), 0, 255);
          uint8_t blue  = (uint8_t)limit((int)round(image(i, j, 2)), 0, 255);
          cv_image.at<cv::Vec3b>(i, j) = cv::Vec3b(blue, green, red);
        }
      }
      break;
    case 4:
      cv_image = cv::Mat(image.n_rows, image.n_cols, CV_8UC4);
      for (arma::uword i = 0; i < image.n_rows; i++) {
        for (arma::uword j = 0; j < image.n_cols; j++) {
          uint8_t red   = (uint8_t)limit((int)round(image(i, j, 0)), 0, 255);
          uint8_t green = (uint8_t)limit((int)round(image(i, j, 1)), 0, 255);
          uint8_t blue  = (uint8_t)limit((int)round(image(i, j, 2)), 0, 255);
          uint8_t alpha = (uint8_t)limit((int)round(image(i, j, 3)), 0, 255);
          cv_image.at<cv::Vec4b>(i, j) = cv::Vec4b(blue, green, red, alpha);
        }
      }
      break;
    default:
      cv_image = cv::Mat::zeros(image.n_rows, image.n_cols, CV_8UC1);
      break;
  }
  return cv_image;
}

cv::Mat arma2opencv(const arma::mat &mtx, int cvtype) {
  cv::Mat cv_mtx;
  switch (cvtype) {
    case CV_32F:
    case CV_64F:
      cv_mtx = cv::Mat::zeros(mtx.n_rows, mtx.n_cols, cvtype);
      break;
    case CV_32FC3:
      cv_mtx = cv::Mat::zeros(mtx.n_rows, 1, cvtype);
  }
  for (int i = 0; i < (int)mtx.n_rows; i++) {
    for (int j = 0; j < (int)mtx.n_cols; j++) {
      switch (cvtype) {
        case CV_32F:
          cv_mtx.at<float>(i, j) = (float)mtx(i, j);
          break;
        case CV_64F:
          cv_mtx.at<double>(i, j) = mtx(i, j);
          break;
        case CV_32FC3:
          cv_mtx.at<cv::Vec3f>(i, 0)[j] = mtx(i, j);
        default:
          printf("Error: cannot do type %d\n", cvtype);
          break;
      }
    }
  }
  return cv_mtx;
}

arma::mat opencv2arma(const cv::Mat &cv_mtx) {
  arma::mat mtx;
  switch (cv_mtx.type()) {
    case CV_32F:
    case CV_64F:
      mtx = arma::mat(cv_mtx.rows, cv_mtx.cols, arma::fill::zeros);
      break;
    case CV_32FC3:
      mtx = arma::mat(cv_mtx.rows, 3, arma::fill::zeros);
      break;
  }
  for (int i = 0; i < (int)mtx.n_rows; i++) {
    for (int j = 0; j < (int)mtx.n_cols; j++) {
      switch (cv_mtx.type()) {
        case CV_32F:
          mtx(i, j) = cv_mtx.at<float>(i, j);
          break;
        case CV_64F:
          mtx(i, j) = cv_mtx.at<double>(i, j);
          break;
        case CV_32FC3:
          mtx(i, j) = cv_mtx.at<cv::Vec3f>(i, 0)[j];
          break;
        default:
          printf("Error: cannot do type %d\n", cv_mtx.type());
          break;
      }
    }
  }
  return mtx;
}

static int limit(int x, int minv, int maxv) {
  return (x < minv) ? minv : ((x > maxv) ? maxv : x);
}

arma::mat rgb2gray(const arma::cube &image) {
  arma::vec scale = { 0.3, 0.6, 0.1 };
  arma::mat new_image = arma::zeros<arma::mat>(image.n_rows, image.n_cols);
  for (arma::uword i = 0; i < image.n_slices; i++) {
    new_image += scale(i) * image.slice(i); // weighted construction
  }
  return new_image;
}

arma::cube gray2rgb(const arma::mat &image) {
  arma::cube new_image(image.n_rows, image.n_cols, 3);
  for (arma::uword i = 0; i < new_image.n_slices; i++) {
    new_image.slice(i) = image;
  }
  return new_image;
}
