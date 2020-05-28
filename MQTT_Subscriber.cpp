#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/exception/all.hpp>

#include "myMosq.h"

using namespace std;
namespace pt = boost::property_tree;

string IP_ADDRESS;
string ID;
string TOPIC;
int PORT;

void init_config();

int main()
{
    init_config();

    char message_receive[500];

    myMosq *message = new myMosq(ID.c_str(), TOPIC.c_str(), IP_ADDRESS.c_str(), PORT);
    int rc;

    message->receive_message(message_receive);

    while (true)
    {
        rc = message->loop();

        if (rc)
        {
            message->reconnect();
        }
        message->~myMosq();
    }

    return 0;
}

// Init the configurations from the JSON file
void init_config()
{
    try
    {
        // Init the JSON file reader
        pt::ptree root;
        pt::read_json("config.json", root);

        // Parse the JSON config parameters
        IP_ADDRESS = root.get<string>("IP_ADDRESS");
        PORT = root.get<int>("PORT");
        ID = root.get<string>("ID");
        TOPIC = root.get<string>("TOPIC");
    }
    catch (std::exception const &e)
    {
        // Get the diagnostic info from the exception
        string diag = e.what();

        // current date/time based on current system
        time_t now = time(0);
        // convert now to string form
        char *dt = ctime(&now);
        string time = dt;
        time.resize(24);

        // Open the log file
        ofstream file("log.log", ofstream::out | ofstream::app);

        cout << "Can't init settings!" << endl;
        cout << "Initializing default settings!" << endl;

        // Init default settings
        IP_ADDRESS = "192.168.100.222";
        PORT = 10030;
        ID = "build_server";
        TOPIC = "test";

        if (file.is_open())
        {
            // Log the diagnostics from the exception
            file << time << " : " << diag << endl;
        }
    }
}
