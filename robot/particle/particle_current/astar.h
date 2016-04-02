//
//	[Authors] = Ming Tai Ha
//              Jon Risinger
//              Timothy Yong
//

#ifndef __TK_ASTAR_H__
#define __TK_ASTAR_H__

#include <vector>
#include <armadillo>
#include "actions.h"

enum Ftype { F_FORWARD, F_BACKWARD };
enum Gtype { G_MIN, G_MAX };
enum Htype { H_MANHATTAN, H_EUCLIDEAN };

class AStarProp {
  public:
    AStarProp(enum Ftype f, enum Gtype g, enum Htype h, bool adaptive) :
      f(f), g(g), h(h), adaptive(adaptive) {}
    enum Ftype f;
    enum Gtype g;
    enum Htype h;
    bool adaptive;
};

class AStar {
  public:
    AStar(arma::imat map, arma::vec &goal,
      AStarProp prop = AStarProp(F_FORWARD, G_MAX, H_EUCLIDEAN, false));
    ~AStar(void);
    void compute(arma::vec &start, std::vector<MotionAction> &path);
    bool complete(void);
    bool impossible(void);

    arma::imat map;
    arma::vec goal;

    // stuff for the decision making capability
	  bool isComplete;
    bool isImpossible;
    AStarProp prop;

};

#endif
