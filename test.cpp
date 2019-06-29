#include "RateLimiter.h"
#include "DataStoreHandler.h"

#include <iostream>
#include <unistd.h>

#define LIMIT_WINDOW 60 // secs
#define COUNT_PER_WINDOW 100
#define RATE_PER_SEC 10
#define RUN_TIME_IN_SECS 3600

int main() {
  cout << "Testing RateLimiter" << endl;

  RateLimiter rl(LIMIT_WINDOW);
  DataStoreHandler dh;
  rl.addResourceLimit(string("css"), COUNT_PER_WINDOW);

  cout << "rateLimit for css: " << rl.getRateLimit("css") << endl;

  int totalCount = 0;
  time_t startTime = time(NULL);
  int allowedCount = 0;
  while ((startTime + RUN_TIME_IN_SECS) >= time(NULL)) {
    if (rl.isRequestAllowed("css")) {
      ++allowedCount;
    }
    ++totalCount;
    cout << "elapsed secs: " << time(NULL) - startTime;
    cout << " total: " << totalCount << " allowed: " << allowedCount << endl;
    dh.getLimitData("css");
    usleep(1000000 / RATE_PER_SEC);
  }

#if 0
    cout << "testing data handler" << endl;

    DataStoreHandler dh;
    dh.getLimitData("css");
    dh.testConnection();
    dh.decRequestCount("css");
    dh.addLimitData("abcd", {{"a", "b"}, {"c", "d"}});
    dh.getLimitData("abcd");
#endif

  return 0;
}
