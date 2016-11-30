#include "smtp.h"
#include "sql.h"

char data[100][60];  //for storing email address
char sender[BUFF_SZ];
int recvr[100]; //To store sfd's for relaying to other server
int recvr_id[100];//To store the id in sql table to store the message for this domain
int no_of_recv=0; //Other domain receiver 
int curr_recvr=0; //This domain receiver

int port_num = 23465;
int port_num_2=25000;
MyDebugger debugger;
int tcp_read(int Socket, char* buff, int len)
{
    memset(buff, 0, len);
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

bool isCharacter(const char Character)
{
	return ( (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z'));
	//Checks if a Character is a Valid A-Z, a-z Character, based on the ascii value
}
bool isNumber(const char Character)
{
	return ( Character >= '0' && Character <= '9');
	//Checks if a Character is a Valid 0-9 Number, based on the ascii value
}
bool isValidEmailAddress(const char * EmailAddress)
{
	if(!EmailAddress) // If cannot read the Email Address...
		return 0;
	if(!isCharacter(EmailAddress[0])) // If the First character is not A-Z, a-z
		return 0;
	int AtOffset = -1;
	int DotOffset = -1;
	unsigned int Length = strlen(EmailAddress); // Length = StringLength (strlen) of EmailAddress
	for(unsigned int i = 0; i < Length; i++)
	{
		if(EmailAddress[i] == '@') // If one of the characters is @, store it's position in AtOffset
			AtOffset = (int)i;
		else if(EmailAddress[i] == '.') // Same, but with the dot
			DotOffset = (int)i;
	}
	if(AtOffset == -1 || DotOffset == -1) // If cannot find a Dot or a @
		return 0;
	if(AtOffset > DotOffset) // If the @ is after the Dot
		return 0;
	return !(DotOffset >= ((int)Length-1)); //Chech there is some other letters after the Dot
}

int connect_client(char *ip,int port_num)
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

int connect_server(char *ip,int port_num)
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
 
int server_accept(int sfd)    
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

int initiate_client(char *id,int sfd) //it will return the message that it will receive from server
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	do{
		memset(buff,0,sizeof(buff));
		tcp_read(sfd,buff,BUFF_SZ);
		//cout<<buff<<endl;
		debugger.debug(buff);
	}
	while(strcmp(buff,"220 service ready")!=0);
	while(1)
	{
		memset(buff2,0,sizeof(buff2));
		strcpy(buff2,"HELO: ");
		strcat(buff2,id);
		tcp_write(sfd,buff2);
		memset(buff,0,sizeof(buff));
		tcp_read(sfd,buff,BUFF_SZ);
		if(strcmp(buff,"220 service ready")!=0)
		{
			break;
		}
	}
	if(strcmp(buff,"250 OK")==0)
		return 0;
	else
		return -1;
}
void RemoveSpaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}
int search(char *id)
{
	char *p = strchr(id, '@');
	//cout<<id<<"    "<<p<<endl;
	*p=0;
	for(int i=0;i<4;i++)
	{
		if(strcmp(data[i],id)==0)
			return 0;
	}
	return -1;
}

int check_valid(char *id,char *domain)
{
	RemoveSpaces(id);
	if(strcmp(id,domain)!=0)
		return -1;
	else 
		return 0;
}
int verify(char *domain)
{
	char temp[BUFF_SZ];
	strcpy(temp,sender);
	char *p = strchr(temp, '@');
	// cout<<domain<<" is "<<(p+1)<<endl;
	if(p)
	{
		if(strcmp(domain,p+1)!=0)
		{
			return 0;
		}
		*p=0;
	}
	//cout<<temp<<endl;
	for(int i=0;i<4;i++)
	{
		if(strcmp(temp,data[i])==0)
		{
			return 0;
		}
	}
	return -1;
}
int check_valid_rcpt(char *id,char *domain,char *ip1,char *ip2,int vrfy)
{
	char temp[BUFF_SZ];
	strcpy(temp,id);
	char *p = strchr(temp, '@');
	if(p)
	{
		if(strcmp(domain,p+1)!=0)
		{
			return s2s_conn(sender,id,ip1,ip2,vrfy);
		}
		else{
			*p=0;
		}
	}
	//cout<<temp<<endl;
	for(int i=0;i<4;i++)
	{
		if(strcmp(temp,data[i])==0)
		{
			return -2;
		}
	}
	//cout<<"550 No such user here"<<":"<<id<<endl;
	return -3;
}
void initialize_data_abc()
{
	strcpy(data[0],"alice");
	strcpy(data[1],"arun");
	strcpy(data[2],"ananya");
	strcpy(data[3],"alex");
}

void initialize_data_xyz()
{
	strcpy(data[0],"bob");
	strcpy(data[1],"bilal");
	strcpy(data[2],"alex");
	strcpy(data[3],"bernee");
}
int initiate_server(char* domain,int cfd) //o for success and -1 for error
{
	if(strcmp(domain,"abc.com")==0)
	{
		initialize_data_abc();
	}
	else
	{
		initialize_data_xyz();
	}
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	char id[BUFF_SZ];
	while(1)
	{
		memset(buff2,0,sizeof(buff2));
		strcpy(buff2,"220 service ready");
		tcp_write(cfd,buff2);
		memset(buff,0,sizeof(buff));
		tcp_read(cfd,buff,BUFF_SZ);
		//cout<<buff<<endl;
		debugger.debug(buff);
		char *p = strchr(buff, ':');
		//cout<<p;
		strcpy(id,p+1);
		//if (!p) /* deal with error: / not present" */;
		*p = 0;
		if(strcmp(buff,"HELO")==0) //to check fopr HELLO command
		{
			int temp=check_valid(id,domain);
			if(temp==-1) //email id doesn't exist
			{
				memset(buff2,0,sizeof(buff2));
				strcpy(buff2,"wrong domain");
				tcp_write(cfd,buff2);
				return -1;
			}
			else{
				memset(buff2,0,sizeof(buff2));
				strcpy(buff2,"250 OK");
				tcp_write(cfd,buff2);
				return 0;
			}
		}
		else
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"Invalid Command");
			tcp_write(cfd,buff2);
			return -1;
		}
	}
}

int env_server(char *srv,int cfd,char *ip1,char *ip2)
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	char mesg[1024]={'\0'};
	while(1)
	{
		int rset=0;
		memset(buff,0,sizeof(buff));
		//cout<<"Going Reading\n";
		tcp_read(cfd,buff,BUFF_SZ);
		// cout<<buff<<endl;
		debugger.debug(buff);
		if(strcmp(buff,"QUIT")==0)
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"221 service closed");
			tcp_write(cfd,buff2);
			break;
		}
		char *p = strchr(buff, ':');
		strcpy(sender,p+1);
		RemoveSpaces(sender);
		//cout<<sender;
		int valid=verify(srv);
		if(p)
			*p=0;
		if(strcmp(buff,"MAIL FROM")==0 && valid==0)
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"250 OK");
			tcp_write(cfd,buff2);
		}
		else if(valid==-1)
		{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"550 No such user here");
			tcp_write(cfd,buff2);
			return -1;
		}
		else{
			memset(buff2,0,sizeof(buff2));
			strcpy(buff2,"Invalid Command in mail from");
			tcp_write(cfd,buff2);
			return -1;
		}
		while(1)
		{
			memset(buff,0,sizeof(buff));
			tcp_read(cfd,buff,BUFF_SZ);
			// cout<<buff<<endl;
			debugger.debug(buff);
			char *p = strchr(buff, ':');
			if(p)
				*p=0;
			if(strcmp(buff,"RCPT TO")==0)
			{
				memset(buff2,0,sizeof(buff2));
				RemoveSpaces(p+1);
				int ret=check_valid_rcpt(p+1,srv,ip1,ip2,0);
				if(ret>0)
				{
					recvr[no_of_recv]=ret;
					no_of_recv++;
					strcpy(buff2,"250 OK");
				}
				else if(ret==-2)
				{
					recvr_id[curr_recvr]=insert(sender,p+1,srv);
					curr_recvr++;
					strcpy(buff2,"250 OK");
				}
				else if (ret==-3){
					strcpy(buff2,"550 No such user here");
					//strcat(buff2,p+1);
				}
				else{
					strcpy(buff2,"Syntax Error or Failure: ");
					strcat(buff2,p+1);
				}
				tcp_write(cfd,buff2);
			}
			else if(strcmp(buff,"VRFY")==0)
			{
				memset(buff2,0,sizeof(buff2));
				RemoveSpaces(p+1);
				int ret=check_valid_rcpt(p+1,srv,ip1,ip2,1);
				if(ret>0)
				{
					recvr[no_of_recv]=ret;
					no_of_recv++;
					strcpy(buff2,"250 OK");
				}
				else if(ret==-2)
				{
					recvr_id[curr_recvr]=insert(sender,p+1,srv);
					curr_recvr++;
					strcpy(buff2,"250 OK");
				}
				else if (ret==-3){
					strcpy(buff2,"550 No such user here");
					//strcat(buff2,p+1);
				}
				else{
					strcpy(buff2,"Syntax Error or Failure: ");
					strcat(buff2,p+1);
				}
				tcp_write(cfd,buff2);
			}
			else if(strcmp(buff,"RSET")==0)
			{
				for(int i=0;i<no_of_recv;i++)
				{
					s2s_rset(recvr[i]);
				}
				memset(buff2,0,sizeof(buff2));
				strcpy(buff2,"250 OK");
				tcp_write(cfd,buff2);
				rset=1;
				break;
			}
			else if((no_of_recv!=0 || curr_recvr!=0) && strcmp(buff,"DATA")==0)
			{
				memset(buff2,0,sizeof(buff2));
				strcpy(buff2,"354 start mail input");
				tcp_write(cfd,buff2);
				break;
			}
			else
			{
				memset(buff2,0,sizeof(buff2));
				//cout<<"buff,no_of_recv,curr_recvr:"<<buff<<","<<no_of_recv<<","<<curr_recvr<<endl;
				strcpy(buff2,"Invalid Command (either no valid recepients)");
				tcp_write(cfd,buff2);
				return -1;
			}
		}
		if(rset==1)//Reseting buffers,sender,receiver and finally server
		{
			memset(buff,0,sizeof(buff));
			memset(buff2,0,sizeof(buff2));
			memset(sender,0,sizeof(sender));
			memset(recvr,0,sizeof(recvr));
			memset(recvr_id,0,sizeof(recvr_id));
			continue;
		}
		for(int i=0;i<no_of_recv;i++)
		{
			s2s_start_data(recvr[i]);
		}
		while(1)
		{
			memset(mesg,0,sizeof(mesg));
			tcp_read(cfd,mesg,1024);
			for(int i=0;i<no_of_recv;i++)
			{
				s2s_send_data(recvr[i],mesg);
			}
			if(strcmp(mesg,".")==0)
			{
				//cout<<"message received succesfully"<<endl;
				memset(buff2,0,sizeof(buff2));
				strcpy(buff2,"250 OK");
				tcp_write(cfd,buff2);
				break;
			}
			debugger.debug(mesg);
			strcat(mesg,"\n");
			// cout<<mesg;
			for(int i=0;i<curr_recvr;i++)
			{
				update(recvr_id[i],mesg,srv);
			}
		}
		for(int i=0;i<no_of_recv;i++)
		{
			s2s_quit(recvr[i]);
		}
	}
	close(cfd);
	//cout<<"Bye Bye client\n";
	debugger.debug("Bye Bye client");
	return 0;
}	

int env_client_from(char *from,int sfd)
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	strcpy(buff2,"MAIL FROM:");
	strcat(buff2,from);
	tcp_write(sfd,buff2);
	memset(buff,0,sizeof(buff));
	tcp_read(sfd,buff,BUFF_SZ);
	// cout<<buff<<endl;
	debugger.debug(buff);
	if(strcmp(buff,"250 OK")==0)
	{
		return 0;
	}
	else if(strcmp(buff,"550 No such user here")==0)
	{	
		return -3;
	}
	cout<<"Invalid Message Received\n";
	return -1;
}

int env_client_to(char *to,int sfd,int vrfy)
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	if(vrfy==0)
	{
		strcpy(buff2,"RCPT TO:");
		strcat(buff2,to);
	}
	else
	{
		strcpy(buff2,"VRFY:");
		strcat(buff2,to);
	}
	tcp_write(sfd,buff2);
	memset(buff,0,sizeof(buff));
	tcp_read(sfd,buff,BUFF_SZ);
	// cout<<buff<<endl;
	//debugger.debug(buff);
	// cout<<"DASDAS: "<<buff<<endl;
	if(strcmp(buff,"250 OK")==0)
	{
		// cout<<buff<<endl;
		debugger.debug(buff);
		return 0;
	}
	else if(strcmp(buff,"550 No such user here")==0)
	{
		// cout<<buff<<endl;
		if(vrfy==1)
			rset_toserver(sfd);
		// debugger.debug(buff);
		return -3;
	}
	else
	{
		// cout<<buff<<endl;
		rset_toserver(sfd);
		debugger.debug(buff);
		return -1;
	}
}

int start_data(int sfd) //from client side
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	strcpy(buff2,"DATA");
	tcp_write(sfd,buff2);
	memset(buff,0,sizeof(buff));
	tcp_read(sfd,buff,BUFF_SZ);
	// cout<<buff<<endl;
	debugger.debug(buff);
	if(strcmp(buff,"354 start mail input")!=0)
	{
		cout<<"Invalid Message Received\n";
		return -1;
	}
	return 0;
}

int rset_toserver(int sfd) //from client side
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	strcpy(buff2,"RSET");
	tcp_write(sfd,buff2);
	memset(buff,0,sizeof(buff));
	tcp_read(sfd,buff,BUFF_SZ);
	// cout<<buff<<endl;
	debugger.debug(buff);
	if(strcmp(buff,"250 OK")!=0)
	{
		cout<<"Invalid Message Received\n";
		return -1;
	}
	return 0;
}

int send_data(int sfd,char *data)
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	strcpy(buff2,data);
	tcp_write(sfd,buff2);
	if(strcmp(data,".")==0)
	{
		memset(buff,0,sizeof(buff));
		tcp_read(sfd,buff,BUFF_SZ);
		//cout<<"eneterd"<<endl;
		if(strcmp(buff,"250 OK")==0)
		{
			cout<<"Mail sent successfully!"<<endl;
			return 0;
		}
		else
		{
			cout<<"Mail Failure"<<endl;
			return -1;
		}
	}
	return -2;
}

// int end_data(int sfd)
// {
// 	char buff[BUFF_SZ]={'\0'};
// 	char buff2[BUFF_SZ]={'\0'};
// 	memset(buff2,0,sizeof(buff2));
// 	strcpy(buff2,".");
// 	tcp_write(sfd,buff2);
// 	memset(buff,0,sizeof(buff));
// 	tcp_read(sfd,buff,BUFF_SZ);
// 	if(strcmp(buff,"250 OK")==0)
// 	{
// 		cout<<"Mail sent successfully!"<<endl;
// 		return 0;
// 	}
// 	else
// 	{
// 		cout<<"Mail Failure"<<endl;
// 		return -1;
// }
int quit(int sfd)
{
	char buff[BUFF_SZ]={'\0'};
	char buff2[BUFF_SZ]={'\0'};
	memset(buff2,0,sizeof(buff2));
	strcpy(buff2,"QUIT");
	tcp_write(sfd,buff2);
	memset(buff,0,sizeof(buff));
	tcp_read(sfd,buff,BUFF_SZ);
	if(strcmp(buff,"221 service closed")==0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

// void command_vrfy(int sfd)
// {

// }

// void command_rset(int sfd)
// {

// }

int s2s_conn(char *from,char *to,char *ip1,char *ip2,int vrfy)
{
	char *p=strchr(to,'@');
	int sfd;
	char domain[BUFF_SZ];
	if(p)
		strcpy(domain,p+1);
	if(strcmp(domain,"abc.com")==0)
	{
		sfd=connect_client(ip1,port_num);
	}
	else
	{
		sfd=connect_client(ip2,port_num_2);
	}
	int tempo;
	tempo=initiate_client(domain,sfd);
	if(tempo==0)
	{
		int temp=env_client_from(from,sfd);
		//cout<<"I am temp from "<<temp<<endl;
		if(temp!=0)
		{
			//cout<<"Failed to connect to "<<to<<endl;
			return -1;
		}
		temp=env_client_to(to,sfd,vrfy);
		//cout<<"I am temp to "<<temp<<endl;
		if(temp==-3)
		{
			//cout<<"550 No such user here"<<":"<<to<<endl;
			return -3;
		}
		else if(temp==0)
		{
			//cout<<"I am sfd "<<sfd<<endl;
			return sfd;
		}
		else
		{
			//cout<<"Failed to connect to "<<to<<endl;
			return -1;
		}
	}
	else 
		return -1;
}

void s2s_start_data(int sfd)
{
	//cout<<sfd<<endl;
	start_data(sfd);
}

void s2s_send_data(int sfd,char *data)
{
	send_data(sfd,data);
}

void s2s_quit(int sfd)
{
	quit(sfd);
	close(sfd);
}

void s2s_rset(int sfd)
{
	rset_toserver(sfd);
	quit(sfd);
	close(sfd);
}

void debug_on()
{
	debugger.setDebug(true);
}

