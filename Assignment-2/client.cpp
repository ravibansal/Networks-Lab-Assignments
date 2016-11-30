#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h> // For the socket (), bind () etc. functions.
#include <netinet/in.h> // For IPv4 data struct..
#include <string.h> // For memset.
#include <arpa/inet.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <sys/time.h>
using namespace std;

#define BUFF_LENGTH 2000

#define DISCONNECT "serd_disconnect"

int port_num = 8888;

int connectToServer(char* ipaddress);
int tcp_read(int clientSocket, char* buff, int len);
int tcp_write(int clientSocket, char* message);

int connectToServer(char* ipaddress)
{
	/*
	*  Standard TCP connection flow to the server
	*/
	int rst; // Return status of functions.

	/**************** Create a socket. *******************************/
	int sfd; // Socket file descriptor.
	sfd = socket(AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
	if (sfd == -1)
	{
		perror ("Client: socket error");
		return (-1);
	}
	printf ("Socket fd = %d\n", sfd);


	/***************** Assign an address of the server **************/
	struct sockaddr_in srv_addr;
	socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

	// Clear the two addresses.
	memset (&srv_addr, 0, addrlen);

	// Assign values to the server address.
	srv_addr.sin_family = AF_INET; // IPv4.
	srv_addr.sin_port   = htons (port_num); // Port Number.

	rst = inet_pton (AF_INET, ipaddress , &srv_addr.sin_addr); /* To type conversion of the pointer here. */
	if (rst <= 0)
	{
		perror ("Client Presentation to network address conversion.\n");
		return (-1);
	}


	printf ("Trying to connect to server %s\n", ipaddress);
	/***************** Connect to the server ************************/
	rst = connect(sfd, (struct sockaddr *) &srv_addr, addrlen);
	if (rst == -1)
	{
		perror ("Client: Connect failed.");
		return (-1);
	}
	printf ("Connected to server\n");
	return sfd;
}

int tcp_read(int clientSocket, char* buff, int len)
{
	memset(buff, 0, len);
	int rst = read (clientSocket, buff, len);
	if (rst == -1)
	{
		perror ("Client: Receive failed");
		return (-1);
	}
	buff[rst] = '\0';
	return rst;
}

int tcp_write(int clientSocket, const char* message)
{
	int rst = send (clientSocket, message, strlen(message), MSG_CONFIRM);
	if (rst == -1)
	{
		perror ("Client: Send failed");
		exit (-1);
	}
	return rst;
}


void booker(int sfd)
{
	char filename[200], resultfile[200];
	printf("Welcome to IRCHT Online Ticket Booking System\n");
	printf("Enter the csv file name of the booking details\n");
	scanf("%s", filename);
	printf("Enter the csv file name to store the booking result details\n");
	scanf("%s", resultfile);
	string line;
	fstream requests(filename, fstream::in);
	if (!requests.is_open())
	{
		perror("Error in opening booking request file\n");
		exit(0);
	}
	fstream responses(resultfile, fstream::out);
	if (!responses.is_open())
	{
		perror("Error in creating booking response file\n");
		exit(0);
	}
	while (getline(requests, line))
	{
		struct timeval tp;
        gettimeofday(&tp, NULL);
        long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        line = line + "," + to_string(ms);
		printf("Read from server:%s\n",line.c_str());
		tcp_write(sfd, line.c_str());
		char response[BUFF_LENGTH];
		tcp_read(sfd, response, BUFF_LENGTH);
		responses << response << std::endl;
		sleep(5);

	}
	requests.close();

}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Error: Usage: booker ip [port]\n");
		exit(1);
	}
	char* ipaddress = argv[1];
	if (argc >= 3)
	{
		port_num = atoi(argv[2]);
	}
	int sfd = connectToServer(ipaddress);
	int cont = 1;
	while (cont)
	{
		booker(sfd);
		printf("Do you want to continue(y/n)\n");
		char r[20];
		scanf("%s", r);
		cont = r[0] == 'y';
	}
	tcp_write(sfd,DISCONNECT);
	printf("Good bye\n");
	return 0;
}