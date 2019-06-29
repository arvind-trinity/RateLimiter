#include "DataStoreHandler.h"

#include <iostream>

using namespace std;

#define REDIS_PORT 6379
#define REDIS_HOST "127.0.0.1"

DataStoreHandler::DataStoreHandler() {
  struct timeval timeout = { 1, 500000 }; // 1.5 seconds

  mContext = redisConnectWithTimeout(REDIS_HOST, REDIS_PORT, timeout);
  if ((mContext == NULL) || (mContext->err)) {
    if (mContext) {
      cout << "Redis Connection error " << mContext->errstr << endl;
      redisFree(mContext);
    } else {
      cout << "Connection error can't allocate redis context" << endl;
    }
  }
}

DataStoreHandler::~DataStoreHandler() {
  if (mContext) {
    redisFree(mContext);
    mContext = NULL;
  }
}

int DataStoreHandler::incRequestCount(const string &aResourceId, int count) {
  int ret = -1;

  if (mContext) {
    redisReply *reply;
    string command("hincrby " + aResourceId + " currCount " + to_string(count));

    reply = (redisReply*) redisCommand(mContext, command.c_str());
    ret = reply->integer;
    freeReplyObject(reply);
  }

  return ret;
}

int DataStoreHandler::decRequestCount(const string &aResourceId) {
  return incRequestCount(aResourceId, -1);
}

bool DataStoreHandler::addLimitData(const string &aResourceId, const strKVMap &aKVMap) {
  bool ret = false;

  if (mContext) {
    redisReply *reply;

    // get the kv map into space separated string
    string kvString;
    for (auto itr = aKVMap.begin(); itr != aKVMap.end(); itr++) {
      kvString += " " + itr->first + " " + itr->second;
    }

    string command("hmset " + aResourceId + kvString);
    reply = (redisReply*) redisCommand(mContext, command.c_str());
    if (reply->type != REDIS_REPLY_ERROR) {
      ret = true;
    } else {
      cout << "adding limit data failed" << endl;
    }
  }

  return ret;
}

unordered_map<string, string> 
DataStoreHandler::getLimitData(const string &aResourceId) {
  unordered_map<string, string> ret;

  if (mContext) {
    redisReply *reply;
    string command("hgetall " + aResourceId);

    reply = (redisReply*) redisCommand(mContext, command.c_str());

    if (reply->type == REDIS_REPLY_ARRAY) {
      for (int i = 0; i < reply->elements; i = i + 2) {
        ret[reply->element[i]->str] = reply->element[i + 1]->str;
        //cout << "key: " << reply->element[i]->str;
        //cout << " val: " << reply->element[i + 1]->str << endl;
      }
    } else {
      cout << "redis reply data type invalid" << endl;
    }
    freeReplyObject(reply);
  }
  return ret;
}

bool DataStoreHandler::testConnection() {
	bool ret = false;

  if (mContext) {
    redisReply *reply;
    reply = (redisReply*) redisCommand(mContext, "PING");
    if (reply->type != REDIS_REPLY_ERROR) {
			cout << "PING: " << reply->str << endl;
			ret = true;
		} else {
			cout << "Connection to Redis faild" << endl;
		}
    freeReplyObject(reply);
  }
	return ret;
}

void DataStoreHandler::DumpData(strKVMap &map) {
	strKVMap::iterator i = map.begin();
	while (i != map.end()) {
		cout << "key: " << i->first;
		cout << " val: " << i->second << endl;
		i++;
	}
}

