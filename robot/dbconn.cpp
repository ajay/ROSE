/**
 * This script allows the robot to pull information from the
 * database and store the information within variables.
 * The variables will then be passed to the function that
 * make the robot move accordingly
 */

#include "dbconn.h"

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;
using bsoncxx::stdx::string_view;

using namespace std;

dbconn::dbconn()
{
}

dbconn::~dbconn()
{
}

//initialize will reset all documents on system startup
//will insert corresponding documents if they do not exist yet
void dbconn::init_rose_status(mongocxx::v_noabi::database db) {
	auto init = db["robo_info"];

	init.delete_many({});

	/* SUBJECT TO CHANGE
	initialize timestamp
	bsoncxx::document::value init_clock = document{} << "_id" << 0 << "clock" << 0.0 << finalize;

	init.insert_one(move(init_clock));

    initialize voltage */

	bsoncxx::document::value init_volt = document{} << "_id" << 1 << "current_voltage" << 0 << finalize;

	init.insert_one(move(init_volt));
}

void dbconn::recv_data(mongocxx::v_noabi::database db)
{
	// Find the collection the webapp is pushing to
	auto mycoll = db["mycollection"];

	// This is used in the cursor for loop to capture the value of the document's key
	bsoncxx::document::element e;

	/*
	 *
	 * SUBJECT TO CHANGE
	*----------- recieve timestamp ------------
	//NOTE: needs to be initialized from web app's end before we can test this

	auto cursor = mycoll.find(document{} << "clock" << open_document << "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor) {
		e = doc["clock"];

		double time_stamp = e.get_double().value;
		printf("this timestamp: %f\n", time_stamp);
		//check if more than 10 ms have elapsed. if so, then stop the robot
		//NOTE: test with the value 0.1 and see if we can still input direction / speed / rotation
		//we shouldnt be able to though
		if (time_stamp - rose_data_recv.time_stamp > 10) {
			rose_data_recv.direction = "STOP";
			rose_data_recv.speed = 0.0;
			rose_data_recv.rotation = 0;
			return;
		} else {
			rose_data_recv.time_stamp = time_stamp;
		}
	}

	*/


	///// Receive state

	// Query all documents which have "state" as a key
	auto cursor = mycoll.find(document{} << "state" << open_document << "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor )
	{
		// Capture value of "state"
		e = doc["state"];

		// Convert a type bsoncxx::document::element to a type std::string
		string direction = e.get_utf8().value.to_string();
		rose_data_recv.direction = direction;
	}

	// Receive rotation

	// Query all documents which have "rotation" as a key
	cursor = mycoll.find(document{} << "rotation" << open_document << "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor)
	{
		// Capture value of "rotation"
		e = doc["rotation"];
		// Acquires the rotation state: -1 = counter clockwise, 0 is NULL, 1 is clockwise
		int s = e.get_int32().value;
		rose_data_recv.rotation = s;
	}

	// This control flow allows the robot to turn
    if (rose_data_recv.direction == "NORTH")
    {
		if (rose_data_recv.rotation == 1)
        {
			rose_data_recv.direction = "NORTHCLOCKWISE";
		}
        else if (rose_data_recv.rotation == -1)
        {
			rose_data_recv.direction = "NORTHCOUNTERCLOCKWISE";
		}
	}
    else if (rose_data_recv.direction == "SOUTH")
    {
		if (rose_data_recv.rotation == 1)
        {
			rose_data_recv.direction = "SOUTHCLOCKWISE";
		}
        else if (rose_data_recv.rotation == -1)
        {
			rose_data_recv.direction = "SOUTHCOUNTERCLOCKWISE";
		}
	}
    else if (rose_data_recv.direction == "STOP")
    {
		if (rose_data_recv.rotation == 1)
        {
			rose_data_recv.direction = "CLOCKWISE";
		}
        else if (rose_data_recv.rotation == -1)
        {
			rose_data_recv.direction = "COUNTERCLOCKWISE";
		}
	}

	///// Receive Speed

	// Query all documents which have "speed" as a key
	cursor = mycoll.find(document{} << "speed" << open_document << "$exists" << true << close_document << finalize);

	for (auto&& doc : cursor )
	{
		// Capture value of "speed"
		e = doc["speed"];
		// Convert a type bsoncxx::document::element to a type double
		double s = e.get_double().value;
		rose_data_recv.speed = s;
	}

	// print statements for testing
	// printf("timestamp: %f\n", rose_data_recv.time_stamp);
    // cout<<"move: "<<rose_data_recv.direction<<endl;
	// printf("%0.2f\n", rose_data_recv.speed);
	// printf("rotate: %i\n", rose_data_recv.rotation);
}

/* ------------ Sending Functions ---------------*/

void dbconn::send_data(mongocxx::v_noabi::database db)
{
	init_rose_status(db);
	auto store_data = db["robo_info"];

	/* NOTE
	 * for every parameter of the robot that we need push into the database,
	 * we must first initialize the parameter in the "init_rose_status()" function
	 */

	/*------------ Send Timestamp -------------*/

	// This pushes the time since process start.

	document find_clock;
	find_clock << "clock" << open_document << "$exists" << true << close_document;

	double t;
	t = ((double)clock() / CLOCKS_PER_SEC) * 1000; // Gives the time in seconds

//	printf("%f\n", t);

	// The web app will check to see if the clock has been updated in the database
	document update_clock;
	update_clock << "$set" << open_document << "clock" << t << close_document;

	// Perform the update
	store_data.update_one(find_clock.view(),update_clock.view());

	/*------------ Send Voltage ---------------*/

	// Search for a document with a key of "current_voltage"
	// NOTE: checking for existence is a document unto itself)
	document volt_doc;
	volt_doc << "current_voltage" << open_document << "$exists" << true << close_document;

	// Update a document with a key of "current_voltage"
	// NOTE: voltage is stored in a struct within the class dbconn
	document update_volt;
	rose_data_send.twelve_volt_voltage = 2.2;
	update_volt << "$set" << open_document << "current_voltage" << rose_data_send.twelve_volt_voltage << close_document;

	// Perform the update
	store_data.update_one(volt_doc.view(), update_volt.view());
}

/* -------------DB connection---------------------*/

void dbconn::db_update()
{
	mongocxx::instance inst{};					// Used to create a client connection and connect to a mongo instance
	mongocxx::client conn{mongocxx::uri{}}; 	// NOTE: make sure to have "mongocxx::uri{}"
	auto db = conn["rosedb"];					// Connect to the rosedb database

	while (1)
	{
		recv_data(db);
//      send_data(db);
//		usleep(1000000);
	}
}

/*main function for testing purposes only */
//int main() {
//	dbconn rose_db;
//	rose_db.db_update();
//	return 0;
//}
