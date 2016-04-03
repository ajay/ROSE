#ifndef __TK_PLANNER_H__
#define __TK_PLANNER_H__

namespace planner {
  thread *planningThread;
  mutex *planningLock;
  BaseRobot *body;

  // start planner
  int init(bool en = true);

  // manual control override
  bool autonomousEnable;
  bool keepRunning;

  // manual goal trees
  search_graph decision_space;

  void start(void);
  void stop(void);

  // stop the planner
  void destroy(void);

  // running thread (dont call)
  void run(void);
}

#endif
