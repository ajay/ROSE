#ifndef mathfun_h
#define mathfun_h

#include <armadillo>

double limit_value(double x, double a, double b);

double map_value(double x, double a1, double b1, double a2, double b2);

double wrap_value(double x, double a, double b);

int within_value(double x, double a, double b);

// Note: returns degrees
double cos_rule_angle(double A, double B, double C);

// Note: accepts degrees
arma::mat genRotateMat(double x, double y, double z);

#endif
