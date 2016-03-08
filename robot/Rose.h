#ifndef ROSE_H
#define ROSE_H

#include <armadillo>
#include "serial.h"

class Rose
{
	public:
		/**
		 * Creates Rose object to interact with robot, as well as
		 * connects to robot using connect()
		 */
		Rose(void);

		/**
		 * Disconnects Rose object from robot
		 */
		~Rose(void);

		/**
		 * Queries the number of arduinos connected
		 * @return Number of connected arduinos
		 */
		int numconnected(void);

		/**
		 * Is Rose connected to all of it's devices?
		 * @return The status of Rose's connection
		 */
		bool connected(void);

		/**
		 * Set's the previous motion back to 0, useful if something goes
		 * wrong or a reset is required for any reason
		 */
		void reset(void);

		/**
		 * Resets values of encoders back to zero, on the arduino side
		 * This is done by sending a reset string over serial:
		 * "[reset]\n"
		 */
		void reset_encoders(void);

		/**
		 * Attemps to connect to all mounted arduinos. Establishes a serial
		 * connection with each one, and differentiates them using each arduino's
		 * DEV_ID.
		 * @return True on a succsessful connection with all devices
		 */
		bool connect(void);

		/**
		 * Disconnects from all connected arduinos
		 */
		void disconnect(void);

		/**
		 * Send strings over serial comm to the ardunios
		 * @param Motion string to send over serial. Current format:
		 * "[leftFront, rightFront, leftBack, rightBack]\n"
		 */
		void send(const arma::vec &motion);

		/**
		 * Used to hold the state of if the encoders
		 * need to be reset on the arduino or not
		 */
		bool reset_enc;

		/**
		 * Used as a sort of "signal handler"
		 * False by default, Rose will stop if it is set to true.
		 */
		bool startStop;

		/**
		 * Array that holds the *current* speeds of all motors,
		 * sent from the arduino
		 */
		int motor_speeds[4] = {-1};

		/**
		 * Array that holds the *current* values of all encoders,
		 * send from the arduino
		 */
		int encoder[4] = {-1};

	private:
		static void* commHandler(void*);

		void threadSend(const arma::vec &motion);
		arma::vec recv(void);
		void threadRecv(void);

		arma::vec commSend;
		arma::vec commRecv;
    	pthread_t *update_thread;
		pthread_mutex_t *commSendLock;
		pthread_mutex_t *commRecvLock;

    	// Threading stuff for handling the communcation
		arma::vec prev_motion;
		arma::vec motion_const;
		int robotid;
		std::vector<serial_t *> connections;
		std::vector<int> ids;
		std::vector<char *> pports;
};

#endif