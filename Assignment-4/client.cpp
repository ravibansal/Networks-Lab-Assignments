#include "smtp.h"
#include "POPClient.hxx"

int port_numi = 23465;
int port_numi_2 = 25000;
int popabcport = 27001;
int popxyzport = 26002;
char ip1[30] = "127.0.0.1"; //abc
char ip2[30] = "127.0.0.1"; //xyz
int main(int argc, char* argv[]) //Usage: ./client_smtp <debug> <ip>
{
	int sfd;
	//cout<<argv[1]<<" "<<argv[2]<<endl;
	if (argc < 4)
	{
		cout << "Usage: ./client_smtp <debug> <ip_abc> <ip_xyz>\n";
		exit(1);
	}

	if (strcmp(argv[1], "d") == 0)
	{
		debug_on();
		//cout<<"debug mode on\n";
	}
	strcpy(ip1, argv[2]);
	strcpy(ip2, argv[3]);
	int given=0;
	if(argc>4)
	{
		given=1;
	}
	int justrep=0;
	while (1)
	{
		justrep=0;		
		int work;
		if(given==1)
		{
			work=1;
		}	
		else
		{
			cout << "Do you want to (1) send emails or (2) retreive emails (3 to quit)?\n";
			cin >> work;
		}
		if (work == 1)
		{
			while (1) 
			{
				char from[60];
				char to[600];
				if(given==0)
				{
					cout<<"Do you want to open more than one account (y/n)\n";
					char repl;
					cin>>repl;
					char usernamesmtp[60];
					char usernamesmtp2[60];
					if(repl=='y' || repl=='Y')
					{
						cout<<"Please enter your username\n";
						cin>>usernamesmtp;
						strcpy(usernamesmtp2,usernamesmtp);
						strcat(usernamesmtp,"@abc.com");
						strcat(usernamesmtp2,"@xyz.com");
						if(fork()==0)
						{
							execlp("xterm","xterm","-title", usernamesmtp,"-e","build/client_smtp",argv[1],argv[2],argv[3],usernamesmtp,(char *)(NULL));
						}
						if(fork()==0)
						{
							execlp("xterm","xterm","-title", usernamesmtp2,"-e","build/client_smtp",argv[1],argv[2],argv[3],usernamesmtp2,(char *)(NULL));
						}
						justrep=1;
						//exit(0);
					}
				}
                if(justrep==1)
                    break;
				while (1)
				{
					// char *buf = NULL;
					char domain[40];
					if(given==0)
					{
						//getchar();
						cout << "Mail From?\n";
						cin>>from;
					}
					else
					{
						strcpy(from,argv[4]);
					}
					for (int i = 0; from[i]; i++) {
						from[i] = tolower(from[i]);
					}
					// cout<<from<<endl;
					char *p = strchr(from, '@');
					//cout<<p;
					// int tobreak = 0;
					if (!isValidEmailAddress(from))
						cout << "Please Enter a valid Email Address\n";
					else
					{
						// cout<<"Helo";
						strcpy(domain, p + 1);
						if (strcmp(domain, "abc.com") == 0 || strcmp(domain, "xyz.com") == 0)
						{
							if (strcmp(domain, "abc.com") == 0)
							{
								//cout<<"1"<<endl;
								sfd = connect_client(ip1, port_numi);
							}
							else {
								//cout<<"2"<<endl;
								sfd = connect_client(ip2, port_numi_2);
							}
							if (initiate_client(domain, sfd) == 0)
								break;
							else
							{
								cout << "Invalid Message received\n";
								exit(1);
							}
						}
						else
						{
							cout << "Please enter a valid domain\n";
							continue;
						}
					}
				}
				while (1)
				{
					int temp = env_client_from(from, sfd);
					if (temp == 0)
					{
						break;
					}
					else if (temp == -3)
					{
						// getchar();
						cout << "No such user here\n";
						cout << "Mail From?\n";
						// scanf("%[^\n]s", from);
						cin>>from;
						for (int i = 0; from[i]; i++) {
							from[i] = tolower(from[i]);
						}
						continue;
					}
					else
					{
						exit(1);
					}
				}
				int vrfy=0;
				while (1)
				{
					char reply;
					cout<<"Do you want to verify mail id (y/n)\n";
					cin>>reply;
					if(reply=='Y' || reply=='y')
					{
						cout<<"Please enter the maild to check"<<endl;
						cin>>to;
						vrfy=1;
					}
					else
					{
						cout << "Mail to?(for multiple mail id's give it as comma separated)\n";
						// getchar();
						// scanf("%[^\n]s", to);
						cin>>to;
					}
					for (int i = 0; to[i]; i++) {
						to[i] = tolower(to[i]);
					}
					const char s[2] = ",";
					char *token;
					token = strtok(to, s);
					/* walk through other tokens */
					int i = 0;
					while ( token != NULL )
					{
						if (!isValidEmailAddress(token))
							cout << to << " :is not a valid email\n";
						else
						{
							char *p = strchr(to, '@');
							if (strcmp(p + 1, "abc.com") != 0 && strcmp(p + 1, "xyz.com") != 0)
							{
								cout << to << " :has not a valid domain\n";
							}
							else
							{
								int to_rep = env_client_to(token, sfd,vrfy);
								if (to_rep == 0)
								{
									i++;
									vrfy=0;
								}
								else if (to_rep == -1)
								{
									cout << "Fatal Error in client\n";
									exit(1);
								}
								else
								{
									cout << "550 No such user here: " << token << endl;
								}
							}
						}
						if(vrfy==1)
						{
							break;
						}
						token = strtok(NULL, s);
					}
					if(vrfy==1)
					{
						break;
					}
					if (i == 0)
					{
						cout << "You haven't provided atleast one valid email\n";
					}
					else
						break;
				}
				if(vrfy==0)
				{
					if (start_data(sfd) == 0)
					{
						char data[1024];
						cout << "Input mail body [end with a single dot(.)]\n";
						while (1)
						{
							getchar();
							scanf("%[^\n]s", data);
							int temp = send_data(sfd, data);
							if (temp == -1)
							{
								cout << "Server Failure\n";
								exit(1);
							}
							else if (temp == 0)
								break;
						}
					}
					else {
						cout << "server failure at begining of data\n";
						exit(1);
					}
				}
				quit(sfd);
				cout << "Do you want to send more emails? (Y) Yes (N) No\n";
				char loop;
				cin >> loop;
				if (loop == 'Y' || loop=='y')
					continue;
				else
					break;
			}
		}
		else if (work == 2)
		{
			cout << "Do you want to use accounts of both domain(y/n)" << endl;
			string ch;
			cin >> ch;
			if (ch[0] == 'y' || ch[0] == 'Y')
			{
				string username = "";
				while (username == "")
				{
					cout << "Enter Username" << endl;
					cin >> username;
				}
				if (fork() == 0)
				{
					string name = username + "@abc.com";
					char port[20];
					sprintf(port, "%d", popabcport);
					execlp("xterm", "xterm", "-title", name.c_str(), "-e", "build/popclient",
					       ip1, "abc.com", username.c_str(), port, (char*)(NULL));
					exit(0);
				}
				else
				{
					if (fork() == 0)
					{
						char port[20];
						sprintf(port, "%d", popxyzport);
						string name = username + "@xyz.com";
						execlp("xterm", "xterm", "-title", name.c_str(), "-e", "build/popclient",
						       ip2, "xyz.com", username.c_str(), port, (char*)(NULL));
						exit(0);
					}
				}
			}
			else
			{
				string email = "";
				while (email == "")
				{
					cout << "Enter Email" << endl;
					cin >> email;
					if (!isValidEmailAddress(email.c_str()))
						email = "";
				}
				int idx = email.find_first_of("@");
				string username = email.substr(0, idx);
				string domain = email.substr(idx + 1);
				if (domain == "abc.com")
				{
					retrieveMails(ip1, popabcport, domain, username);
				}
				else if (domain == "xyz.com")
				{

					retrieveMails(ip2, popxyzport, domain, username);
				}
				else
				{
					cout << "invalid email" << endl;
				}
			}

		}
		else if (work == 3)
		{
			break;
		}
		else
		{
			cout << "Invalid Input" << endl;
		}
		if(given==1)
		{
			break;
		}
	}
	cout << "Bye!\n";
	return 0;
}
