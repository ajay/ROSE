/*code specific to recieving orders from the database (for demo 2) */

#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string>
#include <unistd.h> /* used to sleep */
#include <string.h> /*for string parsing */
#include <cstring> /* for converting from string to char array for strtok function */

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include "dbconntwo.h"

using namespace std;

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

dbconn::dbconn() {
}

dbconn::~dbconn() {
}

//initialize will reset all documents on system startup
//will insert corresponding documents if they do not exist yet
void dbconn::init_rose_status(mongocxx::v_noabi::database db) {
	auto init = db["robo_info"];

	init.delete_many({});

	double t;
	t = ((double)clock() / CLOCKS_PER_SEC) * 1000; // Gives the time in seconds

	//initialize robot parameters
	bsoncxx::document::value init_clock = document{} << "clock" << t << finalize;
	bsoncxx::document::value init_encoders = document{} << "encoders" << open_array
		<< 0 << 0 << 0 << 0 << close_array << finalize;
	bsoncxx::document::value init_volt = document{} << "current_voltage" << 0 << finalize;
	bsoncxx::document::value init_state = document{} << "current_state" << "" << finalize;
	bsoncxx::document::value init_arm_pos = document{} << "arm_pos" << open_array
		<< 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << close_array << finalize;
	bsoncxx::document::value init_base_vel = document{} << "base_vel" << open_array
		<< 0.0 << 0.0 << 0.0 << 0.0 << close_array << finalize;
	bsoncxx::document::value init_arm_vel = document{} << "arm_vel" << open_array
		<< 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << close_array << finalize;


	init.insert_one(move(init_clock));
	init.insert_one(move(init_encoders));
	init.insert_one(move(init_volt));
	init.insert_one(move(init_state));
	init.insert_one(move(init_arm_pos));
	init.insert_one(move(init_base_vel));
	init.insert_one(move(init_arm_vel));

}

void dbconn::recv_data(mongocxx::v_noabi::database db) {

	if (customer_order.parsed_items.size() != 0) {
		//then the order still exists, it has not been fulfilled
		return;
	}

	bsoncxx::document::element e;

	auto orders = db["orders"];

	auto cursor = orders.find(document{} << "items" << open_document << "$exists" << true << close_document << finalize);

	// //used as parameter to remove the order that has been retireved by the robot
	// bsoncxx::builder::stream::document rm;

	// if the data structure to store an order is empty, then the robot is not
	//currently carrying out an order. therefore we can store another one

	//for an item, we will need to push every parameter to customer_order (item, price, etc)
	//before removing the document



	for (auto&& doc : cursor) {
		e = doc["items"];
		customer_order.item = e.get_utf8().value.to_string();
		e = doc["prices"];
		customer_order.price = e.get_utf8().value.to_string();
		e = doc["table"];
		string table_string = e.get_utf8().value.to_string();
		customer_order.table = stoi(table_string);

		//delete the document
		orders.delete_one(doc);
		//break out the for loop so that we only remove one order
		break;
	}

	if (customer_order.item == "") {
		cout<<"the database is empty \n";
		return;
	}
	//from here we will have to parse the information using strtok.
	//store the info using 2-d vector with each row being the attributes of each item
	//of each row:
	//index 0 is the item name
	//index 1 is the item price

	//PRINT FOR TESTING PURPOSES
	// cout<<customer_order.item.length()<<endl;
	// cout<<customer_order.item<<endl;

	//initialization
	//create "item_array" to store parsed items in a c-string (char array)

	//TODO: getting a seg fault here. can pull from database but apparently am bad
	//at mem allocation.
	char* item_array = new char[customer_order.item.length() + 1];
	strcpy(item_array, customer_order.item.c_str());

	vector<string> init_item;
	customer_order.parsed_items.push_back(init_item);
	char* item_token = strtok(item_array,",");
	customer_order.parsed_items[0].push_back(string(item_token));

	//keep track of each item pushed to vector
	int count_item = 1;

	while (1) {

		//reinitialize for new item input
		vector<string> init_item;

		//checks if we have reached the end of our string 
		//strtok returns null if it is done parsing

		customer_order.parsed_items.push_back(init_item);

		item_token = strtok(NULL,",");
		if (item_token == NULL) {
			break;
		}
		customer_order.parsed_items[count_item].push_back(string(item_token));

		count_item++;
	}

	//now do same thing with "price_array"
	char* price_array = new char[customer_order.price.length() + 1];
	strcpy(price_array, customer_order.price.c_str());

	char* price_token = strtok(price_array,",");
	customer_order.parsed_items[0].push_back(string(price_token));

	// cout<<customer_order.parsed_items[0][0]<<endl;
	// cout<<customer_order.parsed_items[0][1]<<endl;

	//keep track of each item pushed to vector
	count_item = 1;

	while (1) {
		price_token = strtok(NULL,",");
		if (price_token == NULL) {
			break;
		}
		customer_order.parsed_items[count_item].push_back(string(price_token));
		count_item++;
	}

	delete item_array;
	delete price_array;

	//PRINT FOR TESTING PURPOSES (the vector of vectors)
	count_item = 0;
	for (vector<vector<string>>::iterator i = customer_order.parsed_items.begin(); 
		i != customer_order.parsed_items.end(); ++i) {
		for (vector<string>::iterator j = i->begin(); j != i->end(); ++j) {
			cout<<*j<<endl;
		}
	}


}

//this fucntion is used to clear ROSE's db_recv_order struct
//so that ROSE knows that the order has been served
void dbconn::clear_data() {
	customer_order.item = "";
	customer_order.price = "";
	customer_order.parsed_items.clear();
	customer_order.table = 0;
}

void dbconn::send_data(mongocxx::v_noabi::database db)
{	
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

	/*------------ Send Encoder ---------------*/

	document enc_doc;
	enc_doc << "encoders" << open_document << "$exists" << true << close_document;

	document update_enc;
	//test code
	update_enc << "$set" << open_document << "encoders" << open_array
		<< 1 << 1 << 3 << 4 << close_array << close_document;


	//actual code
	// update_enc << "$set" << open_document << "encoders" << open_array
	// 	<< rose_data_send.encoders[0] << rose_data_send.encoders[1] << rose_data_send.encoders[2] 
	// 	<< rose_data_send.encoders[3] << close_array << close_document;

	store_data.update_one(enc_doc.view(), update_enc.view());

	/*------------ Send State -----------------*/

	document state_doc;
	state_doc << "current_state" << open_document << "$exists" << true << close_document;

	document update_state;
	update_state << "$set" << open_document << "current_state" << rose_data_send.state << close_document;

	store_data.update_one(state_doc.view(), update_state.view());

	/*----------- Send Arm Position Values -----------------*/

	document arm_pos_doc;
	arm_pos_doc << "arm_pos" << open_document << "$exists" << true << close_document;

	document update_arm_pos;
	update_arm_pos << "$set" << open_document << "arm_pos" << open_array
		<< rose_data_send.arm_pos[0] << rose_data_send.arm_pos[1] << rose_data_send.arm_pos[2]
		<< rose_data_send.arm_pos[3] << rose_data_send.arm_pos[4] << rose_data_send.arm_pos[5]
		<< close_array << close_document;

	store_data.update_one(arm_pos_doc.view(), update_arm_pos.view());

	/*-------------- Send Base Motor Velocity --------------*/

	document base_vel_doc;
	base_vel_doc << "base_vel" << open_document << "$exists" << true << close_document;

	document update_base_vel;
	update_base_vel << "$set" << open_document << "base_vel" << open_array
		<< rose_data_send.base_vel[0] << rose_data_send.base_vel[1] << rose_data_send.base_vel[2]
		<< rose_data_send.base_vel[3] << close_array << close_document;

	store_data.update_one(base_vel_doc.view(), update_base_vel.view());

	/*------------- Send Arm Motor Velocity ---------------*/

	document arm_vel_doc;
	arm_vel_doc << "arm_vel" << open_document << "$exists" << true << close_document;

	document update_arm_vel;
	update_arm_vel << "$set" << open_document << "arm_vel" << open_array
		<< rose_data_send.arm_vel[0] << rose_data_send.arm_vel[1] << rose_data_send.arm_vel[2]
		<< rose_data_send.arm_vel[3] << rose_data_send.arm_vel[4] << rose_data_send.arm_vel[5]
		<< close_array << close_document;

	store_data.update_one(arm_vel_doc.view(), update_arm_vel.view());



}

void dbconn::db_update() {
	mongocxx::instance inst{};					// Used to create a client connection and connect to a mongo instance
	mongocxx::client conn{mongocxx::uri{}}; 	// NOTE: make sure to have "mongocxx::uri{}"
	auto db = conn["rosedb"];					// Connect to the rosedb database

	//re-initialize database on system startup
	init_rose_status(db);

	while (1)
	{
		recv_data(db);
     	send_data(db);

     	//This is only for testing purposes
     	//this function should only be called once the item has been delivered
     	clear_data();

		usleep(1000000);
	}
}

int main() {
	dbconn rose_db;
	rose_db.db_update();
}