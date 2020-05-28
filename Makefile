IDIR=usr/include
#ARCH?=arm
CC=g++
CFLAGS=-I$(IDIR)

LDEPS = -lpthread -lmosquittopp -lmosquitto -lc -lboost_filesystem

DEPS = myMosq.h 
OBJ = MQTT_Subscriber.o myMosq.o 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

MQTT: $(OBJ)
	$(CC) -o $@ $^ $(LDEPS) $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o MQTT

