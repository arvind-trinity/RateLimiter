#include "RateLimiter.h"

#include <iostream>

using namespace std;

RateLimiter::RateLimiter(int aWindowSize) : mWindowSize(aWindowSize) {}

bool RateLimiter::addResourceLimit(const string &aResourceId, const int &aLimitPerWindowSize) {
  // check if the resource is already rate limited
  if (aResourceId.empty() || 
    aLimitPerWindowSize < 0 ||
    mLimitMap.count(aResourceId)) {
    // TODO: add an update interface
    return false;
  }

  // create a new entry in limit data map
  ResourceLimitData lData;
  lData.limitPerWindowSize = aLimitPerWindowSize;
  lData.currCount = 0;
  lData.prevCount = 0;
  lData.windowTimeStamp = 0;
  mLimitMap[aResourceId] = lData;

  // add to data store
  updateResourceLimitDataToStore(aResourceId);

  return true;
}

// core function that performs most of the task.
bool RateLimiter::isRequestAllowed(const string &aResourceId) {
  bool ret = false;
  time_t currTime = getCurrentTime();
  bool isSyncNeeded = false;

  // get the limit record for the resource
  LimitDataMap::iterator itr = mLimitMap.end();
  if ((itr = mLimitMap.find(aResourceId)) != mLimitMap.end()) {
    // check if we are within the current window
    ResourceLimitData &lData = itr->second;
    int timeDiff = currTime - lData.windowTimeStamp; 

    if (timeDiff >= mWindowSize) {
      // we are outside our current window so re-confiure the counts
      if (timeDiff > (2 * mWindowSize)) {
        // count is currCount for a bucket older than 2 times
        // window time so discard it
        lData.prevCount = 0;
      } else {
        // count in current window is within previous window time
        // so store a scaled version of it in prevCount
        int newDiff = (2 * mWindowSize) - timeDiff;
        lData.prevCount = lData.currCount * newDiff / mWindowSize; 
      }
      // as we are starting a new window update the current window 
      // timestamp
      lData.windowTimeStamp = currTime;
      lData.currCount = 0;
      isSyncNeeded = true;
    }

    // TODO: check if the count should be increased even 
    // we are outside the limit? this will answer the question
    // should we allow 100 requests every minute irrespective
    // of how many requests are made
    if (getCount(aResourceId) < lData.limitPerWindowSize) {
      ret = true;
      ++lData.currCount;
      if (!isSyncNeeded) {
        // just increment the count
        mDataStore.incRequestCount(aResourceId);
      }
    }
  }

  if (isSyncNeeded) {
    updateResourceLimitDataToStore(aResourceId);
  }

  //DumpData();
  return ret;
}

int RateLimiter::getCount(const string &aResourceId, time_t *aTime) {
  int ret = -1;
  LimitDataMap::iterator itr = mLimitMap.end();
  time_t currTime = getCurrentTime();

  if ((itr = mLimitMap.find(aResourceId)) != mLimitMap.end()) {
    ResourceLimitData &lData = itr->second;
    int timeDiff = currTime - lData.windowTimeStamp; 
    ret = lData.currCount + ((mWindowSize - timeDiff - 1) * lData.prevCount / mWindowSize);
    if (aTime) {
      *aTime = lData.windowTimeStamp;
    }
  }

  return ret;
}

int RateLimiter::getRateLimit(const string &aResourceId) {
  int ret = -1;

  LimitDataMap::iterator itr = mLimitMap.end();
  if ((itr = mLimitMap.find(aResourceId)) != mLimitMap.end()) {
    ret = itr->second.limitPerWindowSize;
  }

  return ret;
}

bool RateLimiter::updateResourceLimitDataFromStore(const string &aResourceId) {
  bool ret = false;
  strKVMap map = mDataStore.getLimitData(aResourceId);
  ResourceLimitData data;

  // check if we already have limit data in-memory for this 
  // resource
  if (mLimitMap.find(aResourceId) != mLimitMap.end()) {
    ResourceLimitData data = mLimitMap[aResourceId];
    ret = true;
    // TODO: check if the data is in data store if not
    // add it
  }

  // add/update the in-memory limit data
  for (strKVMap::iterator i; i != map.end(); i++) {
    data.windowTimeStamp = stoi(map["windowTimeStamp"]);
    data.currCount = stoi(map["currCount"]);
    data.prevCount = stoi(map["prevCount"]);
    data.limitPerWindowSize = stoi(map["limitPerWindowSize"]);
    mLimitMap[aResourceId] = data;
  }

  return ret;
}

bool RateLimiter::updateResourceLimitDataToStore(const string &aResourceId) {
  bool ret = false;
  ResourceLimitData data;
  LimitDataMap::iterator itr;

  if ((itr = mLimitMap.find(aResourceId)) != mLimitMap.end()) {
    data = itr->second;
    // add to data store
    strKVMap map = { 
      {"windowTimeStamp", to_string(data.windowTimeStamp)},
      {"currCount", to_string(data.currCount)},
      {"prevCount", to_string(data.prevCount)},
      {"limitPerWindowSize", to_string(data.limitPerWindowSize)} 
    };
    mDataStore.addLimitData(aResourceId, map);
  }

  return ret;
}

time_t RateLimiter::getCurrentTime() {
  return time(NULL);
}

void RateLimiter::DumpData() {
  cout << "##### dumping data #####" << endl;
  cout << "window size: " << mWindowSize << endl;

  for (auto itr = mLimitMap.begin(); itr != mLimitMap.end(); ++itr) {
    cout << "resource id: " << itr->first << endl;
    cout << "\twindowTimeStamp: " << itr->second.windowTimeStamp << endl;
    cout << "\tcurrCount: " << itr->second.currCount << endl;
    cout << "\tprevCount: " << itr->second.prevCount << endl;
    cout << "\tlimitPerWindowSize: " << itr->second.limitPerWindowSize << endl;
  }

  cout << "########################\n" << endl;
}
