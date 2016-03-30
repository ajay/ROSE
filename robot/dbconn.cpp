/* Written By: Cedric Blake

This script allows the robot to pull information from the 
database and store the information within variables.
the variables will then be passed to the function that
make the robot move accordingly

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string>
#include <unistd.h> /* used to sleep */
#include <time.h>
#include "dbconn.h"

/* mongodb includes */

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>


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

/*end of mongodb includes */

using namespace std;

struct data_recieve {
	//this struct contains all the data that the robot recieves from the webapp
	string direction;
	double speed;
	int rotation;
};

struct data_send {
	//this struct contains all data that the robot will send to the webapp
	int encoders[4];
	int voltage;
};

struct data_recieve rose_data_rec;
//this integer is used for the simulations
int test = 0;

/*this script specifies how the robot will access and manage information
from the database */

void read_state(mongocxx::v_noabi::database db) {

	auto state = db["mycollection"]; //mycollection currently holds the document about the state

	//SIMULATOR: tests string retreval
	// state.delete_many({});
	// if (test == 0) {
	// 	state.insert_one(document{} << "state" << "left" << finalize);
	// 	test++;
	// } else if (test == 1) {
	// 	state.insert_one(document{} << "state" << "right" << finalize);
	// 	test++;
	// } else {
	// 	state.insert_one(document{} << "state" << "up" << finalize);
	// 	test = 0;
	// }

	//this is used in the cursor for loop to capture the value of the document's key
	bsoncxx::document::element e;

	//query all documents which have "state" as a key
	auto cursor = state.find(document{} << "state" << open_document
		<< "$exists" << true << close_document << finalize);

	document moveDoc;

	for (auto&& doc : cursor ) {
		/* use this to print out the entire document
		cout<<bsoncxx::to_json(doc)<<endl;
		*/

		//capture value of "state", NOTE: the operator [] only works on
		//a type bsoncxx::document::view (which is what "doc" is)
		e = doc["state"];
		//convert a type bsoncxx::document::element to a type std::string
		string direction = e.get_utf8().value.to_string();
		rose_data_rec.direction = direction;
	}

	printf("moving: %s\n", rose_data_rec.direction.c_str());

}

void read_speed(mongocxx::v_noabi::database db) {
	auto speed = db["mycollection"];

	bsoncxx::document::element e;

	//SIMULATOR: test double retreval
	//srand(time(NULL));
	//speed.delete_many({});
	//NOTE: mongodb cannot implicitly convert an int to double, so
	//we need to define the value as a double explicitely before storing
	//into the collection if we want to extract as a double
	// double randDouble = rand() % 10 + 1;
	// speed.insert_one(document{} << "speed" << randDouble << finalize);

	//query all documents which have "speed" as a key
	auto cursor = speed.find(document{} << "speed" << open_document
		<< "$exists" << true << close_document << finalize);

	document moveDoc;

	for (auto&& doc : cursor ) {
		/* use this to print out the entire document
		cout<<bsoncxx::to_json(doc)<<endl;
		*/

		//capture value of "speed", NOTE: the operator [] only works on
		//a type bsoncxx::document::view (which is what "doc" is)
		e = doc["speed"];
		//convert a type bsoncxx::document::element to a type double
		double s = e.get_double().value;
		rose_data_rec.speed = s;
	}

	
	printf("speed: %f\n", rose_data_rec.speed);
}

void read_rotation(mongocxx::v_noabi::database db) {
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
		rose_data_rec.rotation = s;
	}

	printf("rotation state: %i\n", rose_data_rec.rotation);
}

int main() {

	//used to create a client connection and connect to a mongo instance
	mongocxx::instance inst{};
	//NOTE: make sure to have "mongocxx::uri{}"
	mongocxx::client conn{mongocxx::uri{}};
	//connect to the rosedb database
	auto db = conn["rosedb"];

	while (1) {
		read_state(db);
		read_speed(db);
		read_rotation(db);
		usleep(1000000);
	}

	return 0;
}