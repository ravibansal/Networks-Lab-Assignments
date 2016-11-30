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

int port_num = 21436;
#define BUF_SIZE 100
#define HD_SIZE 100
#define CNT_SIZE 1000

void reporter(char *ip)
{
    printf("Connecting to %s....\n", ip);
    printf("Enter the reporter port\n");
    scanf("%d", &port_num);
    /*char ip[30]="127.0.0.1";
    int rst; // Return status of functions.
    int cfd; // File descriptor for the client.

    /**************** Create a socket. *******************************/
    int rst;
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
             * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
    if (sfd == -1)
    {
        perror ("Client: socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);



    /***************** Assign an address of the server **************/
    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
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


    char buf[BUF_SIZE];
    strcpy(buf, "reporter");
    rst = send (sfd, buf, BUF_SIZE, 0);
    /**************** Send-Receive messages ************************/
    while (1)
    {
        printf("Hello Reporter!!Welcome to InLong News Center\n");
        printf("Please enter the group: 1.Academic 2. Non-Academic\n");
        int gp;
        char buf[BUF_SIZE] = {'\0'};
        while (1)
        {
            scanf("%d", &gp);
            if (gp == 1)
            {
                strcpy(buf, "academic");
                break;
            }
            else if (gp == 2)
            {
                strcpy(buf, "non_academic");
                break;
            }
            else
            {
                printf("Please enter a valid input i.e. 1 or 2\n");
            }
        }
        rst = send (sfd, buf, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for group");
            exit (1);
        }

        //printf ("Sent message group bytes= %d\n", rst);
        char temp[BUF_SIZE] = {'\0'};
        rst = recv (sfd, temp, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for date");
            exit (1);
        }
        printf("%s", temp);
        char date[BUF_SIZE] = {'\0'};
        scanf("%s", date);
        rst = send (sfd, date, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for date");
            exit (1);
        }
        //printf ("Sent message date bytes= %d\n", rst);

        char head[HD_SIZE] = {'\0'};
        char temp1[BUF_SIZE] = {'\0'};
        rst = recv (sfd, temp1, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for date");
            exit (1);
        }
        printf("%s", temp1);
        //printf("Please enter the heading for the news in a single line\n");
        getchar();
        scanf("%[^\n]", head);
        /*int sz=strlen(head);
        if(head[sz-1]=='\n')
            head[sz-1]='\0';*/
        //printf("%s\n\n",head);
        rst = send (sfd, head, HD_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for heading");
            exit (1);
        }
        //printf("%s\n",head);
        //printf ("Sent message heading bytes= %d\n", rst);
        char content[CNT_SIZE];
        char temp2[BUF_SIZE] = {'\0'};
        rst = recv (sfd, temp2, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for date");
            exit (1);
        }
        printf("%s", temp2);
        //printf("Please enter the content for the news, use # for the end of content\n");
        scanf("%[^#]%*c", content);
        //printf("%s\n\n\n",content);
        rst = send (sfd, content, CNT_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for content");
            exit (1);
        }
        char temp_y[2];
        //printf("Do you want to continue: y or n\n");
        char temp3[BUF_SIZE] = {'\0'};
        rst = recv (sfd, temp3, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for date");
            exit (1);
        }
        printf("%s", temp3);
        getchar();
        scanf("%c", &temp_y[0]);
        temp_y[1] = '\0';
        while (temp_y[0] != 'y' && temp_y[0] != 'n')
        {
            printf("Please give a valid input i.e. y or n\n");
            getchar();
            scanf("%c", &temp_y[0]);
            printf("   %c\n", temp_y[0]);
        }
        //printf("%s\n\n\n",content);
        rst = send (sfd, temp_y, 2, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for content");
            exit (1);
        }
        if (temp_y[0] == 'n')
            break;
    }
    //printf ("Sent message content bytes= %d\n", rst);
    /****************** Close ****************************************/
    printf("GoodBye!!\n");
    rst = close (sfd); // Close the socket file descriptor.
    if (rst == -1)
    {
        perror ("Client close failed");
        exit (1);
    }
}

static struct termios old;
static struct termios _new;

/* Initialize _new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    _new = old; /* make _new settings same as old settings */
    _new.c_lflag &= ~ICANON; /* disable buffered i/o */
    _new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &_new); /* use these _new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void)
{
    return getch_(0);
}

/* Read 1 character with echo */
char getche(void)
{
    return getch_(1);
}
int admin(char *ip)
{
    printf("Enter the admin port\n");
    scanf("%d", &port_num);
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

    //printf("Please enter the password\n");
    //scanf("%s",pwd);

    //printf("%s\n",password);
    /* Sending message to the server. */
    while (1)
    {
        // char *password = getpass("Password: ");
        char password[30] = {'\0'};
        printf("Password: ");
        scanf("%s", password);
        //char password[20];
        //scanf("%s",password);
        //printf("Yes\n");
        rst = sendto (sfd, password, 30, flags, (struct sockaddr *) &dest_addr,
                      sizeof (struct sockaddr_in)); /* Value of rst is 20,
            /* on successful transmission; i.e. It has nothing to do with a
            * NULL terminated string.
            */
        if (rst < 0)
        {
            perror ("Client: Sendto function call failed");
            exit (1);
        }
        char pwd_y[2];
        //printf("Yes1\n");
        struct sockaddr_in sender_addr;
        socklen_t sender_len;
        int b_recv = recvfrom (sfd, pwd_y, 2, flags,
                               (struct sockaddr *) &sender_addr, &sender_len);
        if (b_recv == -1)
        {
            perror ("Server: recvfrom failed");
            exit (1);
        }
        //printf("Yes2\n");
        if (pwd_y[0] == 'n')
        {
            printf("Password is incorrect\n");
        }
        else
            break;
    }
    while (1)
    {
        int year, mon, dt, hr, min, sec;
        printf("Please enter the date and time(24 Hr) below befor which you want file deletion\n");
        printf("Year(YYYY): ");
        scanf("%d", &year);
        printf("Month(MM): ");
        scanf("%d", &mon);
        printf("Date(DD): ");
        scanf("%d", &dt);
        printf("Hour(HH): ");
        scanf("%d", &hr);
        printf("Min(MM): ");
        scanf("%d", &min);
        printf("Sec(SS): ");
        scanf("%d", &sec);
        char tm_stmp[30];
        sprintf(tm_stmp, "%d_%d_%d_%d_%d_%d", year - 1900, mon - 1, dt, hr, min, sec);
        rst = sendto (sfd, tm_stmp, 30, flags, (struct sockaddr *) &dest_addr,
                      sizeof (struct sockaddr_in)); /* Value of rst is 20,
            /* on successful transmission; i.e. It has nothing to do with a
            * NULL terminated string.
            */
        if (rst < 0)
        {
            perror ("Client: Sendto function call failed");
            exit (1);
        }
        else
        {
            printf ("Sent data size = %d\n", rst);
        }

        char temp_y[2];
        char temp1[BUF_SIZE];
        //printf("Do you want to continue: y or n\n");
        struct sockaddr_in sender_addr;
        socklen_t sender_len;
        int b_recv = recvfrom (sfd, temp1, BUF_SIZE, flags,
                               (struct sockaddr *) &sender_addr, &sender_len);
        if (b_recv == -1)
        {
            perror ("Server: recvfrom failed");
            exit (1);
        }
        printf("%s\n", temp1);
        getchar();
        scanf("%c", &temp_y[0]);
        temp_y[1] = '\0';
        while (temp_y[0] != 'y' && temp_y[0] != 'n')
        {
            printf("Please give a valid input i.e. y or n\n");
            getchar();
            scanf("%c", &temp_y[0]);
            //printf("   %c\n",temp_y[0]);
        }
        //printf("%s\n\n\n",content);
        rst = sendto (sfd, temp_y, 2, flags, (struct sockaddr *) &dest_addr,
                      sizeof (struct sockaddr_in));
        if (rst == -1)
        {
            perror ("Client: Send failed for content");
            exit (1);
        }
        if (temp_y[0] == 'n')
            break;
    }
    rst = close (sfd); // Close the socket file descriptor.
    if (rst < 0)
    {
        perror ("Client close failed");
        exit (1);
    }
    printf("GoodBye!!\n");
    return 0;

}
#define BUFF_LENGTH 2000

int connectToServer(char* ipaddress);
int tcp_read(int clientSocket, char* buff, int len);
int tcp_write(int clientSocket, char* message);
char* create_temp_file(char* buffer, int *fd);
int displayNews(char* buffer);

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

int tcp_write(int clientSocket, char* message)
{
    int rst = send (clientSocket, message, strlen(message), MSG_CONFIRM);
    if (rst == -1)
    {
        perror ("Client: Send failed");
        exit (-1);
    }
    return rst;
}

char* create_temp_file(char* buffer, int *fd)
{
    char *nameBuff  = (char*)malloc(32 * sizeof(char));
    memset(nameBuff, 0, sizeof(nameBuff));
    strncpy(nameBuff, "/tmp/newstmpfile-XXXXXX", 25);
    errno = 0;
    // Create the temporary file, this function will replace the 'X's
    int filedes = mkstemp(nameBuff);
    //i//nt filedes = open("test.txt",O_CREAT|O_WRONLY,0666);
    // Call unlink so that whenever the file is closed or the program exits
    // the temporary file is deleted
    //

    if (filedes < 0)
    {
        perror("Error in creating temp file");
        return (NULL);
    }

    errno = 0;
    // Write some data to the temporary file
    if (-1 == write(filedes, buffer, strlen(buffer)))
    {
        perror("Error in writing to file");
        return (NULL);
    }
    *fd = filedes;
    return nameBuff;
}

int displayNews(char* buffer) {
    int fd;
    char *tmp = create_temp_file(buffer, &fd);
    if (tmp == NULL) {
        return -1;
    }
    int pid = fork();
    if (pid == 0)
    {
        char* argv[] = {"xterm", "-hold", "-e", "more", tmp, NULL};
        execvp("xterm", argv);
        exit(0);
    }
    else {
        //sleep(2);
        //printf("%s\n",buffer);
        //unlink(tmp);
        close(fd);
    }
    return 0;
}

int reader(char *ipaddress)
{
    printf("Enter the reader port\n");
    scanf("%d", &port_num);
    int clientSocket = connectToServer(ipaddress);
    char read_buff[BUFF_LENGTH] = {'\0'};
    char write_buff[BUFF_LENGTH] = {'\0'};
    if (tcp_write(clientSocket, "reader") < 0)
        return -1;
    int num = tcp_read(clientSocket, read_buff, BUFF_LENGTH);
    if (num < 0)
    {
        return -1;
    }
    printf("%s\n", read_buff);
    while (1)
    {

        printf("Enter 1 for Academic and 2 for Non-Academic(# to quit): ");
        char ch[10];
        scanf("%s", ch);
        //getchar();
        write_buff[0] = ch[0];
        write_buff[1] = '\0';
        fprintf(stderr, "Input read: %c\n", ch[0]);

        if (tcp_write(clientSocket, write_buff) < 0) return -1;
        if (ch[0] == '#')
            break;
        int id;
        if (tcp_read(clientSocket, read_buff, BUFF_LENGTH) < 0) return -1;
        printf("%s", read_buff);
        printf("Enter the news article number to read (0 to exit):");
        scanf("%d", &id);
        sprintf(write_buff, "%d", id);
        if (tcp_write(clientSocket, write_buff) < 0)return -1;
        if (id == 0)
            return (0);
        if (tcp_read(clientSocket, read_buff, BUFF_LENGTH) < 0) return -1;
        //printf("Message got: %s\n", read_buff);
        if (displayNews(read_buff) < 0) return -1;
    }
    return 0;
}
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error: Usage: reporter serverIP\n");
        exit(1);
    }
    char* ipaddress = argv[1];
    int choice = 4;
    do
    {
        printf("--------------Welcome to InLong News Center--------------------\n");
        printf("Press 1 to use as a reporter\n");
        printf("Press 2 to use as a reader\n");
        printf("Press 3 to use as an admin\n");
        printf("Press 4 to exit\n");

        scanf("%d", &choice);
        switch (choice)
        {
        case 1: reporter(ipaddress);
            break;
        case 2: reader(ipaddress);
            break;
        case 3: admin(ipaddress);
            break;
        case 4: break;
        default: printf("Invalid Choice\n");
        }
    }
    while (choice != 4);
    printf("Byeee!! Have a good day\n");
    return 0;
}