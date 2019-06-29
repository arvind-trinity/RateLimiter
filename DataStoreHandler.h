#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <unordered_map>
#include <string>
#include <hiredis.h>

using namespace std;

typedef unordered_map<string, string> strKVMap;

class DataStoreHandler {
  public:
  // constructor
  DataStoreHandler();
  //destructor
  ~DataStoreHandler();
  // increments request count for the given resource
  virtual int incRequestCount(const string &aResourceId, int count = 1);
  // decrement request count for the given resource 
  virtual int decRequestCount(const string &aResourceId);
  // adds limit data for the given resource id
  virtual bool addLimitData(const string &aResourceId, const strKVMap &aKVMap);
  // gets limit data for the given resource id
  virtual strKVMap getLimitData(const string &aResourceId);
  // function to test data store connection
  void testConnection();
  // gets a dump of data limits from data store
  // can be used to sync local data with the data store
  //virtual ResourceLimitData getLimitsData();

  private:
  redisContext *mContext;
};

#endif
