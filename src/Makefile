CC = g++
CFLAGS = -Wall -O2 
CFLAGS += -I../inc -I../work/share -I../work/cjson
CFLAGS += -I../work/rtspperf/inc
LDFLAGS = -L../work/rtspperf/lib
LDFLAGS += -lplayer -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
LDFLAGS += -lpthread -lrt
SRC = main.cpp db.cpp
SRC += ../work/share/share.cpp
SRC += ../work/cjson/cJSON.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o mediaServer

clean:
	rm -f *.o mediaServer

