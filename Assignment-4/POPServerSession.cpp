#include "POPServerSession.hxx"

#include <locale>
POPServerSession::POPServerSession(int socketfd,string hostname): 
	socketfd(socketfd), state(AUTHORIZATION), hostname(hostname)
{
	conn = connect(hostname.c_str());
}

POPServerSession::~POPServerSession()
{
	delete conn;
}
int POPServerSession::processCMD(string buf)
{
	if(buf.size() < 4)
	{
		sendMessage("Invalid command",ERR);
		return 1;
	}
	cout<<"Received:"<<buf;
	string cmd = buf.substr(0, 4);
	int status;
	if (cmd == "USER")
		status =  processUSER(buf.substr(5, buf.size() - 7));
	else if (cmd == "PASS")
		status =  processPASS(buf.substr(5, buf.size() - 7));
	else if (cmd == "STAT")
		status =  processSTAT(buf.substr(5, buf.size() - 7));
	else if (cmd == "LIST")
		status =  processLIST(buf.substr(5, buf.size() - 7));
	else if (cmd == "RETR")
		status =  processRETR(buf.substr(5, buf.size() - 7));
	else if (cmd == "QUIT")
		status =  processQUIT(buf.substr(5, buf.size() - 7));
	else if (cmd == "DELE")
		status =  processDELE(buf.substr(5, buf.size() - 7));
	else
	{
		msg = "Invalid Command";
		status =  -1;
	}
	if (status == -1)
	{
		sendMessage(msg, ERR);
		return 1;
	}
	if(cmd == "QUIT")
	{
		return 0;
	}
	return 1;
}

int POPServerSession::processUSER(string buf)
{
	cout<<"Username "<<buf;
	if (state != AUTHORIZATION)
	{
		msg = "Invalid state and Command combination";
		return -1;
	}
	int uid = getuserid(conn,buf);
	if (uid <= 0)
	{
		msg = "User not known on server!! Make sure you specified the username correctly";
		return -1;
	}
	else
	{
		username = buf;
		userid = uid;
		msg = "Welcome " + buf + ". Please provide your password";
		sendMessage(msg, OK);
		return 0;
	}
}

int POPServerSession::processPASS(string buf)
{
	if (state != AUTHORIZATION || userid == 0)
	{
		msg = "Invalid command sequence";
		return -1;
	}
	if (verifyPassword(conn,buf,username) == 0)
	{
		msg = "Invalid password!! Note passwords are case-sensitive";
		return -1;
	}
	else
	{
		//TRY TO acquire lock
		state = TRANSACTION;
		//msgno = getMessagesNo(userid);
		//octetsize = getOctetSize(userid);
		//msg = username + "'s mailbox has " + to_string(msgno) + " messages( " + to_string(octetsize) + " octets )";
		msg = "Welcome to "+hostname+" mail server";
		sendMessage(msg, OK);
		return 0;
	}
}

int POPServerSession::processSTAT(string buf)
{
	if (state != TRANSACTION)
	{
		msg = "Invalid command sequence";
		return -1;
	}
	pair<int,double> stat = getStat(conn,username,hostname);
	msg = to_string(stat.first) + " " + to_string(stat.second) + "KB";
	sendMessage(msg, OK);
	return 0;
}

int POPServerSession::processLIST(string buf)
{
	if (state != TRANSACTION)
	{
		msg = "Invalid command sequence";
		return -1;
	}
	int msgid = 0;
	if(buf.size() > 0) msgid = stoi(buf);
	if (buf.size() == 0 || msgid == -1)
	{
		if(read.size() > 0)
		{
			markread(conn,read);
			read.clear();
		}
		if(to_delete.size() > 0)
		{
			deletemsg(conn,to_delete);
			to_delete.clear();
		}
		int unread = (msgid == -1) ? 1 : 0;
		msglist = getMsgList(conn, username, hostname,unread);
		pair<int,double> stat = msglist.back();
		{
			int n = 0;
			string tmp = "You have " + to_string(stat.first) + " messages ( "+ to_string(stat.second)+" KB )";
			sendMessage(tmp, OK);
			for (std::vector<pair<int, double> >::iterator i = msglist.begin(); i != msglist.end()-1; ++i)
			{
				tmp = to_string(++n) + " " + to_string(i->second)+ "KB";
				sendMessage(tmp);
				usleep(50*1000);
			}
			sendMessage(".");
			return 0;
		}
	}
	else
	{
		if(msgid >= (signed int)msglist.size() || msgid <= 0)
		{
			msg = "No such message";
			return -1;
		}
		pair<int,double> msgdet = msglist.at(msgid - 1);
		if (msgdet.second == -1)
		{
			msg = "Message already deleted";
			return -1;
		}
		else
		{
			string tmp = to_string(msgdet.first) + " " + to_string(msgdet.second) +" KB";
			sendMessage(tmp, OK);
			return 0;
		}
	}
	return 0;
}


int POPServerSession::processRETR(string buf)
{
	if (state != TRANSACTION)
	{
		msg = "Invalid command sequence";
		return -1;
	}
	if (buf.size() == 0)
	{
		msg = "Message Id is required";
		return -1;
	}
	else
	{
		int msgid = stoi(buf);
		if(msgid <= 0 || msgid >= (signed int)msglist.size())
		{
			msg = "No such message";
			return -1;
		}
		pair<int, double> mstat = msglist[msgid-1];
		if (mstat.second == -1)
		{
			msg = "error in retrieving. Message already deleted";
			return -1;
		}
		pair<string,string> message = getMessage(conn, mstat.first);
		read.insert(mstat.first);
		msg = to_string(mstat.second) + " KB";
		sendMessage(msg, OK);
		sendMessage("Mail From:"+message.first);
		char buf[message.second.size() + 1];
		strcpy(buf,message.second.c_str());
		char *line = strtok(buf, "\n");
		string extra = "Mail Body: ";
		while(line != NULL)
		{
			sendMessage(extra+line);
			extra = "";
			line = strtok(NULL,"\n");
			usleep(50*1000);
		}
		sendMessage(".");
		return 0;
	}
}

int POPServerSession::processDELE(string buf)
{
	if (state != TRANSACTION)
	{
		msg = "Invalid command sequence";
		return -1;
	}
	if (buf.size() == 0)
	{
		msg = "Message Id is required";
		return -1;
	}
	else
	{
		int msgid = stoi(buf);
		if(msgid <= 0 || msgid >= (signed int) msglist.size() )
		{
			msg = "No such message";
			return -1;
		}
		pair<int, double> mstat = msglist[msgid-1];
		if (mstat.second == -1)
		{
			msg = "error in retrieving. Message already deleted";
			return -1;
		}
		to_delete.push_back(mstat.first);
		msglist[msgid-1].second = -1;
		msg = "Deleted Successfully";
		sendMessage(msg, OK);
		return 0;
	}
}

int POPServerSession::processQUIT(string buf)
{
	if (state == TRANSACTION)
	{
		state = UPDATE;
		if(to_delete.size() > 0)
		{
			deletemsg(conn,to_delete);
		}
	}
	state = UPDATE;
	msg = "server signing off";
	sendMessage(msg, OK);
	return 0;
}

int POPServerSession::sendMessage(string message, int status)
{
	if (status == OK)
	{
		message = "+OK " + message;
	}
	else if(status == ERR)
	{
		message = "-ERR " + message;
	}
	message = message + "\n";
	cout<<"sending message :"<<message;
	int bytes = send(socketfd, message.c_str(), BUFFSIZE, 0);
	if (bytes < 0)
	{
		perror("Error sending message");
		exit(EXIT_FAILURE);
		return bytes;
	}
	return bytes;
}
