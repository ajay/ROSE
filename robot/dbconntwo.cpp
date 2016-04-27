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

void dbconn::init_rose_status(mongocxx::v_noabi::database db) {
	recv_data(db);
}

void dbconn::recv_data(mongocxx::v_noabi::database db) {
	if (customer_order.item != "") {
		//if "item" is not empty, then we do not retrieve from the database
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

	//from here we will have to parse the information using strtok.
	//store the info using 2-d vector with each row being the attributes of each item
	//of each row:
	//index 0 is the item name
	//index 1 is the item price

	cout<<customer_order.item.length()<<endl;
	cout<<customer_order.item<<endl;
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

	count_item = 0;
	for (vector<vector<string>>::iterator i = customer_order.parsed_items.begin(); 
		i != customer_order.parsed_items.end(); ++i) {
		for (vector<string>::iterator j = i->begin(); j != i->end(); ++j) {
			cout<<*j<<endl;
		}
	}


}

int main() {
	mongocxx::instance inst{};					
	mongocxx::client conn{mongocxx::uri{}};
	auto db = conn["rosedb"];

	dbconn rose_db;
	rose_db.init_rose_status(db);
}