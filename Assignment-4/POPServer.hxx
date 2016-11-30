#ifndef __POPSERVERHXX__
#define __POPSERVERHXX__

#include "TCPServer.hxx"
#include "POPServerSession.hxx"


class POPServer: public TCPServer
{
	string domain;
public:
	POPServer(string ipaddress,int port,string domain);
	int runForever();
	void handleClient(int cfd);
};

#endif