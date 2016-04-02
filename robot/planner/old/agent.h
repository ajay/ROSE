#ifndef __TK_AGENT_H__
#define __TK_AGENT_H__

#include "coord.h"

namespace agent {
  // Functions
  int wakeup(void);
  void set_enable(bool en);
  void gotosleep(void);
  int get_poses(pose3d_t *base, pose3d_t *arm);
}

#endif
