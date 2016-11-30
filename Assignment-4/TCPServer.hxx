#ifndef __TCPSERVERHXX__
#define __TCPSERVERHXX__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <errno.h>

using namespace std;
class TCPServer
{
	static const int BUFFSIZE = 512;

	static const int SOCK_BACKLOG =  10;
protected:
	string ipaddress;
	int port;
	int sfd;

public:
	TCPServer(string ipaddress, int port);
	int connect();
	int tcpsend(int clientfd, string message);
	string receive(int clientfd, int& size);

};

#endif
