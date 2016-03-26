/* Written by Neil Patel

This script reads from the database "rosedb" and prints the state and speed data contained within it, using a fixed reading interval of 10 ms.
It assumes that state and speed data are stored in two separate collections, named "state" and "speed",
with the following schema for each document in these collections:
"State" collection: _id | state
where "state" is the actual state of the robot.
"Speed" collection: _id | speed
where "speed" is the current robot speed ranging from -1 to 1.
*/

#include <iostream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include <unistd.h> 	// used to sleep

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

int main(int, char**) {
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{}};

    auto db = conn["rosedb"];
    auto stateColl = db["state"];
    auto speedColl = db["speed"];
    //Clear collections first.
    stateColl.delete_many({});
    speedColl.delete_many({});

    //Step 1: Write to the database to populate the two collections.
    bsoncxx::builder::stream::document stateDoc;
    bsoncxx::builder::stream::document speedDoc;
    bsoncxx::builder::stream::document stateDoc2;
    bsoncxx::builder::stream::document speedDoc2;
    stateDoc << "state" << "FORWARDS";  //"state" and "speed" collections get two documents each
    stateDoc2 << "state" << "BACKWARDS"; 
    stateColl.insert_one(stateDoc.view());
    stateColl.insert_one(stateDoc2.view());
    speedDoc << "speed" << "1.0";
    speedDoc2 << "speed" << "0.0"; 
    speedColl.insert_one(speedDoc.view());
    speedColl.insert_one(speedDoc2.view());

    //Step 2: Read the database contents.
    int numIterRem = 3;

    // Query for all the documents in the "state" and "speed" collections 3 times, spacing each iteration by 10 ms.  
    std::cout << "DATABASE CONTENTS BEFORE:" << std:: endl;
    while (numIterRem > 0) {
        // @begin: cpp-query::cout << "Now printing contents of 'state' collection: " << std:: endl;
        auto cursor = db["state"].find({});
        for (auto&& doc : cursor) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
        auto cursor2 = db["speed"].find({});
    	std::cout << "Now printing contents of 'speed' collection: " << std:: endl;
    	for (auto&& doc : cursor2) {
    		std::cout << bsoncxx::to_json(doc) << std::endl;
    	}
        usleep(10000);	//sleep for 10 ms
        numIterRem--;
    }

    //Step 3: Write more data to the database (1 addtl document for each collection) for further reading testing.
    bsoncxx::builder::stream::document stateDoc3;
    bsoncxx::builder::stream::document speedDoc3;
    stateDoc3 << "state" << "RIGHT"; 
    stateColl.insert_one(stateDoc3.view());
    speedDoc3 << "speed" << "-1.0"; 
    speedColl.insert_one(speedDoc3.view());

    //Step 4: Query again for all documents in both collections 3 times with spacing of 10 ms between iterations.
    numIterRem = 3; //reset iteration count
    auto cursor = db["state"].find({});
    
    std::cout << "DATABASE CONTENTS AFTER:" << std:: endl;
    while (numIterRem > 0) {
        // @begin: cpp-query-all
        auto cursor3 = db["state"].find({});
        std::cout << "Now printing contents of 'state' collection: " << std:: endl;
        for (auto&& doc : cursor3) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
        auto cursor4 = db["speed"].find({});
        std::cout << "Now printing contents of 'speed' collection: " << std:: endl;
        for (auto&& doc : cursor4) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
        usleep(10000);  //sleep for 10 ms
        numIterRem--;
    }

    return 0;
}