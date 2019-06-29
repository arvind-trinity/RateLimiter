#include "RateLimiter.h"
#include "DataStoreHandler.h"

#include <iostream>
#include <unistd.h>

#define LIMIT_WINDOW 60 // secs
#define COUNT_PER_WINDOW 100
#define RATE_PER_SEC 10
#define RUN_TIME_IN_SECS 300

int main() {
	cout << "Running a 5 mins load, ";
	cout << "with load of 10 rps, ";
	cout << "window size: 60 secs and ";
	cout << "rate limit of 100 per min" << endl;
	cout << "Press any key to continue..." << endl;
	cin.get();

  RateLimiter rl(LIMIT_WINDOW);
  DataStoreHandler dh;
  rl.addResourceLimit(string("css"), COUNT_PER_WINDOW);

  cout << "rateLimit for css: " << rl.getRateLimit("css") << endl;

  int totalCount = 0;
  time_t startTime = time(NULL);
  int allowedCount = 0;
  while ((startTime + RUN_TIME_IN_SECS) > time(NULL)) {
    if (rl.isRequestAllowed("css")) {
      ++allowedCount;
    }
    ++totalCount;

    cout << "elapsed secs: " << time(NULL) - startTime + 1;
    cout << " total Requests: " << totalCount;
		cout << " allowed Requests: " << allowedCount << endl;
    dh.getLimitData("css");
    usleep(1000000 / RATE_PER_SEC);
  }

  return 0;
}
