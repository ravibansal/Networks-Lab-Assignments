#include "smtp.h"

using namespace std;

char data[100][60];  //for storing email address
char sender[BUFF_SZ];
int recvr[100]; //To store sfd's for relaying to other server
int recvr_id[100];//To store the id in sql table to store the message for this domain
int no_of_recv = 0; //Other domain receiver
int curr_recvr = 0; //This domain receiver

int port_num = 25;
int port_num_2 = 25000;
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
    //printf("Recv:%s",buff);
    //fprintf(stderr, "Read from client %s\n", buff);
    return rst;
}

int tcp_write(int Socket, char* message)
{
    //printf("Sending %s\n",message);
    int len = strlen(message);
    debugger.debug(message);
    int rst = send(Socket, message, len, MSG_CONFIRM);
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
    if (!EmailAddress) // If cannot read the Email Address...
        return 0;
    if (!isCharacter(EmailAddress[0])) // If the First character is not A-Z, a-z
        return 0;
    int AtOffset = -1;
    int DotOffset = -1;
    unsigned int Length = strlen(EmailAddress); // Length = StringLength (strlen) of EmailAddress
    for (unsigned int i = 0; i < Length; i++)
    {
        if (EmailAddress[i] == '@') // If one of the characters is @, store it's position in AtOffset
            AtOffset = (int)i;
        else if (EmailAddress[i] == '.') // Same, but with the dot
            DotOffset = (int)i;
    }
    if (AtOffset == -1 || DotOffset == -1) // If cannot find a Dot or a @
        return 0;
    if (AtOffset > DotOffset) // If the @ is after the Dot
        return 0;
    return !(DotOffset >= ((int)Length - 1)); //Chech there is some other letters after the Dot
}


void RemoveSpaces(char* source)
{
    char* i = source;
    char* j = source;
    while (*j != 0)
    {
        *i = *j++;
        if (*i != ' ')
            i++;
    }
    *i = 0;
}
int search(char *id)
{
    char *p = strchr(id, '@');
    //cout<<id<<"    "<<p<<endl;
    *p = 0;
    for (int i = 0; i < 4; i++)
    {
        if (strcmp(data[i], id) == 0)
            return 0;
    }
    return -1;
}


int check_valid(char *id, char *domain)
{
    RemoveSpaces(id);
    if (strcmp(id, domain) != 0)
        return -1;
    else
        return 0;
}
int verify(char *domain)
{
    char temp[BUFF_SZ];
    strcpy(temp, sender);
    char *p = strchr(temp, '@');
    // cout<<domain<<" is "<<(p+1)<<endl;
    if (p)
    {
        if (strcmp(domain, p + 1) != 0)
        {
            return 0;
        }
        *p = 0;
    }
    //cout<<temp<<endl;
    for (int i = 0; i < 4; i++)
    {
        if (strcmp(temp, data[i]) == 0)
        {
            return 0;
        }
    }
    return -1;
}

int connect_client(char *ip, int port_num)
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
    if (rst == -1)
        return -1;
    else
        return sfd;
}


int initiate_client(char *id, int sfd) //it will return the message that it will receive from server
{
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff, 0, sizeof(buff));
    tcp_read(sfd, buff, BUFF_SZ);
    //cout<<buff<<endl;
    debugger.debug(buff);

    if (checkStatus(buff,220) != 0)
    {
        return  -1;
    }

    while (1)
    {
        memset(buff2, 0, sizeof(buff2));
        sprintf(buff2,"HELO %s\r\n",id);
        tcp_write(sfd, buff2);
        memset(buff, 0, sizeof(buff));
        tcp_read(sfd, buff, BUFF_SZ);
        if (checkStatus(buff,250) == 0)
        {
            break;
        }
    }
    if (checkStatus(buff,250) == 0)
        return 0;
    else
        return -1;
    return 0;
}

int checkStatus(char *buff, int status)
{
    int r;
    sscanf(buff, "%d", &r);
    if (r == status)
        return 0;
    else
    {
        //printf("Error from server:%d %d%s\n",r,status, buff);
        return -1;
    }
}


int env_client_from(char *from, int sfd)
{
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff2, 0, sizeof(buff2));

    sprintf(buff2,"MAIL FROM:%s\r\n",from);

    tcp_write(sfd, buff2);
    
    memset(buff, 0, sizeof(buff));
    tcp_read(sfd, buff, BUFF_SZ);
    // cout<<buff<<endl;
    debugger.debug(buff);
    if (checkStatus(buff,250) == 0)
    {
        return 0;
    }
    else if (checkStatus(buff,550) == 0)
    {
        return -3;
    }
    cout << "Invalid Message Received\n";
    return -1;
}

int env_client_to(char *to, int sfd)
{
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff2, 0, sizeof(buff2));
    
    sprintf(buff2,"RCPT TO:%s\r\n",to);

    tcp_write(sfd, buff2);
    memset(buff, 0, sizeof(buff));
    tcp_read(sfd, buff, BUFF_SZ);
    // cout<<buff<<endl;
    //debugger.debug(buff);
    // cout<<"DASDAS: "<<buff<<endl;
    if (checkStatus(buff,250) == 0)
    {
        // cout<<buff<<endl;
        debugger.debug(buff);
        return 0;
    }
    else if (checkStatus(buff,550) == 0)
    {
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
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff2, 0, sizeof(buff2));
    strcpy(buff2, "DATA\r\n");
    tcp_write(sfd, buff2);
    memset(buff, 0, sizeof(buff));
    tcp_read(sfd, buff, BUFF_SZ);
    // cout<<buff<<endl;
    debugger.debug(buff);
    if (checkStatus(buff,354) != 0)
    {
        cout << "Invalid Message Received\n";
        return -1;
    }
    return 0;
}


int send_data(int sfd, char *data)
{
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff2, 0, sizeof(buff2));
    sprintf(buff2,"%s\r\n",data);
    //strcpy(buff2, data);
    if (strcmp(data, ".") == 0)
    {
        sprintf(buff2,"\r\n.\r\n");
    }
    tcp_write(sfd, buff2);
    if (strcmp(data, ".") == 0)
    {
        memset(buff, 0, sizeof(buff));
        tcp_read(sfd, buff, BUFF_SZ);
        //printf("Reply%s\n",buff );
        if (checkStatus(buff,250) == 0)
        {
            cout << "Mail sent successfully!" << endl;
            return 0;
        }
        else if (checkStatus(buff,550) == 0)
        {
            cout << "Unknown user" << endl;
            return -1;
        }
        else
        {
            cout << "Mail Failuredd" << endl;
            return -3;
        }
    }
    return -2;
}

int quit(int sfd)
{
    char buff[BUFF_SZ] = {'\0'};
    char buff2[BUFF_SZ] = {'\0'};
    memset(buff2, 0, sizeof(buff2));
    strcpy(buff2, "QUIT\r\n");
    tcp_write(sfd, buff2);
    memset(buff, 0, sizeof(buff));
    tcp_read(sfd, buff, BUFF_SZ);
    if (checkStatus(buff,221) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int rset_toserver(int sfd) //from client side
{
    char buff[BUFF_SZ]={'\0'};
    char buff2[BUFF_SZ]={'\0'};
    memset(buff2,0,sizeof(buff2));
    strcpy(buff2,"RSET\r\n");
    tcp_write(sfd,buff2);
    memset(buff,0,sizeof(buff));
    tcp_read(sfd,buff,BUFF_SZ);
    // cout<<buff<<endl;
    debugger.debug(buff);
    if(checkStatus(buff,250) != 0)
    {
        cout<<"Invalid Message Received\n";
        return -1;
    }
    return 0;
}

void debug_on()
{
    debugger.setDebug(true);
}

