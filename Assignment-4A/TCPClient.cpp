#include "TCPClient.hxx"

TCPClient::TCPClient(string ipaddress, int port):ipaddress(ipaddress),port(port),sfd(-1)
{
	
}

int TCPClient::tcpsend(string message)
{
	int bytes = send(sfd, message.c_str(), message.size(), 0);
	if (bytes < 0)
	{
		perror("Error sending message");
		exit(1);
	}
	return bytes;
}

/*string TCPClient::receive(int &size)
{
	char buff[10*1024];
	memset(buff, 0, 10*1024);
	size = 0;
	char *ptr = buff;
	while(1)
	{
		int bytes = recv(sfd, ptr, BUFFSIZE, 0);
		if (bytes < 0)
		{
			perror("Error reading message");
			exit(1);
		}
		if(bytes == 0)
			break;
		ptr += bytes;
		ptr[0] = '\0';
		size = size + bytes;
		cout<<"here"<<size<<endl;
		printf("HEre:%s\n",buff);
		if(buff[size-2] == '\r' && buff[size-1] == '\n')
			break;
		int t;
		cin>>t;
	}
	buff[size] = '\0';
	string s = string(buff);
	cout<<"size"<<size<<endl;
	return s;
}*/

string TCPClient::receive(int &size)
{
	char buff[BUFFSIZE];
	memset(buff, 0, BUFFSIZE);
	int bytes = recv(sfd, buff, BUFFSIZE, 0);
	if (bytes < 0)
	{
		perror("Error reading message");
		exit(1);
	}
	buff[bytes] = '\0';
	string s(buff,buff+bytes+1);
	size = bytes;
	return s;
}

int TCPClient::_connect()
{
	int status,rst;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
		perror("Client: Socket Error");
		exit(1);
	}
	printf("Socket fd = %d\n", sfd);


    int enable = 1;
	status = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if(status < 0)
	{
		perror("Unable to set port as reusable");
	}

	struct sockaddr_in server_addr;
	socklen_t srvaddr_len = sizeof(struct sockaddr_in);

	memset(&server_addr, 0, srvaddr_len);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	status = inet_pton(AF_INET, ipaddress.c_str(), &server_addr.sin_addr);

	if (status <= 0)
	{
		perror("Server: Presentation to network address conversion error");
		exit(1);
	}

	/***************** Connect to the server ************************/
    rst = connect (sfd, (struct sockaddr *) &server_addr, srvaddr_len);
    if (rst == -1)
    {
        perror ("Client: Connect failed.");
        exit (1);
    }

	return sfd;
}
