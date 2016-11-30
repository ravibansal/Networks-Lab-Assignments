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


int connect_toUDPserver(char *ip,int port_num)
{
	int rst; // Return status of functions.
    int sfd; // Socket file descriptor.
    /***************** Create a socket *******************************/
    sfd = socket (AF_INET, SOCK_DGRAM, 0); /* AF_INET --> IPv4,
                * SOCK_DGRAM --> UDP Protocol, 0 --> for the protocol. */
    if (sfd == -1)
    {
        perror ("Client_1.c socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);

    /****************** Send - receive messages **********************/
    int flags = 0; /* Even Now the client doesn't wait even if server
        * is not running.
        * Now the client will wait if its send-buffer is full.
        * */

    struct sockaddr_in dest_addr; /* sockaddr_in because we are using
            * IPv4. Type casted to struct sockaddr * at time of
            * various system calls. */

    socklen_t addrlen = sizeof (struct sockaddr_in);


    // Initializing destination address.
    memset (&dest_addr, 0, addrlen); // Initializes address to zero.

    dest_addr.sin_family = AF_INET;  // Address is in IPv4 format.
    dest_addr.sin_port   = htons (port_num);  // Port number of the server.


    rst = inet_pton (AF_INET, ip, &dest_addr.sin_addr); /* Note
            * that third field should point to an in_addr (in6_addr). */
    if (rst <= 0)
    {
        perror ("Client Presentation to network address conversion.\n");
        exit (1);
    }
    char buff[BUFF_SZ];
	char buff2[BUFF_SZ];
	memset(buff,0,sizeof(buff));
	memset(buff2,0,sizeof(buff));
	int inp;
	while(1){
	while(1)
	{
		cout<<"Do You want to Browse(1) or Buy(2)\n";
		cin>>inp;
		if(inp!=1 && inp!=2)
			cout<<"Invalid input. try again\n";
		else
			break;
	}
	// cout<<"writing\n";
	if(inp==1)
	{
	    strcpy(buff2,"BROWSE");
	    rst = sendto (sfd, buff2, BUFF_SZ, flags, (struct sockaddr *) &dest_addr,
	                      sizeof (struct sockaddr_in)); /* Value of rst is 20,
	            /* on successful transmission; i.e. It has nothing to do with a
	            * NULL terminated string.
	            */
	    if (rst < 0)
	    {
	        perror ("Client: Sendto function call failed");
	        exit (1);
	    }
	    struct sockaddr_in sender_addr;
	    socklen_t sender_len;
		// cout<<"reading\n";

	    int b_recv = recvfrom (sfd, buff, BUFF_SZ, flags,
	                           (struct sockaddr *) &sender_addr, &sender_len);
	    const char s[2] = ",";
	    char *token;
		token = strtok(buff, s);
		/* walk through other tokens */
		int i = 0;
		while ( token != NULL )
		{
			cout<<token<<endl;
			token = strtok(NULL, s);
		}
	}
	else{
		int id,quantity;
		cout<<"Please input id no.\n";
		cin>>id;
		cout<<"Please enter quantity\n";
		cin>>quantity;
		sprintf(buff2,"BUY:%d %d",id,quantity);
	    rst = sendto (sfd, buff2, BUFF_SZ, flags, (struct sockaddr *) &dest_addr,
	                      sizeof (struct sockaddr_in)); /* Value of rst is 20,
	            /* on successful transmission; i.e. It has nothing to do with a
	            * NULL terminated string.
	            */
	    if (rst < 0)
	    {
	        perror ("Client: Sendto function call failed");
	        exit (1);
	    }
	    struct sockaddr_in sender_addr;
	    socklen_t sender_len;
		// cout<<"reading\n";

	    int b_recv = recvfrom (sfd, buff, BUFF_SZ, flags,
	                           (struct sockaddr *) &sender_addr, &sender_len);
	    cout<<buff<<endl;
	}
}
    return sfd;
}

int udp_read(int Socket, char* buff, int len)
{
    int flags=0;
    struct sockaddr_in sender_addr;
    socklen_t sender_len;
    int b_recv = recvfrom (Socket, buff, sizeof(buff), flags,
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
    rst = sendto (Socket, message, sizeof(message), flags, (struct sockaddr *) &dest_addr,
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

int main()
{	
	int sfd;
	char ip[60]="127.0.0.1";
	sfd=connect_toUDPserver(ip,PORT2);
	// strcpy(buff2,"BROWSE");
	// cout<<"Writing\n";
	// if(udp_write(sfd,buff2)==-1)
	// 	printf("Error in upd send in client\n");
	// cout<<"Reading\n";
	// if(udp_read(sfd,buff,BUFF_SZ)==-1)
	// 	printf("Error in udp recvfrom in client\n");
	// printf("%s\n",buff);
	return 0;
}