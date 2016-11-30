#include "POPClient.hxx"
#include <stdio.h>

POPClient::POPClient(string ipaddress, int port, string domain):
	TCPClient(ipaddress,port),domain(domain)
{
}

int POPClient::popsend(string message, string command)
{
	string msg = command + " " + message + "\r\n";
	return tcpsend(msg);
}

string POPClient::popreceive(int &status)
{
	int size;
	string msg = receive(size);
	//cout<<"Receives: "<<msg;
	if(size <= 0)
	{
		status = -1;
		return "Error in receive\n";
	}
	if(msg[0] == '+')
	{
		status = 1;
		return msg.substr(4);
	}
	else if(msg[0] == '-')
	{
		status = 0;
		return msg.substr(5);
	}
	else
	{
		status = 2;
		return msg;
	}
}

void POPClient::sendLIST(int unread)
{
	if(!unread)
	{
		popsend("","LIST");	
	}
	else
	{
		popsend("-1","LIST");
	}
	int status;
	string msg = popreceive(status);
	if(status <= 0)
	{
		cout<<msg;
	}
	else
	{
		cout<<msg;
		while(1)
		{
			string msg = popreceive(status);
			if(msg == ".\n")
			{
				break;
			}
			cout<<msg;
		}
	}
}

void POPClient::sendRETR(int id)
{
	popsend(to_string(id),"RETR");
	int status;
	FILE* fp = tmpfile();

	string msg = popreceive(status);
	if(status <= 0)
	{
		fprintf(fp,"%s",msg.c_str());
		cout<<msg;
	}
	else
	{
		while(1)
		{
			string msg = popreceive(status);
			if(msg == ".\n")
			{
				cout<<"Do you want to save the mail to a file(y/n)?"<<endl;
				string ch;
				cin>>ch;
				if(ch[0] == 'y')
				{
					cout<<"Enter the file name with path(else the file is created in the current directory)"<<endl;
					string fname;
					cin>>fname;
					FILE *fp2= fopen(fname.c_str(),"w");
					if(fp2 == NULL)
					{
						cout<<"Could not create file\n";
						break;
					}
					rewind(fp);
					char buffer[520];
					while(!feof(fp))
					{
						if(fgets(buffer,520,fp) == NULL) break;
						fputs(buffer, fp2);
					}
					fclose(fp);
					fclose(fp2);
					cout<<"Mail successfully saved"<<endl;

				}

				break;
			}
			fprintf(fp,"%s",msg.c_str());
			cout<<msg;
		}
	}
}

void POPClient::sendDELE(int id)
{
	popsend(to_string(id),"DELE");
	int status;
	string msg = popreceive(status);
	if(status <= 1)
	{
		cout<<msg;
	}
}

void POPClient::sendSTAT()
{
	popsend("","STAT");
	int status;
	string msg = popreceive(status);
	if(status <= 1)
	{
		cout<<msg;
	}
}

void POPClient::sendQUIT()
{
	popsend("","QUIT");
	int status;
	string msg = popreceive(status);
	if(status <= 1)
	{
		cout<<msg;
	}
}

void retrieveMails(string ip, int port, string domain, string username)
{
	POPClient client(ip,port,domain);
	client._connect();
	int status;
	cout<<"Welcome to "<<domain<<" mailbox"<<endl;
	int quit = 0;
	while(!quit)
	{
		client.popsend(username,"USER");
		string msg = client.popreceive(status);
		if(status <= 0)
		{
			cout<<msg;
		}
		else if(status == 1)
		{
			string password;
			cout<<"Enter password"<<endl;
			cin>>password;
			client.popsend(password,"PASS");
			string msg = client.popreceive(status);
			if(status <= 0)
			{
				cout<<msg;
			}
			else if(status == 1)
			{
				client.sendLIST();
				while(!quit)
				{
					cout<<"Enter 1 to retrieve\n 2 to delete\n 3 to stat\n 4 to list(unread)\n 5 to list(all) \n 6 for quit"<<endl;
					int choice;
					cin>>choice;
					int id;
					switch(choice)
					{
						case 1:
							cout<<"Enter the message id(0 for back)"<<endl;
							cin>>id;
							if(id > 0)
								client.sendRETR(id);
							break;
							break;
						case 2:
							cout<<"Enter the message id(0 for back)"<<endl;
							cin>>id;
							if(id > 0)
								client.sendDELE(id);
							break;
						case 3: 
							client.sendSTAT();
							break;
						case 4:
							client.sendLIST();
							break;
						case 5:
							client.sendLIST(1);
							break;
						case 6:
							client.sendQUIT();
							quit = 1;
							break;
						default:
							cout<<"Invalid Choice"<<endl;
							break;
					}
				}

			}
			else
			{
				cout<<"Some unexpected error occurred"<<endl;	
			}
		}
		else
		{
			cout<<"Some unexpected error occurred"<<endl;
		}
		if(quit == 0)
		{
			cout<<"Press q to quit and any other key to continue"<<endl;
			string q;
			cin>>q;
			if(q == "q")
				quit = 1;
		}
	}
	cout<<"Bye"<<endl;

}

