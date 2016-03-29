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

void dbconn::db_recv_update()
{
	mongocxx::instance inst{};					// Used to create a client connection and connect to a mongo instance
	mongocxx::client conn{mongocxx::uri{}}; 	// NOTE: make sure to have "mongocxx::uri{}"
	auto db = conn["rosedb"];					// Connect to the rosedb database

	while (1)
	{
		read_state(db);
		//read_speed(db);
	}
}

void dbconn::read_state(mongocxx::v_noabi::database db)
{
	auto state = db["mycollection"];

	// This is used in the cursor for loop to capture the value of the document's key
	bsoncxx::document::element e;

	// Query all documents which have "state" as a key
	auto cursor = state.find(document{} << "state" << open_document << "$exists" << true << close_document << finalize);

	document moveDoc;

	for (auto&& doc : cursor )
	{
		// Capture value of "state"
		e = doc["state"];

		// Convert a type bsoncxx::document::element to a type std::string
		string direction = e.get_utf8().value.to_string();
		this->data_recv.direction = direction;
	}
}

void dbconn::read_speed(mongocxx::v_noabi::database db)
{
	srand(time(NULL));
	auto speed = db["mycollection"];

	bsoncxx::document::element e;

	// Query all documents which have "speed" as a key
	auto cursor = speed.find(document{} << "speed" << open_document << "$exists" << true << close_document << finalize);

	document moveDoc;

	for (auto&& doc : cursor )
	{
		// Capture value of "speed"
		e = doc["speed"];

		// Convert a type bsoncxx::document::element to a type double
		double s = e.get_double().value;
		this->data_recv.speed = s;
	}
}