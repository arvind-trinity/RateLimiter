#include "ConnectionHandler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <iostream>

using namespace std;

ConnectionHandler::ConnectionHandler(int aPort) : 
	mPort(aPort), mRateLimiter(NULL) {
	createAndBindServerSocket();
	// creates rate limiter with window size 60 secs
	mRateLimiter = RateLimiter::make();
}

ConnectionHandler::~ConnectionHandler() {
	if (mRateLimiter) {
		RateLimiter::destroy();
	}
}

bool ConnectionHandler::createAndBindServerSocket() {
	struct sockaddr_in address;
	bool ret = true;
	int opt = 1;

	// create an inet socket
	if ((mSocketFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		cout << "socket creation failed" << endl;
		ret = false;
	}

	// force re-use of address and port
	if (ret && (setsockopt(
					mSocketFd, 
					SOL_SOCKET, 
					SO_REUSEADDR | SO_REUSEPORT,
					&opt, 
					sizeof(opt)))) {
		cout << "socket force bind failed" << endl;
		ret = false;
	}

	// bind socket to an address and port
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( mPort );
	if (ret && (bind(mSocketFd,
					(struct sockaddr*)&address,
					sizeof(address)) < 0)) {
		cout << "socket bind failed" << endl;
		ret = false;
	}

	return ret;
}

void ConnectionHandler::handleConnections() {
	int backlog = 3;
	int newSocket = -1;
	bool ret = true;

	if (listen(mSocketFd, backlog) < 0) {
		cout << "listen failed" << endl;
		ret = false;
	}

	if (ret)
	{
		while (true) {
			cout << "waiting for connection on port: " << mPort << endl;
			struct sockaddr_in address;
			int addrlen = sizeof(address);
			int msgLen = 0;
			char *msg = NULL;

			// accept new connection
			if ((newSocket = accept(mSocketFd,
							(struct sockaddr*) &address,
							(socklen_t*) &addrlen)) < 0) {
				cout << "accept failed" << endl;
				break;
			}

			// read the message length first
			while (recv(newSocket, &msgLen, sizeof(int), 0) > 0) {
				msg = new char[msgLen + 1];

				// read the msg
				recv(newSocket, msg, msgLen, 0);
				msg[msgLen] = '\0';
				cout << "Server received msg: " << msg << endl;

				// try to add the resource to limits table
				// call will fail if the resource is already
				// in table, ignore the failure
				mRateLimiter->addResourceLimit(msg, 100);
				int resp = (int) mRateLimiter->isRequestAllowed(msg);
				delete [] msg;

				// send response
				cout << "Server response: " << resp << endl;
				send(newSocket, &resp, sizeof(resp), 0);
			}
		}
	}
}

