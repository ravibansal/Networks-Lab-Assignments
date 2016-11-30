#include "smtp.h"
#include "POPClient.hxx"

int port_numi = 25;
int popabcport = 110;
int popxyzport = 26002;
char ip1[30] = "10.5.30.131"; //abc
int main(int argc, char* argv[]) //Usage: ./client_smtp <debug> <ip>
{
	int sfd = -1;
	//cout<<argv[1]<<" "<<argv[2]<<endl;
	if (argc < 3)
	{
		cout << "Usage: ./client_smtp <debug> <ip_garuda>\n";
		exit(1);
	}

	if (strcmp(argv[1], "d") == 0)
	{
		debug_on();
		//cout<<"debug mode on\n";
	}
	strcpy(ip1, argv[2]);
	while (1)
	{	
		int work;
		cout << "Do you want to (1) send emails or (2) retreive emails (3 to quit)?\n";
		cin >> work;
		if (work == 1)
		{
			while (1) 
			{
				char from[60];
				char to[600];
				while (1)
				{
					// char *buf = NULL;
					char domain[40] = "garudaserver.com";
					cout << "Mail From?\n";
					cin>>from;

					for (int i = 0; from[i]; i++) {
						from[i] = tolower(from[i]);
					}

					if (!isValidEmailAddress(from))
						cout << "Please Enter a valid Email Address\n";
					else 
					{
						sfd = connect_client(ip1, port_numi);
					}
					if (initiate_client(domain, sfd) == 0)
						break;
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
				while (1)
				{
					
					cout << "Mail to?(for multiple mail id's give it as comma separated)\n";

					cin>>to;
					
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
							cout << to << " is not a valid email\n";
						else
						{
							int to_rep = env_client_to(token, sfd);
							if (to_rep == 0)
							{
								i++;
							}
							else if (to_rep == -1)
							{
								cout << "Fatal Error in client\n";
								exit(1);
							}
							else
							{
								cout << "550 No such user here " << token << endl;
							}
						}
						token = strtok(NULL, s);
					}
					if (i == 0)
					{
						cout << "You haven't provided atleast one valid email\n";
					}
					else
						break;
				}
				if (start_data(sfd) == 0)
				{
					char data[1024];
					cout << "Input mail body [end with a single dot(.)]\n";
					while (1)
					{
						getchar();
						scanf("%[^\n]s", data);
						int temp = send_data(sfd, data);
						if (temp == -3)
						{
							cout << "Server Failure\n";
							exit(1);
						}
						else if(temp != -2)
							break;
					}
				}
				else 
				{
					cout << "server failure at begining of data\n";
					exit(1);
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
			if (domain == "garudaserver.com")
			{
				retrieveMails(ip1, popabcport, domain, username);
			}
			else
			{
				cout << "invalid email" << endl;
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
	}
	cout << "Bye!\n";
	return 0;
}
