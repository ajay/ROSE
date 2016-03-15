/* Written by Cedric Blake

This script is used to push the displacement values of each encoder
to the database "rosedb"

currently, this script only simulates the encoder values by using
a random number generator. the while loop breaks after 5 iterations

stored within the "encoders" collection of the "rosedb" database

*/

#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string>
#include <unistd.h> /* used to sleep */

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

using namespace std;

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

int main() {

	//stores the values of the encoders corresponding to each wheel
	int tl = 0;
	int tr = 0;
	int bl = 0;
	int br = 0;

	//used to create a client connection and connect to a mongo instance
	mongocxx::instance inst{};
	//NOTE: make sure to have "mongocxx::uri{}"
	mongocxx::client conn{mongocxx::uri{}};

	auto db = conn["rosedb"];
	auto encoders = db["encoders"];


	//clears the collection on every reset
	encoders.delete_many({});

	//this document is used to reinitialize the position and velocity reading
	//each time we need to restart the robot, as the current readings would
	//now be out of date
	bsoncxx::document::value initializer = 
	document{} << "top left" << tl << "top right" << tr 
	<< "bottom left" << bl << "bottom right" << br << finalize;

	encoders.insert_one(move(initializer));



	srand(time(NULL));

	int count = 5;

	while (count > 0){

		bsoncxx::builder::stream::document encoderVals;

		//used to update all the values in the encoderVals document
		//initialized within the while loop so that it is reset
		//and does not have multiple "$set" documents appended
		bsoncxx::builder::stream::document updateVals;

		tl = rand() % 100;
		tr = rand() % 100;
		bl = rand() % 100;
		br = rand() % 100;


		//update values here
		updateVals << "$set" << open_document
			<< "top left" << tl << "top right" << tr 
			<< "bottom left" << bl << "bottom right" << br 
			<< close_document;

		encoderVals << "top left" << open_document
		<< "$exists" << true << close_document;

		encoders.update_one(encoderVals.view(), updateVals.view());

		auto cursor = encoders.find({}); //"encoders" is the collection
		//printing out for testing purposes
		for (auto&& doc : cursor ) {
			cout<<bsoncxx::to_json(doc)<<endl;
		}

		count--;
		usleep(1000000); //sleep for 1 second
	}

	return 0;
 
}