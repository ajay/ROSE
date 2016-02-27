#ifndef ROSE_H
#define ROSE_H

#include <armadillo>
#include "serial.h"

class Rose
{
	public:
		Rose(void);
		~Rose(void);
		int numconnected(void);
		void send(const arma::vec &motion);
		void readClear(void);
		arma::vec recv(void);
		void reset(void);
		int id(void);
		bool connect(void);
		bool connected(void);
		void disconnect(void);

		bool startStop;

		arma::vec commSend;
		arma::vec commRecv;
    	pthread_t *update_thread;
		pthread_mutex_t *commSendLock;
		pthread_mutex_t *commRecvLock;
		void threadSend(const arma::vec &motion); 
		void threadRecv(void);

		int motor_speeds[4] = {-1};
		int encoder[4] = {-1};

	private:
    	// Threading stuff for handling the communcation
		arma::vec prev_motion;
		arma::vec motion_const;
		int robotid;
		std::vector<serial_t *> connections;
		std::vector<int> ids;
		std::vector<char *> pports;
};

#endif
