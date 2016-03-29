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
		std::string direction;
		double speed;
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

		/**
		 * Get data from the database, sent from the webapp
		 */
		void db_recv_update();

		/**
		 *
		 */
		struct db_recv data_recv;

		/**
		 *
		 */
		struct db_send data_send;

	private:
		/**
		 * Read state string from mongodb
		 * @param db Mongodb database instance
		 */
		void read_state(mongocxx::v_noabi::database db);

		/**
		 * Read speed int from mongodb
		 * @param db Mongodb database instance
		 */
		void read_speed(mongocxx::v_noabi::database db);
};

#endif