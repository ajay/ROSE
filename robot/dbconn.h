#ifndef DBCONN_H
#define DBCONN_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>		// srand, rand
#include <string>
#include <time.h>		// time
#include <unistd.h>		// used to sleep

// Libbson includes
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

// Mongodb includes
#include <mongocxx/client.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/distinct.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class dbconn
{
	struct db_recv
	{
		double time_stamp = 0;
		std::string direction;
		double speed;
		int rotation;
	};

	struct db_send
	{
		int encoders[4];
		double twelve_volt_voltage;
		std::string state;
	};

	public:
		dbconn();
		~dbconn();

		//clear and insert a collection that holds the status of the robot to send to the web app
		void init_rose_status(mongocxx::v_noabi::database db);

		/**
		 * Get data from the database, sent from the webapp
		 */
		void db_update();


		/**
		 *
		 */
		struct db_recv rose_data_recv;

		/**
		 *
		 */
		struct db_send rose_data_send;

	private:
		/* reads data from the database
		 *currently reads the following:
		 *State (direction)
		 *speed
		 *rotational direction
		 */
		void recv_data(mongocxx::v_noabi::database db);

		/*sends information about the robot to the database
		 * for now, only sends voltage
		 */
		void send_data(mongocxx::v_noabi::database db);


};

#endif