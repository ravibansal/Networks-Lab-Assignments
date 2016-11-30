#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "allocate.cpp"
#include <unistd.h>

#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <sys/select.h> // For select().
#include <errno.h>

#include <vector>
#include <queue>

#define BUFFSIZE 500
#define SOCK_BACKLOG 10
#define DISCONNECT "serd_disconnect"

int tcp_send(int clientfd, string message)
{
	int bytes = send(clientfd, message.c_str(), BUFFSIZE, 0);
	if (bytes < 0)
	{
		perror("Error sending message");
		return bytes;
		exit(1);
	}
	return bytes;
}

string tcp_receive(int clientfd, int *size)
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
	*size = bytes;
	return s;
}

int tcp_connect(string ipaddress, int port_num)
{
	int status;
	int sfd;
	sfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK , 0);
	if (sfd == -1)
	{
		perror("Server: Socket Error");
		exit(1);
	}

	printf("Socket fd = %d\n", sfd);

	struct sockaddr_in server_addr;
	socklen_t srvaddr_len = sizeof(struct sockaddr_in);

	memset(&server_addr, 0, srvaddr_len);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_num);

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

	printf("Socket successfully bound to %s on port %d\n", ipaddress.c_str(), port_num);
	printf("Maxinum connections allowed: %d\n", SOMAXCONN);
	status = listen(sfd, SOCK_BACKLOG);

	if (status == -1)
	{
		perror("Server: Error in listen");
		exit(1);
	}
	printf("Server listening successfully on %s, %d", ipaddress.c_str(), port_num);

	return sfd;
}

void print_client_info(struct sockaddr_in clientaddr,int cfd)
{
	printf("Connected to client: %d\n",cfd);
	//TODO: implement this

}

void handle_clients(int sfd)
{
	memset(t1,0,sizeof(t1));
	struct sockaddr_in client_addr;
	socklen_t cltaddr_len = sizeof(client_addr);
	vector<int> clients;
	priority_queue<Request> requests;
	int cfd;
	while(1)
	{
		while ((cfd = accept(sfd, (struct sockaddr*)&client_addr, &cltaddr_len)) != -1) {
			print_client_info(client_addr,cfd);
			clients.push_back(cfd);
		}
		if (clients.size() < 1)
			continue;
		fd_set rd_set;
		FD_ZERO(&rd_set);
		int maxcfd = -1;
		for (std::vector<int>::iterator cl = clients.begin(); cl != clients.end(); ++cl)
		{
			if (*cl > maxcfd)
				maxcfd = *cl;
			FD_SET(*cl, &rd_set);
		}

		int sel = select(maxcfd + 1, &rd_set, NULL, NULL, NULL);

		if (sel == -1)
		{
			perror("Server: Error in select");
			exit(1);
		}

		for (std::vector<int>::iterator cl = clients.begin(); cl != clients.end(); ++cl)
		{
			if (FD_ISSET(*cl, &rd_set))
			{
				int num = 0;
				string req = tcp_receive(*cl, &num);
				printf("Read %s from %d\n",req.c_str(),*cl);
				if(req.size() < 1)
					continue;
				if (req == DISCONNECT)
				{
					close(*cl);
					clients.erase(cl);
					cl--;
				}
				else
				{
					Request r(*cl,req);
					requests.push(r);
				}
			}
		}

		while(! requests.empty())
		{
			Request r = requests.top();
			requests.pop();
			int result = allocator_berth(r);
			std::stringstream buff;
			if(result==1)
			{
                buff<<r.p_id<<","<<r.train<<",";
                for(int iter=0;iter<r.nop;iter++)
                {
                    if(r.cls=="AC")
                        buff<<"A"<<(coach[iter]+1)<<":"<<person[iter]<<",";
                    else
                    {
                        buff<<"SL"<<(coach[iter]+1)<<":"<<person[iter]<<",";
                    }
                }
			}
			else
			{
                buff<<"No Berths Available";
			}
			tcp_send(r.cfd,buff.str());
		}
		sleep(4);
	}
}
void *handle(void* sfd)
{
	handle_clients((long)sfd);
	pthread_exit(NULL);
}

void *print(void* sfd)
{
	while(1)
	{
		cout<<"Press any to list booking details"<<endl;
		char c;
		cin>>c;
		avl();
	}
	pthread_exit(NULL);
}
int main(int argc, char* argv[])
{
    pthread_t threads[2];
    int rc;
    long i;
    if(argc < 2)
    {
    	printf("Usage server port");
    }
    int sfd = tcp_connect("127.0.0.1",atoi(argv[1]));
    i=0;
    rc=pthread_create(&threads[i],NULL,handle,(void*)sfd);
    if(rc)
    {
    	cout<<"Error:unable to create thread, "<<rc<<endl;
    	exit(1);
    }
    i=1;
    rc=pthread_create(&threads[i],NULL,print,(void*)sfd);
    if(rc)
    {
    	cout<<"Error:unable to create thread, "<<rc<<endl;
    	exit(1);
    }
    //handle_clients(sfd);
    pthread_exit(NULL);
    return 0;
}

