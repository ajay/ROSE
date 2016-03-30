#ifndef BASEROBOT_H
#define BASEROBOT_H

#include <armadillo>
#include "serial.h"

#define NO_ROBOT			0x00000000
#define TENNIS_BALL_ROBOT	0x00000002
#define TACHIKOMA			0x00000003
#define ARM					0x00000005

class BaseRobot
{
	public:
		/** Constructor
		 *  @param robotid
		 *    the id for the robot
		 */
		BaseRobot(int robotid);

		/** Deconstructor
		 */
		~BaseRobot(void);

		/** Get the robot's id
		 *  @return the id
		 */
		int id(void);

		/** Default connect method
		 *  @return true if connected to at least one device,
		 *    false otherwise
		 *  @note
		 *    will try to connect based on [%d ... \n
		 *    if the id <= 0, it will disconnect
		 */
		virtual bool connect(void);

		/** Default connect detection method
		 *  @return true if connected, false otherwise
		 */
		virtual bool connected(void);

		/** Default disconnect method
		 */
		virtual void disconnect(void);

		/** Default send method
		 *  @param motion
		 *    the motion vector
		 */
		virtual void send(const arma::vec &motion);

		/** Default recv method
		 *  @return a vector with no elements
		 */
		virtual arma::vec recv(void);

		/** Default reset method
		 */
		virtual void reset(void);

	protected:
		int robotid;
		std::vector<serial_t *> connections;
		std::vector<int> ids;
		std::vector<char *> pports;
};

#endif