#ifndef __TCPCLIENTHXX__
#define __TCPCLIENTHXX__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <iostream>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <errno.h>

using namespace std;
class TCPClient
{
	static const int BUFFSIZE = 512;
protected:
	string ipaddress;
	int port;
	int sfd;

public:
	TCPClient(string ipaddress, int port);
	int _connect();
	int tcpsend(string message);
	string receive(int& size);

};

#endif
