#include "POPServer.hxx"


void POPServer::handleClient(int cfd)
{
	POPServerSession session(cfd,domain);
	int cont = 1;
	while(cont)
	{
		int size;
		string msg = receive(cfd,size);
		cont = session.processCMD(msg);
	}
	printf("Client disconnected\n");
}

POPServer::POPServer(string ipaddress, int port, string domain):
	TCPServer(ipaddress,port),domain(domain)
{

}

int POPServer::runForever()
{
	struct sockaddr_in client_addr;
	socklen_t cltaddr_len = sizeof(client_addr);
	memset(&client_addr,0,cltaddr_len);
	while(1)
	{
		cout<<sfd<<endl;
		int cfd = accept(sfd,(struct sockaddr*)&client_addr, &cltaddr_len);
		if(cfd <= 0)
		{
			perror("Error in accept");
			exit(EXIT_FAILURE);
		}
		if(fork() == 0)
		{
			close(sfd);
			handleClient(cfd);
			exit(EXIT_SUCCESS);
		}
		close(cfd);
		//std::thread thd(&this->handleClient,cfd);thd.detach();
	}
	return 0;
}
