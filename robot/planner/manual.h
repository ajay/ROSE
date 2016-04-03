#ifndef __TK_MANUAL_H__
#define __TK_MANUAL_H__

namespace manual_input {
  /** Try to connect to an input device
   */
  void connect(void);

  /** Try to disconnect from an input device
   */
  void disconnect(void);

  /** Try to update the robot
   */
  void update(void);
}

#endif
