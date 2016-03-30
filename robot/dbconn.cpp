/**
 * This script allows the robot to pull information from the
 * database and store the information within variables.
 * The variables will then be passed to the function that
 * make the robot move accordingly
 */

#include "dbconn.h"

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

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;
using bsoncxx::stdx::string_view;

using namespace std;

// struct db_recv {
// 	//this struct contains all the data that the robot recieves from the webapp
// 	string direction;
// 	double speed;
// 	int rotation;
// };

// struct data_send {
// 	//this struct contains all data that the robot will send to the webapp
// 	int encoders[4];
// 	double twelve_volt_voltage;
// 	string state;
// };

// dbconn::dbconn()
// {
// }

// dbconn::~dbconn()
// {
// }

//create an instance of the class "dbconn"
dbconn db;

void dbconn::read_state(mongocxx::v_noabi::database db)
{
	auto state = db["mycollection"];

	// This is used in the cursor for loop to capture the value of the document's key
	bsoncxx::document::element e;

	// Query all documents which have "state" as a key
	auto cursor = state.find(document{} << "state" << open_document << "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor )
	{
		// Capture value of "state"
		e = doc["state"];

		// Convert a type bsoncxx::document::element to a type std::string
		string direction = e.get_utf8().value.to_string();
		rose_data_recv.direction = direction;
	}

	printf("moving: %s\n", rose_data_recv.direction.c_str());

}

void dbconn::read_speed(mongocxx::v_noabi::database db)
{
	auto speed = db["mycollection"];

	bsoncxx::document::element e;

	//query all documents which have "speed" as a key
	auto cursor = speed.find(document{} << "speed" << open_document
		<< "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor )
	{
		// Capture value of "speed"
		e = doc["speed"];
		//convert a type bsoncxx::document::element to a type double
		double s = e.get_double().value;
		rose_data_recv.speed = s;
	}

	
	printf("speed: %f\n", rose_data_recv.speed);
}

void dbconn::read_rotation(mongocxx::v_noabi::database db) {
	auto rotation = db["mycollection"];
	//query documents which have a key of "rotation"
	auto cursor = rotation.find(document{} << "rotation" << open_document
		<< "$exists" << true << close_document << finalize);

	bsoncxx::document::element e;

	for (auto&& doc : cursor) {
		//capture the rotation direction. we have a constant speed for rotation
		e = doc["rotation"];
		//acquires the rotation state: -1 = counter clockwise, 0 is NULL, 1 is clockwise
		int s = e.get_int32().value;
		rose_data_recv.rotation = s;
	}

	printf("rotation state: %i\n", rose_data_recv.rotation);
}

void dbconn::db_recv_update()
{
	mongocxx::instance inst{};					// Used to create a client connection and connect to a mongo instance
	mongocxx::client conn{mongocxx::uri{}}; 	// NOTE: make sure to have "mongocxx::uri{}"
	auto db = conn["rosedb"];					// Connect to the rosedb database

	while (1)
	{
		read_state(db);
		read_speed(db);
		read_rotation(db);
		usleep(1000000);
	}
}

int main() {

	db.db_recv_update();
	return 0;
}