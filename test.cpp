#include "RateLimiter.h"

#include <iostream>
#include <unistd.h>

#define LIMIT_WINDOW 60 // secs
#define COUNT_PER_WINDOW 100
#define RATE_PER_SEC 10

int main() {
  cout << "Testing RateLimiter" << endl;

  RateLimiter rl(LIMIT_WINDOW);
  rl.addResourceLimit(string("css"), COUNT_PER_WINDOW);

  cout << rl.getRateLimit("css") << endl;

  int totalCount = 0;
  time_t startTime = time(NULL);
  int allowedCount = 0;
  while (true) {
    if (rl.isRequestAllowed("css")) {
      ++allowedCount;
    }
    ++totalCount;
    cout << "elapsed secs: " << time(NULL) - startTime;
    cout << " total: " << totalCount << " allowed: " << allowedCount << endl;
    usleep(1000000 / RATE_PER_SEC);
  }

  return 0;
}
