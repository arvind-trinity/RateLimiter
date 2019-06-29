CC=g++
RM=rm -f
CPPFLAGS=-I. -I/usr/include/hiredis/
LDLIBS=-lhiredis
APP_TEST=test
APP_TEST_RUN=test_run

${APP_TEST}: test.o RateLimiter.o DataStoreHandler.o

${APP_TEST_RUN}: test_run.o RateLimiter.o DataStoreHandler.o

test_run.o: test_run.cpp

test.o: test.cpp

RateLimiter.o: RateLimiter.cpp RateLimiter.h

DataStoreHandler.o: DataStoreHandler.cpp DataStoreHandler.h

test_func: ${APP_TEST}
	./test

test_load: ${APP_TEST_RUN}
	./test_run

all: ${APP_TEST} ${APP_TEST_RUN}

clean:
	${RM} *.o ${APP_TEST} ${APP_TEST_RUN}

.PHONY: all clean

