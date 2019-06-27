CC=g++
CFLAGS=-I.

RateLimiter: test.o RateLimiter.o

test.o: test.cpp

RateLimiter.o: RateLimiter.cpp

.PHONY: all clean

all: RateLimiter

