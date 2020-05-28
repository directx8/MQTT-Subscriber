#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

#include "myMosq.h"
#include "logging.h"
#include "lcdDisplay.h"
#include <mosquittopp.h>

using namespace std;

myMosq::myMosq(const char *_id, const char *_topic, const char *_host, int _port) : mosquittopp(_id)
{
    mosqpp::lib_init(); // initialization for mosquitto library

    this->keepalive = 60; // Basic configuration setup for myMosq class
    this->id = _id;
    this->port = _port;
    this->host = _host;
    this->topic = _topic;
    connect_async(host, // non blocking connection to broker request
                  port,
                  keepalive);
    loop_start(); // Start thread managing connection / publish / subscribe
};

myMosq::~myMosq()
{
    loop_stop();           // Kill the thread
    mosqpp::lib_cleanup(); // Mosquitto library cleanup
}

bool myMosq::receive_message(const char *message)
{
    int set = subscribe(NULL, this->topic, 1);

    return (set == MOSQ_ERR_SUCCESS);
}

void myMosq::on_message(const mosquitto_message *message)
{
    string tmp(reinterpret_cast<char *>(message->payload)); // Whole message

    lcdDisplay LCD = lcdDisplay(); //Init the LCD class

    if ((tmp.substr(0, tmp.find(" "))) == "NSF")
    {
        string NSF = tmp.substr(0, tmp.find(" ")); // Extract the no sensor message
        string nSensors = tmp.substr(4, 2);        //Extract the number of sensors

        LCD.ClearDisplay();
        LCD.sendStringToLCD(NSF, 0); // Send data to LCD

        // Init logger
        Logger log = Logger(NSF, stoi(nSensors));

        cout << "Subscriber " << id << " : "
             << "No Sensor Found!" << endl;

        // Log the temperature data in the log file
        log.log_write();
    }
    else
    {
        // Extract the sensor data from the readings
        string degrees = tmp.substr(0, 6);   // Extract the degrees
        string serialID = tmp.substr(10, 4); // Extract the ID
        string nSensors = tmp.substr(14, 2); // Extract the number of Sensors

        LCD.ClearDisplay();
        LCD.sendStringToLCD((degrees.substr(0, 5) + " C"), 0);
        LCD.sendStringToLCD(("ID " + serialID), 1);

        // Init the logger
        Logger log = Logger(stoi(nSensors), tmp, degrees, serialID);

        cout << "Subscriber " << id << " : " << degrees << " Degrees C and Serial ID: " << serialID << endl;

        // Log the temperature data in the log file
        log.log_write();
    }
}

void myMosq::on_disconnect(int rc)
{
    cout << ">> myMosq - disconnection(" << rc << ")" << endl;
}

void myMosq::on_connect(int rc)
{
    if (rc == 0)
    {
        cout << ">> myMosq - connected with server" << endl;
    }
    else
    {
        cout << ">> myMosq - Impossible to connect with server(" << rc << ")" << endl;
    }
}

void myMosq::on_subscribe(int mid)
{
    cout << ">> Subscription succeeded (" << mid << ") " << endl;
}
