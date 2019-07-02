#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "RateLimiter.h"

// class to handle incomming connections
class ConnectionHandler {
	public:
	ConnectionHandler(int aPort);
	~ConnectionHandler();
	// runs continuously and handles connecitons
	void handleConnections();

	private:
	// creates a server socket and binds to given port
	bool createAndBindServerSocket();

	int mSocketFd;
	int mPort;
	RateLimiter *mRateLimiter;
};
#endif
