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
		 * Send strings over serial comm to the ardunios. Manages
		 * the locks required for sending and calls threadSend() to
		 * actually send the serial string
		 * @param motion
		 *        Motion string to send over serial. Current format:
		 *        "[leftFront, rightFront, leftBack, rightBack]\n"
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

		/**
		 * Holds voltage of 12V battery powering TK1
		 */
		float twelve_volt_voltage = 0;

		/**
		 * Holds current draw from 12V battery powering TK1
		 */
		float twelve_volt_current = 0;

	private:
		/**
		 * Private method for handing data transfer over serial.
		 * Sets up and maintains mutex locks for sending & receiving
		 */
		static void* commHandler(void*);

		/**
		 * Private methods that accepts any data received over serial and
		 * stores it in the correct location. Currently gets info from drive
		 * base, as well as the arm.
		 */
		void threadRecv(void);

		/**
		 * Sends data over serial after being called by send().
		 * Determines which arduino to send data to based on the
		 * DEV_ID of the arduino
		 * @param motion
		 *        The vector to be sent over to the arduinos
		 */
		void threadSend(const arma::vec &motion);

		/**
		 * Manages receiving data over serial. Manages mutex locks
		 * when actually reading the data. This function is only useful
		 * if the actually data received is required. Otherwise, use
		 * threadRecv(), which will actually place the data somewhere instead.
		 * @return  A vector of data received
		 */
		arma::vec recv(void);

		/**
		 * Hold the last value that was assigned to the motors over serial.
		 * Used to make sure the same values are not sent over twice.
		 */
		arma::vec prev_motion;

		/**
		 * Vector holding values of 255. Used to convert double from
		 * -1 to 1 to a pwm value from -255 to 255. Can be used to limit
		 * overall motion range between any two values.
		 */
		arma::vec motion_const;

		/**
		 * Stores current robotid, useful in the future if multiple
		 * ROSE robots are ever connected
		 */
		int robotid;

		/**
		 * Vector holding all current connections to arduinos
		 */
		std::vector<serial_t *> connections;

		/**
		 * Vector holding DEV_IDs of all current connections
		 */
		std::vector<int> ids;

		/**
		 * Vector holding all pport strings used for arduinos to
		 * connect. Stored with a prefix of '/dev/ttyACM#'
		 */
		std::vector<char *> pports;

		// Threading stuff for handling the communcation
		arma::vec commSend;
		arma::vec commRecv;
		pthread_t *update_thread;
		pthread_mutex_t *commSendLock;
		pthread_mutex_t *commRecvLock;
};

#endif
