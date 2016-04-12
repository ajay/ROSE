/*code specific to recieving orders from the database (for demo 2) */

#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <string>
#include <unistd.h> /* used to sleep */
#include <string.h> /*for string parsing */

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

vector<vector<string>> dbconn::recv_data(mongocxx::v_noabi::database db) {
	if (customer_order.item != "") {
		//if "item" is not empty, then we do not retrieve from the database
		return;
	}

	bsoncxx::document::element e;

	auto mycoll = db['mycollection'];

	auto cursor = mycoll.find(document{} << "item" << open_document << "$exists" << true << close_document << finalize);

	// //used as parameter to remove the order that has been retireved by the robot
	// bsoncxx::builder::stream::document rm;

	// if the data structure to store an order is empty, then the robot is not
	//currently carrying out an order. therefore we can store another one

	//for an item, we will need to push every parameter to customer_order (item, price, etc)
	//before removing the document


	for (auto&& doc : cursor) {
		e = doc["item"];
		customer_order.item = e.get_utf8().value.to_string();
		e = doc["price"];
		customer_order.price = e.get_utf8().value.to_string();
		//delete the document
		mucoll.delete_one(doc);
		//break out the for loop so that we only remove one order
		break;
	}

	//from here we will have to parse the information using strtok.
	//store the info using 2-d vector with each row being the attributes of each item
	//of each row:
	//index 0 is the item name
	//index 1 is the item price
	vector<vector<string>> parsed_items;

	//initialization
	char* ptr_item;
	ptr_item = strtok(customer_order.item);
	parsed_items[0][0] = ptr;
	char* ptr_price;
	ptr_price = strtok(customer_order.price);
	parsed_items[0][1];
	//keep track of each item pushed to vector
	int count_item = 1;

	while (ptr_item != NULL) {
		ptr_item = strtok(customer_order.items);
		parsed_items[count_item][0];
		ptr_price = strtok(customer_order.price);
		parsed_items[count_item][1];
		count_item++;
	}

	return parsed_items;

}