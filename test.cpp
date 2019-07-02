#include "RateLimiter.h"
#include "DataStoreHandler.h"
#include "ConnectionHandler.h"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

#define PORT 8080

#define LIMIT_WINDOW 5 // secs
#define COUNT_PER_WINDOW 2
#define RATE_PER_SEC 1
#define RUN_TIME_IN_SECS 10

#define ENABLE_RATE_LIMITER_TESTING true
#define ENABLE_CONNECTION_TESTING true

using namespace std;

bool run_client() {
	cout << "Starting Client..." << endl;

	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	string resourceId = "cooking.com";
	int len = resourceId.size();
	int resp = -1;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "socket connection error" << endl;
		return false;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return false;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return false;
	}

	// send messages in loop
	int count = 5;
	cout << "Running client for " << count << " secs" << endl;
	while (count--) {
		send(sock, &len, sizeof(len), 0 );
		send(sock, resourceId.c_str(), len, 0 );
		read(sock, &resp, sizeof(int));
		cout << "Client isRequestAllowed for " << resourceId;
		cout << ": " << resp << endl;
		sleep(1);
	}
	return true;
}

void testConnectionHandler() {
	cout << "Testing Connection Handler" << endl;
	ConnectionHandler ch(8080);
	ch.handleConnections();
}

int main() {
	cout << "#### Testing Data Store Handler ####" << endl;

	DataStoreHandler dh;
	strKVMap map = {
		{ "windowTimeStamp", to_string(time(NULL)) },
		{ "currCount", to_string(5) },
		{ "prevCount", to_string(0) },
		{ "limitPerWindowSize" , to_string(100) }
	};
	string resourceId = "cooking.com";
	string testResult;

	testResult = dh.testConnection()? "PASSED" : "FAILED";
	cout << "Data store connection test: " << testResult << endl;

	testResult = dh.addLimitData(resourceId, map)? "PASSED" : "FAILED";
	cout << "Data store addLimitData test: " << testResult << endl;

	map.clear();
	map = dh.getLimitData(resourceId);
	testResult = map.size() == 4? "PASSED" : "FAILED";
	cout << "Data store getLimitData test: " << testResult << endl;

	testResult = dh.incRequestCount(resourceId) == 6? "PASSED" : "FAILED";
	cout << "Data store incRequestCount test: " << testResult << endl;

	testResult = dh.decRequestCount(resourceId) == 5? "PASSED" : "FAILED";
	cout << "Data store decRequestCount test: " << testResult << endl;

	//dh.delLimitData(resourceId);

	if (ENABLE_RATE_LIMITER_TESTING) {
		cout << "#### Testing RateLimiter ####" << endl;
		cout << "Running a 10 sec load ";
		cout << "with load of 1 rps, window: 5 and ";
		cout << "allowed rate: 2/window" << endl;

		RateLimiter *rateLimiter = RateLimiter::make(LIMIT_WINDOW);
		RateLimiter &rl = *rateLimiter;
		rl.addResourceLimit(string(resourceId), COUNT_PER_WINDOW);

		cout << "rateLimit for " << resourceId;
		cout << ": " << rl.getRateLimit(resourceId) << endl;

		int totalCount = 0;
		time_t startTime = time(NULL);
		int allowedCount = 0;
		while ((startTime + RUN_TIME_IN_SECS) > time(NULL)) {
			if (rl.isRequestAllowed(resourceId)) {
				++allowedCount;
			}
			++totalCount;

			cout << "elapsed secs: " << time(NULL) - startTime + 1;
			cout << " total Requests: " << totalCount;
			cout << " allowed Requests: " << allowedCount << endl;
			//rl.DumpData();
			//dh.DumpData(resourceId);

			usleep(1000000 / RATE_PER_SEC);
		}
		RateLimiter::destroy();
	}

	if (ENABLE_CONNECTION_TESTING) {
		cout << "##### Testing Connection Handler #####" << endl;
		int client, server;

		cout << "Running Server Process" << endl;
		if ((server = fork()) == 0) {
			testConnectionHandler();
			return 0;
		}
		usleep(250000);

		cout << "Running Clinet Process" << endl;
		if ((client = fork()) == 0) {
			run_client();
			return 0;
		}

		// wait for 7 secs and then kill
		sleep(7);
		kill(client, SIGKILL);
		kill(server, SIGKILL);
	}

	return 0;
}
