#include "POPClient.hxx"
#include <stdio.h>

POPClient::POPClient(string ipaddress, int port, string domain):
	TCPClient(ipaddress, port), domain(domain)
{
}

int POPClient::popsend(string message, string command)
{
	string msg = command + " " + message + "\r\n";
	//cout << "Sebding:" << msg << endl;
	return tcpsend(msg);
}

string POPClient::popreceive(int &status)
{
	int size;
	string msg = receive(size);
	//cout << "Receives: " <<size<< msg;
	if (size <= 2)
	{
		status = -1;
		return "Error in receive\n";
	}
	if (msg[0] == '+' && size >= 4)
	{
		status = 1;
		return msg;//.substr(3);
	}
	else if (msg[0] == '-' && size >= 5)
	{
		status = 0;
		return msg;//.substr(4);
	}
	else
	{
		status = 2;
		return msg;
	}
}

void POPClient::sendLIST()
{
	popsend("", "LIST");
	int status;
	string msg = popreceive(status);
	if (status <= 0)
	{
		cout << msg;
	}
	else
	{
		cout<<"The messages in your inbox are:\n";
		cout << msg;
		/*while(1)
		{
			string msg = popreceive(status);
			if(msg == ".\n")
			{
				break;
			}
			cout<<msg;
		}*/
	}
}

void POPClient::sendRETR(int id)
{
	popsend(to_string(id), "RETR");
	int status;
	string msg = popreceive(status);
	if (status <= 0)
	{
		cout << msg;
	}
	else
	{
		cout << msg;
		cout << "Do you want to save the mail to a file(y/n)?" << endl;
		string ch;
		cin >> ch;
		if (ch[0] == 'y')
		{
			cout << "Enter the file name with path(else the file is created in the current directory)" << endl;
			string fname;
			cin >> fname;
			FILE *fp2 = fopen(fname.c_str(), "w");
			if (fp2 == NULL)
			{
				cout << "Could not create file\n";
				return;
			}
			fprintf(fp2, "%s", msg.c_str());
			cout << "Mail successfully saved" << endl;

		}
		
		
	}
}

void POPClient::sendDELE(int id)
{
	popsend(to_string(id), "DELE");
	int status;
	string msg = popreceive(status);
	if (status <= 1)
	{
		cout << msg;
	}
}

void POPClient::sendSTAT()
{
	popsend("", "STAT");
	int status;
	string msg = popreceive(status);
	if (status <= 1)
	{
		cout << msg;
	}
}

void POPClient::sendQUIT()
{
	popsend("", "QUIT");
	int status;
	string msg = popreceive(status);
	if (status != 1)
	{
		cout << msg;
	}
}

void retrieveMails(string ip, int port, string domain, string username)
{
	POPClient client(ip, port, domain);
	client._connect();
	int status;
	string str = client.popreceive(status);
	if (status != 1)
	{
		cout << "Error :" << str << endl;
		exit(1);
	}
	cout << "Welcome to " << domain << " mailbox" << endl;
	int quit = 0;
	while (!quit)
	{
		client.popsend(username + "@" + domain, "USER");
		string msg = client.popreceive(status);
		if (status <= 0)
		{
			cout << msg;
		}
		else if (status == 1)
		{
			string password;
			cout << "Enter password" << endl;
			cin >> password;
			client.popsend(password, "PASS");
			//client.tcpsend("PASS "+password+"\n");
			string msg = client.popreceive(status);
			if (status <= 0)
			{
				cout << msg;
			}
			else if (status == 1)
			{
				client.sendLIST();
				while (!quit)
				{
					cout << "Enter 1 to retrieve, 2 to list , 3 for quit" << endl;
					int choice;
					cin >> choice;
					int id;
					switch (choice)
					{
					case 1:
						cout << "Enter the message id(0 for back)" << endl;
						cin >> id;
						if (id > 0)
							client.sendRETR(id);
						break;
					case 2:
						client.sendLIST();
						break;
					case 3:
						client.sendQUIT();
						quit = 1;
						break;
					default:
						cout << "Invalid Choice" << endl;
						break;
					}
				}

			}
			else
			{
				cout << "Some unexpected error occurred" << endl;
			}
		}
		else
		{
			cout << "Some unexpected error occurred" << endl;
		}
		if (quit == 0)
		{
			cout << "Press q to quit and any other key to continue" << endl;
			string q;
			cin >> q;
			if (q == "q")
				quit = 1;
		}
	}
	cout << "Bye" << endl;

}

