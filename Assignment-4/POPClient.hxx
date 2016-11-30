#ifndef __POPCLIENTHXX__
#define __POPCLIENTHXX__

#include "TCPClient.hxx"

void retrieveMails(string ip, int port, string domain, string username);
class POPClient : public TCPClient
{
protected:
	string domain;
public:
	POPClient(string ipaddress,int port,string domain);

	int popsend(string message,string command);
	string popreceive(int& status);

	void sendLIST(int unread=0);
	void sendRETR(int id);
	void sendDELE(int id);
	void sendSTAT();
	void sendQUIT();

};

#endif