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

struct data {
	string direction;
	double speed;
};

struct data roseData;
//this integer is used for the simulations
int test = 0;

/*this script specifies how the robot will access and manage information
from the database */

void read_state(mongocxx::v_noabi::database db) {

	auto state = db["mycollection"]; //mycollection currently holds the document about the state

	//SIMULATOR: tests string retreval
	// state.delete_many({});
	// if (test == 0) {
	// 	state.insert_one(document{} << "message" << "left" << finalize);
	// 	test++;
	// } else if (test == 1) {
	// 	state.insert_one(document{} << "message" << "right" << finalize);
	// 	test++;
	// } else {
	// 	state.insert_one(document{} << "message" << "up" << finalize);
	// 	test = 0;
	// }

	//this is used in the cursor for loop to capture the value of the document's key
	bsoncxx::document::element e;

	//query all documents which have "message" as a key
	auto cursor = state.find(document{} << "message" << open_document
		<< "$exists" << true << close_document << finalize);

	document moveDoc;

	for (auto&& doc : cursor ) {
		/* use this to print out the entire document
		cout<<bsoncxx::to_json(doc)<<endl;
		*/

		//capture value of "message", NOTE: the operator [] only works on
		//a type bsoncxx::document::view (which is what "doc" is)
		e = doc["message"];
		//convert a type bsoncxx::document::element to a type std::string
		string direction = e.get_utf8().value.to_string();
		roseData.direction = direction;
	}

	printf("%s\n", roseData.direction.c_str());

}

void read_speed(mongocxx::v_noabi::database db) {
	srand(time(NULL));
	auto speed = db["mycollection"];

	bsoncxx::document::element e;

	//SIMULATOR: test double retreval
	// speed.delete_many({});
	// //NOTE: mongodb cannot implicitly convert an int to double, so
	// //we need to define the value as a double explicitely before storing
	// //into the collection if we want to extract as a double
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
		roseData.speed = s;
	}

	
	printf("%f\n", roseData.speed);
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
		usleep(1000000);
	}

	return 0;
}