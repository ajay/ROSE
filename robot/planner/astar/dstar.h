#ifndef __TK_DSTAR_H__
#define __TK_DSTAR_H__

#include <vector>
#include <armadillo>
#include "actions.h"

class DStar {
  public:
    DStar(arma::imat map, arma::ivec &goal);
    ~DStar(void);
    void compute(arma::ivec &start, std::vector<MotionAction> &path);
    bool complete(void);
    bool impossible(void);

    arma::imat map;
    arma::ivec goal;

    // stuff for the decision making capability
	  bool isComplete;
    bool isImpossible;
    DStarProp prop;

};

#endif
