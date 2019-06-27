#include "RateLimiter.h"

#include <iostream>
#include <unistd.h>

int main() {
  cout << "Testing RateLimiter" << endl;

  RateLimiter rl(10);
  rl.addResourceLimit(string("css"), 5);

  cout << rl.getRateLimit("css") << endl;

  int count = 0;
  while (true) {
    bool isAllowed = rl.isRequestAllowed("css");
    cout << count <<"th second " << ++count << " request, isAllowed?: " << isAllowed << endl;
    usleep(500000);
  }

  return 0;
}
