#include <iostream>
#include <map>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <ctime>
#include <string>
#include <stdlib.h>

using namespace std;

#define MAX_NUMBER_SENSORS 10

// Define the priority flags and map the labels
enum Priorities {EMERGENCY=0, ALERT=1, CRITICAL=2, ERROR=3, WARNING=4, INFO=5, NONE=6};
ostream& operator<<(ostream& out, const Priorities value)
{
    static map<Priorities, string> strings;
    if (strings.size() == 0){
        #define INSERT_ELEMENT(p) strings[p] = #p
        INSERT_ELEMENT(EMERGENCY);     
        INSERT_ELEMENT(ALERT);     
        INSERT_ELEMENT(CRITICAL); 
        INSERT_ELEMENT(ERROR);
        INSERT_ELEMENT(WARNING);
        INSERT_ELEMENT(INFO);
        INSERT_ELEMENT(NONE);            
        #undef INSERT_ELEMENT
    }   
    return out << strings[value];
}

int* count = new int[MAX_NUMBER_SENSORS]; // Counter for each sensor
string* id = new string[MAX_NUMBER_SENSORS]; // ID for each sensor

class Logger
{
private:
    string getTime();
    Priorities priorities();
    string action(Priorities prt);

    // Parameters
    string message;
    string degrees;
    string serialID;
    int size;

public: 
    void log_write();
    Logger(int _size, string _message, string _degrees, string _sensorID);
    Logger(string _message, int _size);
    ~Logger();
};

// Normal constructor
Logger::Logger(int _size, string _message, string _degrees, string _sensorID)
{
    // Basic configuration setup
    this->message = _message;
    this->degrees = _degrees;
    this->serialID = _sensorID;
    this->size = _size;
}

// Constructor for no sensor found
Logger::Logger(string _message, int _size)
{
    // Basic configuration setup
    this->message = _message;
    this->serialID = "No Sensor";
    this->size = _size;
}

// Deconstructor
Logger::~Logger(){ }

void Logger::log_write()
{
    string time = getTime();
    ofstream file("log.log", ofstream::out | ofstream::app);
    Priorities priority = priorities();

    // Get the current time and limit the size 
    time.resize(24);

    // Check if ID is filled into the list of sensors & init counter
    // for each sensor
    for(int i = 0; i < this->size; i++)
    {
        if ((id[i] != this->serialID) && id[i].empty()) 
        {
            id[i] = this->serialID;
            count[i] = 0;

            break;
        }
    }

    // DO all of the operations for all sensors
    for(int i = 0; i < this->size; i++)
    {
        if (id[i] == this->serialID)
        {
            count[i]++;

            if(file.is_open())
            {   
                // If 60 seconds of normal temp behaviour have passed log the temperature value
                if (count[i] == 30 && priority == INFO)
                {
                    file << time << " : " << priority << " : " << this->degrees << " Degrees C : serialID : " << this->serialID << endl;
                    file.close();

                    cout << this->serialID << endl;

                    // Reset counter
                    count[i] = 0;
                }
                // If there is a FLAG raised then log the info immediatelly
                else if(priority != INFO && priority != ERROR)
                {
                    file << time << " : " << priority << " : " << this->degrees << " Degrees C : serialID : " << this->serialID << endl;
                    file.close();

                    // Take needed action
                    action(priority);

                    count[i] = 0;
                }
                else if(priority == ERROR)
                {
                    file << time << " : " << priority << " : " << this->message << endl;
                    file.close();
                    
                    // Take needed action
                    action(priority);

                    count[i] = 0;
                } 
            }
        }
    }
}

// Assignes priorities based on temp readings
Priorities Logger::priorities()
{
    if(this->message == "NSF") { return ERROR; }
    else
    {     
        double value = stod(this->degrees);

        if(value <= (double)40) { return INFO; }
        if((value > (double)40) && (value <= (double)50)) { return WARNING; }
        if((value > (double)50) && (value <= (double)60)) { return CRITICAL; }
        if((value > (double)60) && (value <= (double)90)) { return ALERT; }
        if(value > (double)90) { return EMERGENCY; }
    }

    return NONE;
}
// What needs to be done in case of one of these flags is triggered
string Logger::action(Priorities prt)
{
    switch(prt)
    {
        case INFO :
            break;
        case ALERT :
            break;
        case WARNING : 
            break;
        case CRITICAL :
            break;
        case EMERGENCY :
            system("shutdown -P now"); 
            break;
        case ERROR :
            break;
        default:
            break;
    }
}

// Gives the current time,date,day,etc.
string Logger::getTime()
    {
        // current date/time based on current system
        time_t now = time(0);
        
        // convert now to string form
        char* dt = ctime(&now);

        return dt;
    }