#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <string>
#include <unordered_map>
#include <time.h>

#include "DataStoreHandler.h"

using namespace std;

#define WINDOW_SIZE 60 // secs

struct ResourceLimitData {
  time_t windowTimeStamp;
  int currCount;
  int prevCount;
  int limitPerWindowSize;
};

typedef unordered_map<string, ResourceLimitData> LimitDataMap;

class RateLimiter {
  public:
  // public interface to register a resource for rate limiting
  virtual bool addResourceLimit(const string &aResourceId, const int &aLimitPerMinute);
  // public interface to check if rate limit kicked in for the current resource
  virtual bool isRequestAllowed(const string &aResourceId);
  // public interface to get ratelimit applied for the given resource
  virtual int getRateLimit(const string &aResourceId);
  // contructor
  RateLimiter(int aWindowSize = WINDOW_SIZE);

  private:
  // get the current request count for the given resource
  // and optionally the previous updated timestamp
  int getCount(const string &aResourceId, time_t *aTime = NULL);
  // sync resource limit data in-memory from store
  bool updateResourceLimitDataFromStore(const string &aResourceId);
  // sync resource limit data in store with in-memory data
  bool updateResourceLimitDataToStore(const string &aResourceId);
  // interface to get current time used by all members
  time_t getCurrentTime();
  // data dumper for debugging
  void DumpData();

  // member data
  LimitDataMap mLimitMap; // map to store resorce vs limit data
  int mWindowSize; // configurable window size
  DataStoreHandler mDataStore; // handles storage requests
};

#endif

