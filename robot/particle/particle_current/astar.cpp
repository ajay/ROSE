//
//	[Authors] = Ming Tai Ha
//              Jon Risinger
//              Timothy Yong
//

#include "astar.h"
#include "heap.h"
#include "heap.cpp"
#include <iostream>
#include <vector>
#include <cassert>

using namespace arma;
using namespace std;

static MotionAction getPreviousAction(MotionAction currAction, imat &backtrace);
static vector<MotionAction> getNextAction(MotionAction currAction, imat &map);

/** The goal of this function is to initialize the AStar algorithm,
 *  including any data structures which you are to use in the
 *  computation of the next state
 *  @param map This is the map which you are given
 *  @param goal This is the goal of the robot
 */
AStar::AStar(imat map, vec &goal, AStarProp prop) :
    isComplete(false),
    isImpossible(false),
    goal(goal),
    prop(prop) {
  this->map = map.t();
  assert(0 <= goal(0) && goal(0) < (int)this->map.n_rows &&
         0 <= goal(1) && goal(1) < (int)this->map.n_cols);
}

AStar::~AStar(void) {
}

/** In this function, you are to get the next state off the
 *  priority queue and then traverse to that state,
 *  computing the cost and placing the state that was traversed
 *  into the search space
 */
void AStar::compute(vec &start, vector<MotionAction> &path) {
  this->isComplete = false;
  this->isImpossible = false;

  MotionAction start_state(this->goal(0), this->goal(1)); // changed to backward
  Heap<MotionAction> opened;
  opened.push(start_state, sum(abs(start - goal)));

  // create a matrix of parents that have been closed
  imat closed(this->map.n_rows, this->map.n_cols, fill::zeros);
  imat backtrace(this->map.n_rows, this->map.n_cols, fill::zeros);

  // after pushing the initial state, start trying to get the next state
  while (!opened.empty()) {
    // grab a state
    MotionAction curr = opened.pop();
    double x = curr.x;
    double y = curr.y;

    closed(x, y) = true;
    // if this state is the goal state, then return the path
    if (x == start(0) && y == start(1)) { // changed to backward
      path.clear();
      while (x != this->goal(0) || y != this->goal(1)) { // changed to backward
        path.push_back(curr);
        curr = getPreviousAction(curr, backtrace);
        x = curr.x;
        y = curr.y;
      }
      path.push_back(curr);
      //reverse(path.begin(), path.end());
      this->isComplete = true;
      return;
    }
    // otherwise try to find new neighbors and add them in
    vector<MotionAction> next_actions = getNextAction(curr, this->map);
    for (MotionAction &action : next_actions) {
      x = action.x;
      y = action.y;
      if (!backtrace(x, y) && !closed(x, y)) {
        backtrace(x, y) = action.id;
        assert(action.id != 0); // just in case
        // euclidean distance
        double hcost = action.gcost + sqrt(dot(action.pos - goal, action.pos - goal));
        opened.push(action, hcost);
      }
    }
  }
  this->isImpossible = true;
}

/** Return whether or not the goal is impossible to reach
 *  @return true if it is impossible, false otherwise
 */
bool AStar::impossible(void) {
  return this->isImpossible;
}

/** Return whether or not the goal has been reached
 *  @return true if goal is reached, false otherwise
 */
bool AStar::complete(void) {
  return this->isComplete;
}

/** Return the parent action of the current action
 *  @param currAction the current action of the robot
 *  @param backtrace a matrix of all backtraced actions
 *  @return the parent action
 */
static MotionAction getPreviousAction(MotionAction currAction, imat &backtrace) {
  mat neighbor4 = reshape(mat({
      0, 0, -1, 1,
      1, -1, 0, 0
    }), 4, 2).t();
  if (currAction.id == STARTING_ACTION) {
    return currAction;
  } else {
    vec neighbor = neighbor4.col(currAction.id - MOVE_FORWARD);
    vec pos = currAction.pos - neighbor;
    MotionAction parent(pos(0), pos(1), (enum ActionId)backtrace(pos(0), pos(1)));
    parent.cost = 1;
    parent.gcost = currAction.gcost - parent.cost;
    return parent;
  }
}

/** Return a vector of possible actions at this particular action
 *  @param currAction the current action of the robot)
 *  @param map the map of the environment
 *  @return the list of possible actions
 */
static vector<MotionAction> getNextAction(MotionAction currAction, imat &map) {
  mat neighbor4 = reshape(mat({
      0, 0, -1, 1,
      1, -1, 0, 0
    }), 4, 2).t();
  vector<enum ActionId> neighborActions = { MOVE_FORWARD, MOVE_BACKWARD, MOVE_LEFT, MOVE_RIGHT };
  vector<MotionAction> actionlist;
  for (int i = 0; i < 4; i++) {
    MotionAction action(currAction.x + neighbor4(0, i), currAction.y + neighbor4(1, i), neighborActions[i]);
    // check feasibility of the action
    if (action.x >= map.n_cols || action.y >= map.n_rows ||
        map(action.x, action.y) > 0.5) {
      continue;
    }
    bool h1 = currAction.id == MOVE_LEFT || currAction.id == MOVE_RIGHT;
    bool v1 = currAction.id == MOVE_FORWARD || currAction.id == MOVE_BACKWARD;
    bool h2 = action.id == MOVE_LEFT || currAction.id == MOVE_RIGHT;
    bool v2 = action.id == MOVE_FORWARD || currAction.id == MOVE_BACKWARD;
    action.cost = ((h1 && v2) || (v1 && h2)) ? 2 : (action.id != currAction.id ? 3 : 1); // have this cost function take into account the pose difference
    action.gcost = currAction.gcost + action.cost;
    actionlist.push_back(action);
  }
  return actionlist;
}
