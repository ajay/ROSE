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

void dbsim() {
	mongocxx::instance inst{};					
	mongocxx::client conn{mongocxx::uri{}}; 	
	auto db = conn["rosedb"];
	auto sim = db["orders"];

	string sim_item = "";
	string sim_price = "";

	int rand_quant;
	int rand_can;

	srand(time(NULL));

	for (int i = 0; i < 10; ++i) {
		rand_quant = rand() % 3 + 1;

		for (int j = 0; j < rand_quant; ++j) {

			rand_can = rand() % 3 + 1;
			if (rand_can == 1) {
				sim_item.append("coke");
			} else if (rand_can == 2) {
				sim_item.append("sprite");
			} else {
				sim_item.append("fanta");
			}
			sim_price.append("1.99");
			if (j != rand_quant - 1) {
				sim_item.append(",");
				sim_price.append(",");
			}
		}
		bsoncxx::document::value sim_order = document{} << "items" << sim_item << "prices" << sim_price << "table" << "1" << finalize;
		sim.insert_one(move(sim_order));
		sim_item = "";
		sim_price = "";
	}
}

int main() {
	dbconn rose_db;
	dbsim();
	rose_db.db_update();
}