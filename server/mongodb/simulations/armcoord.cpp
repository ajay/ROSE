/* Written By: Cedric Blake

this script currently simulates storing information of the arm's positioning 
from variables into the database

stored withing the collection "arm positioning" of the "rosedb" database
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
	int cordX = 0; //horizontal position
	int cordY = 0; //height
	int cordZ = 0; //depth
	int cordTheta = 90; //rotation of claw (yaw)

	/*NOTE: theta will be between 0 and 180 degrees with 90 degrees being the neutral point (no rotation)
	0 degrees will be complete rotation to right
	180 degrees will be complete rotation to left
	linear coordinates (x,y,z), will be between 0 and 255 with 0 being fully contract and 255 being fully extended
	*/

	//used to create a client connection and connect to a mongo instance
	mongocxx::instance inst{};
	//NOTE: make sure to have "mongocxx::uri{}"
	mongocxx::client conn{mongocxx::uri{}};

	auto db = conn["rosedb"];
	auto armPos = db["arm positioning"];

	armPos.delete_many({});

	bsoncxx::document::value initializer = 
	document{} << "X" << cordX << "Y" << cordY
	<< "Z" << cordZ << "Theta" << cordTheta << finalize;

	armPos.insert_one(move(initializer));

	//initialize random seed
	srand(time(NULL));

	//breaking condition for while loop
	int count = 5;
	while (count > 0) {

		bsoncxx::builder::stream::document armVals;

		//used to update all the values in the encoderVals document
		//initialized within the while loop so that it is reset
		//and does not have multiple "$set" documents appended
		bsoncxx::builder::stream::document updateVals;

		// assign random values to each coordinate
		cordX = rand() % 255;
		cordY = rand() % 255;
		cordZ = rand() % 255;
		cordTheta = rand() % 180;


		//used to identify the document we are searching for in the update function
		//says to query any document that contains "X" as a key/ field
		armVals << "X" << open_document << "$exists" << true << close_document;

		updateVals << "$set" << open_document << "X" << cordX
			<< "Y" << cordY << "Z" << cordZ << "Theta" << cordTheta << close_document;

		armPos.update_one(armVals.view(), updateVals.view());

		//find and print all documents (will be removed after testing is complete)
		auto cursor = armPos.find({});
		for (auto&& doc : cursor) {
			cout<<bsoncxx::to_json(doc)<<endl;
		}

		count--;
		usleep(1000000); //sleep for 1 second
	}

	return 0;
}