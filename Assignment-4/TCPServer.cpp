#include "TCPServer.hxx"

TCPServer::TCPServer(string ipaddress, int port):ipaddress(ipaddress),port(port),sfd(-1)
{
	
}

int TCPServer::tcpsend(int clientfd, string message)
{
	char buff[BUFFSIZE];
	memset(buff,0,BUFFSIZE);
	strcpy(buff, message.c_str());
	int bytes = send(clientfd, buff, BUFFSIZE, 0);
	if (bytes < 0)
	{
		perror("Error sending message");
		return bytes;
		exit(1);
	}
	return bytes;
}

string TCPServer::receive(int clientfd, int &size)
{
	char buff[BUFFSIZE];
	memset(buff, 0, BUFFSIZE);
	int bytes = recv(clientfd, buff, BUFFSIZE, 0);
	if (bytes < 0)
	{
		perror("Error reading message");
		return "";
		exit(1);
	}
	string s(buff);
	size = bytes;
	return s;
}

int TCPServer::connect()
{
	int status;
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
	{
		perror("Server: Socket Error");
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

	status = bind(sfd, (struct sockaddr*)&server_addr, srvaddr_len);

	if (status == -1)
	{
		perror("Server: Error in binding socket to address");
		exit(1);
	}

	printf("Socket successfully bound to %s on port %d\n", ipaddress.c_str(), port);
	printf("Maxinum connections allowed: %d\n", SOMAXCONN);
	status = listen(sfd, SOCK_BACKLOG);

	if (status == -1)
	{
		perror("Server: Error in listen");
		exit(1);
	}
	printf("Server listening successfully on %s, %d\n", ipaddress.c_str(), port);

	return sfd;
}

