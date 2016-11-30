#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h> // For inet_pton (), inet_ntop ().
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <bits/stdc++.h>
#include <sys/select.h> // For select().
using namespace std;

#define BUFF_SZ 512
#define PROB 0.5 //p1=p2=p3
#define PORT1 23465 //TCP conn
#define PORT2 25000 //UDP conn
#define TIMEOUT 2 //sec

int prod[10][2];
int buyreq[10];

int tcp_read(int Socket, char* buff, int len)
{
    memset(buff, 0, len);
    fd_set rd_set;
	FD_ZERO(&rd_set);
	FD_SET(Socket, &rd_set);
	struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0; 
    int sel = select(Socket + 1, &rd_set, NULL, NULL, &timeout);
    if (sel == -1)
	{
		perror("Server: Error in select");
		exit(1);
	}
	if(FD_ISSET(Socket, &rd_set))
    {
    	int rst = read(Socket, buff, len);
	    if (rst == -1)
	    {
	        perror ("Server/Client: Receive failed");
	        exit (1);
	    }
	    buff[rst] = '\0';
	    //fprintf(stderr, "Read from client %s\n", buff);
		return rst;
	}
	else
		return -2;
}

int tcp_write(int Socket, char* message)
{
    int rst = send(Socket, message, strlen(message), MSG_CONFIRM);
    if (rst == -1)
    {
        perror ("Server/Client: Send failed");
        exit (1);
    }
    return rst;
}

int connect_TCPserver(char *ip,int port_num)
{
	int rst; // Return status of functions.
    /**************** Create a socket. *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
                * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
    if (sfd == -1) 
    {
        perror ("Server: socket error");
        exit (1);
    }
    // printf ("Socket fd = %d\n", sfd);
    
    int enable = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    
    /***************** Assign an address to the server **************/
    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.
    
    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);
    memset (&cli_addr, 0, addrlen);

    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    srv_addr.sin_port   = htons (port_num); // Port Number.
    
    rst = inet_pton (AF_INET, ip, &srv_addr.sin_addr); /* To 
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Server Presentation to network address conversion.\n");
        exit (1);
    }    
    
    
    
    /****************** Bind the server to an address. ***************/
    rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen); /* Note
        * the type casting of the pointer to the server's address. */
    if (rst == -1)
    {
        perror ("Server: Bind failed");
        exit (1);
    }
    
    
    
    /***************** listen (): Mark the socket as passive. *******/
    // printf ("Max connections allowed to wait: %d\n", SOMAXCONN);
    rst = listen (sfd, SOMAXCONN);
    if (rst == -1)
    {
        perror ("Server: Listen failed");
        exit (1);
    }
  	return sfd;
 }   

int server_accept(int sfd)    //TCP 
{
	struct sockaddr_in cli_addr;
	socklen_t addrlen = sizeof (struct sockaddr_in);
	memset (&cli_addr, 0, addrlen);
	/***************** accept (): Waiting for connections ************/
	int cfd = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen); /* 
	    * Returns the file descriptor for the client. 
	    * Stores the address of the client in cli_addr. */
	if (cfd == -1)
	{
	    perror ("Server: Accept failed");
	    exit (1);
	}

	return cfd;
}

int connect_UDPserver(char* ipaddress, int port_num,int tcp_cfd)
{

    int rst; // Return status of functions.
    /***************** Create a socket *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_DGRAM, 0);
    int enable = 1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (sfd == -1)
    {
        perror ("Server_1.c socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);

    /***************** Binding the server to an address. *************/
    struct sockaddr_in srv_addr, cl_addr; // For sever and client addresses.
    socklen_t addrlen = sizeof (struct sockaddr_in);


    /* Clear the two addresses. */
    memset (&srv_addr, 0, addrlen);
    memset (&cl_addr, 0, addrlen);

    /* Assign a server address. */
    srv_addr.sin_family = AF_INET; // IPv4
    srv_addr.sin_port   = htons (port_num);


    /* The servers IP address. */
    rst = inet_pton (AF_INET, ipaddress, &srv_addr.sin_addr);
    if (rst <= 0)
    {
        perror ("Server Presentation to network address conversion.\n");
        exit (1);
    }
    //printf("Yes\n");
    rst = bind (sfd, (struct sockaddr *) &srv_addr, addrlen);
    if (rst < 0)
    {
        perror ("Server: bind failed");
        exit (1);
    }
    int b_recv;
    char buff[BUFF_SZ];
	char buff2[BUFF_SZ];
	memset(buff,0,sizeof(buff));
	memset(buff2,0,sizeof(buff2));
	int flags=0;
	int i=0;
	while(1){
	cout<<"Receiving\n";
    b_recv = recvfrom (sfd, buff, BUFF_SZ, flags,
                               (struct sockaddr *) &cl_addr, &addrlen);
    if (b_recv == -1)
    {
        perror ("Server: recvfrom failed");
        exit (1);
    }
    if(strcmp(buff,"BROWSE")==0)
	{
	 	memset(buff2,0,sizeof(buff2));
	    for(i=0;i<10;i++)
	    {
	    	char temp[20];
	    	sprintf(temp,"Id: %d Q: %d,",prod[i][0],prod[i][1]);
	    	strcat(buff2,temp);
	    }
	    cout<<"Writing\n";
	    rst = sendto (sfd, buff2, BUFF_SZ, flags,
	                          (struct sockaddr *) &cl_addr,
	                          sizeof (struct sockaddr_in));
	    if (rst < 0)
	    {
	        perror ("Client: Sendto function call failed");
	        exit (1);
	    }
	}
	else 
	{	
		char *p=strchr(buff,':');
		char domain[BUFF_SZ];
		if(p)
			strcpy(domain,p+1);
		int id,qt;
		sscanf(domain,"%d %d",&id,&qt);
		cout<<"Request Id:"<<id<<" Quantity:"<<qt<<endl;
		int sucs=0;
		int index;
		for(i=0;i<10;i++)
		{
			// cout<<prod[i][0]<<endl;
			if(prod[i][0]==id)
			{
				// cout<<"hello"<<id<<"q"<<prod[i][1]<<endl;
				if(prod[i][1]>=qt)
				{
					prod[i][1]-=qt;
					buyreq[i]=1;
					sucs=1;
					index=i;
				}
			}
		}
		// cout<<sucs<<endl;
		if(sucs==1)
		{
			memset(buff2,0,sizeof(buff2));
			sprintf(buff2,"BUYREQ: %d %d",index,prod[index][1]);
			cout<<"Tcp write"<<endl;
			tcp_write(tcp_cfd,buff2);
			memset(buff,0,sizeof(buff));
			cout<<"Tcp read\n";
			if(tcp_read(tcp_cfd,buff,BUFF_SZ)!=-2)
			{
				if(strcmp(buff,"SUCCESS")==0)
				{
					buyreq[index]=0;
					printf("Sold Prod: Id:%d Quantity and Acknowledged: %d\n",id,qt);
				}
				else
				{
					printf("Sold Prod: Id:%d Quantity but not Acknowledged: %d\n",id,qt);
				}
			}
			else
			{
				printf("Sold Prod: Id:%d Quantity but not Acknowledged: %d\n",id,qt);
			}
			strcpy(buff2,"Successful!!");
			
		}
		else
		{	
			strcpy(buff2,"Unsucessful!!");
		}
		rst = sendto (sfd, buff2, BUFF_SZ, flags,
	                          (struct sockaddr *) &cl_addr,
	                          sizeof (struct sockaddr_in));
	    if (rst < 0)
	    {
	        perror ("Client: Sendto function call failed");
	        exit (1);
	    }
	}
}
    return sfd;
}

int udp_read(int Socket, char* buff, int len)
{
    int flags = 0;
    struct sockaddr_in sender_addr;
    socklen_t sender_len;
    int b_recv = recvfrom (Socket, buff, len, flags,
                           (struct sockaddr *) &sender_addr, &sender_len);
    if (b_recv == -1)
    {
        perror ("Server: recvfrom failed");
        exit (1);
    }
    return b_recv;
}

int udp_write(int Socket, char* message)
{
    int flags = 0;
    struct sockaddr_in dest_addr; /* sockaddr_in because we are using
            * IPv4. Type casted to struct sockaddr * at time of
            * various system calls. */

    socklen_t addrlen = sizeof (struct sockaddr_in);


    // Initializing destination address.
    memset (&dest_addr, 0, addrlen); // Initializes address to zero.

    dest_addr.sin_family = AF_INET;  // Address is in IPv4 format.
    dest_addr.sin_port   = htons (PORT2);  // Port number of the server.
    int rst;
    rst = inet_pton (AF_INET, "127.0.0.1", &dest_addr.sin_addr);
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }
    rst = sendto (Socket, message, BUFF_SZ, flags, (struct sockaddr *) &dest_addr,
                  sizeof (struct sockaddr_in)); /* Value of rst is 20,
        /* on successful transmission; i.e. It has nothing to do with a
        * NULL terminated string.
        */
    if (rst < 0)
    {
        perror ("Client: Sendto function call failed");
        exit (1);
    }
    return rst;
}

void initialize_data()
{
	int i=0;
	for(i=0;i<10;i++)
	{
		prod[i][0]=i+1;
		prod[i][1]=i+1;
	}
	memset(buyreq,0,sizeof(buyreq));
}


int main()
{
	char buff[BUFF_SZ];
	char buff2[BUFF_SZ];
	initialize_data();
	memset(buff,0,sizeof(buff));
	memset(buff2,0,sizeof(buff));
	int sfd1,sfd2,cfd;
	char ip[60]="127.0.0.1";
	sfd1=connect_TCPserver(ip,PORT1);
	cfd=server_accept(sfd1);
	sfd2=connect_UDPserver(ip,PORT2,cfd);
	while(1);
	return 0;
}