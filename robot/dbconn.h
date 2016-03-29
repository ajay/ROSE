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

#ifndef DBCONN_H
#define DBCONN_H

void read_state(mongocxx::v_noabi::database db);
//commenting out until webapp can push speed
//void read_speed(mongocxx::v_noabi::database db);

#endif