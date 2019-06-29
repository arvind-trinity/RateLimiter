#include "RateLimiter.h"
#include "DataStoreHandler.h"

#include <iostream>
#include <unistd.h>

#define LIMIT_WINDOW 5 // secs
#define COUNT_PER_WINDOW 2
#define RATE_PER_SEC 1
#define RUN_TIME_IN_SECS 10

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

	cout << "#### Testing RateLimiter ####" << endl;
	cout << "Running a 10 sec load ";
	cout << "with load of 1 rps, window: 5 and ";
	cout << "allowed rate: 2/window" << endl;

	RateLimiter rl(LIMIT_WINDOW);
	rl.addResourceLimit(string(resourceId), COUNT_PER_WINDOW);

	cout << "rateLimit for css: " << rl.getRateLimit(resourceId) << endl;

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
		usleep(1000000 / RATE_PER_SEC);
	}

	return 0;
}
