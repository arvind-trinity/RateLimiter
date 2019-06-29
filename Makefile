CC=g++
RM=rm -f
CPPFLAGS=-I. -I/usr/include/hiredis/
LDLIBS=-lhiredis
APP=test

${APP}: test.o RateLimiter.o DataStoreHandler.o

test.o: test.cpp

RateLimiter.o: RateLimiter.cpp RateLimiter.h

DataStoreHandler.o: DataStoreHandler.cpp DataStoreHandler.h

all: ${APP}

clean:
	${RM} *.o ${APP}

.PHONY: all clean

