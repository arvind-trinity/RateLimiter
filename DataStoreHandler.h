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
	// TODO: delete the limit data
	//virtual bool delLimitData(const string &aResourceId);
  // function to test data store connection
  bool testConnection();
	// dump the given strKVMap
  virtual void DumpData(strKVMap &map);

  private:
  redisContext *mContext;
};

#endif
