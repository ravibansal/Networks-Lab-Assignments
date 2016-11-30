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
using namespace std;

#define BUFF_SZ 512
#define PROB 0.5 //p1=p2=p3
#define PORT1 23465 //TCP conn
#define PORT2 25000 //UDP conn

int prod[10][2];

int tcp_read(int clientSocket, char* buff, int len)
{
    memset(buff, 0, len);
    int rst = read(clientSocket, buff, len);
    if (rst == -1)
    {
        perror ("Server: Receive failed");
        exit (1);
    }
    buff[rst] = '\0';
    fprintf(stderr, "Read from client %s\n", buff);

    return rst;
}

int tcp_write(int clientSocket, char* message)
{
    int rst = send(clientSocket, message, strlen(message), MSG_CONFIRM);
    if (rst == -1)
    {
        perror ("Server: Send failed");
        exit (1);
    }
    return rst;
}

int connect_toTCPserver(char *ip,int port_num)
{
	int rst; // Return status of functions.
    // int cfd; // File descriptor for the client.

    /**************** Create a socket. *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
             * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
    if (sfd == -1) 
    {
        perror ("Client: socket error");
        exit (1);
    }
    //printf ("Socket fd = %d\n", sfd);
    
    
    
    /***************** Assign an address of the server **************/
    struct sockaddr_in srv_addr;
    //cli_addr; // Addresses of the server and the client.
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.
    
    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);
    
    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    srv_addr.sin_port   = htons (port_num); // Port Number.
    
    rst = inet_pton (AF_INET, ip, &srv_addr.sin_addr); /* To 
                              * type conversion of the pointer here. */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }    


    
    /***************** Connect to the server ************************/
    rst = connect (sfd, (struct sockaddr *) &srv_addr, addrlen);
    if (rst == -1)
    {
        perror ("Client: Connect failed.");
        exit (1);
    }
    if(rst==-1)
    	return -1;
    else
    	return sfd;
}

void initialize_data()
{
	int i=0;
	for(i=0;i<10;i++)
	{
		prod[i][0]=i+1;
		prod[i][1]=i+1;
	}
}


int main()
{
	char buff[BUFF_SZ];
	char buff2[BUFF_SZ];
	initialize_data();
	memset(buff,0,sizeof(buff));
	memset(buff2,0,sizeof(buff));
	int sfd;
	char ip[60]="127.0.0.1";
	initialize_data();
	sfd=connect_toTCPserver(ip,PORT1); //connect to tcp server and get the sfd
	cout<<"Waiting"<<endl;
	while(1){
	if(tcp_read(sfd,buff,BUFF_SZ)>0);
	{
		char *p=strchr(buff,':');
		char domain[BUFF_SZ];
		if(p)
			strcpy(domain,p+1);
		int ind,qt;
		sscanf(domain,"%d %d",&ind,&qt);
		// cout<<"Request Id:"<<id<<" Quantity:"<<qt<<endl;
		int sucs=0;
		int index;
		int i;
		if(ind<10)
		{
			prod[ind][1]=qt;
			sucs=1;
		}
		if(sucs==1)
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"SUCCESS");
		}
		else
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"UNSUCCESS");
		}
		cout<<"Sent"<<endl;
		tcp_write(sfd,buff2);
	}
}
	return 0;
}