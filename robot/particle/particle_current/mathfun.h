#ifndef MATHFUN_H
#define MATHFUN_H

#include <armadillo>

double limit_value(double x, double a, double b);

double map_value(double x, double a1, double b1, double a2, double b2);

double wrap_value(double x, double a, double b);

int within_value(double x, double a, double b);

// conversion functions
double rad2deg(double x);
double deg2rad(double x);

// polar coordinate transforms
double eucdist(arma::vec v);
double angle(arma::vec v);

// Note: returns degrees
double cos_rule_angle(double A, double B, double C);

// Note: accepts degrees
arma::mat rotationMat(double x, double y, double z);

#endif