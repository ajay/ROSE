#ifndef waypoint_h
#define waypoint_h

#include <armadillo>

/** Waypoints are a position based constraint
 *  they often do not work with anything other
 *  than position completion
 */
class waypoint {
  public:
    waypoint(int x, int y);
    double x;
    double y;
    bool completed(arma::vec pos);
    // statically defined next events
    vector<waypoint *> next;
};

#endif
