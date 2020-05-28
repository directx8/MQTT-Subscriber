#ifndef MYMOSQ_H
#define MYMOSQ_H

#include <mosquittopp.h>
#include <cstring>

using namespace std;

class myMosq : public mosqpp::mosquittopp
{
private:
   const char *host;
   const char *id;
   const char *topic;
   int port;
   int keepalive;

   void on_connect(int rc);
   void on_disconnect(int rc);
   void on_subscribe(int mid);
   void on_message(const mosquitto_message *message);

public:
   myMosq(const char *_id, const char *_topic, const char *host, int port);
   ~myMosq();
   bool receive_message(const char *_message);
};

#endif
