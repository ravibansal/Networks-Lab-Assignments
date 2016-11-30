#ifndef __POPSERVERSESSIONHXX__
#define __POPSERVERSESIONHXX__

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <errno.h>
#include "sql.h"

using namespace std;

#define AUTHORIZATION 1
#define TRANSACTION 2
#define UPDATE 3
#define ERR 0
#define OK 1

class POPServerSession
{
	static const int BUFFSIZE = 512;
	int socketfd;
	int state;
	string hostname;
	string msg;
	string username;
	sql::Connection* conn;
	int userid;
	vector< pair<int,double> > msglist;
	vector<int> to_delete;
	set<int> read;

public:
	POPServerSession(int socketfd,string hostname);
	~POPServerSession();
	int sendResponse(int type, string message="");
	int processCMD(string buf);

protected:
	int processUSER(string buf);
	int processPASS(string buf);
	int processLIST(string buf);
	int processSTAT(string buf);
	int processRETR(string buf);
	int processQUIT(string buf);
	int processDELE(string buf);

private:
	int sendMessage(string msg, int status=-1);
};


#endif

