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


#define BUFF_LENGTH 2000
int port_num = 23465;
#define BUF_SIZE 100
#define HD_SIZE 100
#define CNT_SIZE 1000


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

int getIndex(char* str, char c)
{
    int i = 1;
    char last = str[0];
    while (str[i] != '\0')
    {
        if (last == '#' && str[i] == '#')
        {
            return i;
        }
        last = str[i];
        i++;
    }
    return -1;
}

int sortByTime(const struct dirent **a, const struct dirent **b)
{
    return strverscmp((*a)->d_name, (*b)->d_name);
}


int server_ls(char *pathname, char * buff, int len, char* list[])
{
    memset(buff, 0, len);
    struct dirent **files;
    int i, count;
    count = scandir(pathname, &files, NULL, sortByTime);
    buff[0] = '\0';
    /* If no files found, make a non-selectable menu item */
    int cnt = 0;
    if (count <= 0)
        sprintf(buff, "No news article present\n");
    else
    {
        char temp[BUFF_LENGTH];
        for (i = count - 1; i >= 0; i--)
        {
            if (strcmp(files[i]->d_name, ".") == 0 || strcmp(files[i]->d_name, "..") == 0)
                continue;
            if (files[i]->d_name[0] == '.')
                continue;
            char* name = files[i]->d_name;
            int k = getIndex(name, '#');
            if (k == -1)
            {
                printf("Error in file name\n");
                exit(0);
            }
            name = name + k + 1;
            //printf("Filename orig = %s mine = %s\n", files[i]->d_name, name);
            sprintf(temp, "%d\t%s\n", cnt + 1, name);
            strcat(buff, temp);
            list[cnt] = (char *)malloc((strlen(files[i]->d_name) + 2) * sizeof(char));
            list[cnt][0] = '\0';
            strcpy(list[cnt], files[i]->d_name);
            //printf("Filename orig = %s mine = %s\n", files[i]->d_name, list[cnt]);
            cnt++;
            free(files[i]);
        }
        free(files);
    }
    if (cnt <= 0)
        sprintf(buff, "No news article present\n");
    return cnt;
}

int  write_article_list(int clientSocket, int choice, char *list[])
{
    char buff[BUFF_LENGTH];

    int cnt = 0;
    switch (choice)
    {
    case 1: cnt = server_ls("dump/academic", buff, BUFF_LENGTH, list);
        break;
    case 2: cnt = server_ls("dump/non_academic", buff, BUFF_LENGTH, list);
        break;
    default: sprintf(buff, "Please select a valid option(1 for Academic and 0 for Non-Academic)\nChoose any number less than 0 to continue\n");
        break;
    }
    tcp_write(clientSocket, buff);
    //tcp_write(clientSocket,"$$end$$");
    return cnt;
}

void transfer_news(int clientSocket, int id, int choice, char* list[]) {
    //printf("Here: id = %d, filename=%s\n", id, list[id - 1]);
    char * filename = list[id - 1];
    char buff[BUFF_LENGTH];
    memset(buff, 0, BUFF_LENGTH);
    buff[0] = '\0';
    if (choice == 1)
    {
        strcat(buff, "dump/academic/");
        strcat(buff, filename);
        //sprintf(buff,"dump/academic/%s",filename);
    }
    else
    {
        strcat(buff, "dump/non_academic/");
        strcat(buff, filename);
        //sprintf(buff,"dump/non_academic/%s",filename);
    }
    //printf("Here: id = %d, filename=%s\n", id, buff);
    FILE *fp = fopen(buff, "r");
    if (fp == NULL) {
        tcp_write(clientSocket, "The news no longer exists");
        return;
    }
    int i = 0;
    char c;
    //scanf("%d",&i);
    memset(buff, 0, BUFF_LENGTH);
    buff[0] = '\0';
    i = 0;
    while (1)
    {
        c = fgetc(fp);
        if (feof(fp))
            break;
        buff[i++] = c;
    }
    buff[i] = '\0';
    fclose(fp);
    //printf("News transf: %s\n", buff);
    tcp_write(clientSocket, buff);
}

void reader(int clientSocket)
{
    char msg[] = "Welcome to InLong News Center\nThere are two news group\n\t1. Academic\n\t2. Non Academic";
    tcp_write(clientSocket, msg);
    while (1)
    {
        char read_buffer[BUFF_LENGTH];
        char *file_list[1000];
        tcp_read(clientSocket, read_buffer, BUFF_LENGTH);
        int choice;
        if(read_buffer[0] == '#')
            return;
        sscanf(read_buffer, "%d", &choice);
        int cnt = write_article_list(clientSocket, choice, file_list);
        int i;
        /*for (i = 0; i < cnt; i++)
        {
            printf("%s\n", file_list[i]);
        }*/
        tcp_read(clientSocket, read_buffer, BUFF_LENGTH);
        int id = 0;
        sscanf(read_buffer, "%d", &id);
        printf("Reader asking for news article:%d %d\n", id, cnt);
        if (id == 0)
        {
            tcp_write(clientSocket, "Thanks for visiting!!Visit Again\n");
            break;

        }
        else if (id > 0 && id <= cnt)
        {
            transfer_news(clientSocket, id, choice, file_list);
        }
        else
        {
            char buff[BUFF_LENGTH];
            buff[0] = '\0';
            strcpy(buff, "No such news present\n");
            tcp_write(clientSocket, buff);
        }
    }
    printf("Child ending\n");
    exit(0);

}


void reporter(int cfd)
{
    int rst;
    while (1)
    {
        char buf[BUF_SIZE] = {'\0'};
        rst = recv (cfd, buf, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for group");
            exit (1);
        }
        printf("%s:\n", buf);
        char temp[BUF_SIZE] = "Please enter the date in the form of DD/MM/YYY\n";
        rst = send (cfd, temp, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for temp");
            exit (1);
        }
        char date[BUF_SIZE] = {'\0'};
        rst = recv (cfd, date, BUF_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for date");
            exit (1);
        }
        printf("Date: %s\n", date);
        char temp1[BUF_SIZE] = "Please enter the heading for the news in a single line\n";
        rst = send (cfd, temp1, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for temp1");
            exit (1);
        }
        char heading[HD_SIZE] = {'\0'};
        rst = recv (cfd, heading, HD_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for heading");
            exit (1);
        }
        printf("Heading: %s\n", heading);
        char temp2[BUF_SIZE] = "Please enter the content for the news, use # for the end of content\n";
        rst = send (cfd, temp2, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for temp1");
            exit (1);
        }
        char content[CNT_SIZE] = {'\0'};
        rst = recv (cfd, content, CNT_SIZE, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for content");
            exit (1);
        }
        printf("Content:  %s\n", content);
        time_t t1 = time(NULL);
        struct tm k = *localtime(&t1);
        FILE *fp;
        char fl_nm[1500] = {'\0'};
        //sprintf(str, "%d", aInt);
        sprintf(fl_nm, "dump/%s/%d_%d_%d_%d_%d_%d##", buf, k.tm_year, k.tm_mon, k.tm_mday, k.tm_hour, k.tm_min, k.tm_sec);
        //+"_"++"_"++"_"++"_"++"_"++"##";
        strcat(fl_nm, heading);
        fp = fopen(fl_nm, "w");
        if (fp != NULL)
        {
            //fprintf(fp,"%s:\n",buf);
            fprintf(fp, "Date: %s\n", date);
            fprintf(fp, "Heading: %s\n\n", heading );
            fprintf(fp, "%s\n", content);
            fclose(fp);
        }
        else
        {
            perror("File was not opened properly");
            exit(1);
        }
        char temp_y[2];
        char temp3[BUF_SIZE] = "Do you want to continue: y or n\n";
        rst = send (cfd, temp3, BUF_SIZE, 0);
        if (rst == -1)
        {
            perror ("Client: Send failed for temp1");
            exit (1);
        }
        rst = recv (cfd, temp_y, 2, 0);
        if (rst == -1 || rst == 0)
        {
            perror ("Server recv failed for content");
            exit (1);
        }
        if (temp_y[0] == 'n')
            break;
    }
    exit(0);
}


int TCPServer(char *ipaddress, int port_num)
{
    int rst; // Return status of functions.
    int cfd; // File descriptor for the client.

    /**************** Create a socket. *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_STREAM, 0); /* AF_INET --> IPv4,
                * SOCK_STREAM --> TCP Protocol, 0 --> for the protocol. */
    if (sfd == -1)
    {
        perror ("Server: socket error");
        exit (1);
    }
    printf ("Socket fd = %d\n", sfd);



    /***************** Assign an address to the server **************/
    struct sockaddr_in srv_addr, cli_addr; // Addresses of the server and the client.
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.

    // Clear the two addresses.
    memset (&srv_addr, 0, addrlen);
    memset (&cli_addr, 0, addrlen);

    // Assign values to the server address.
    srv_addr.sin_family = AF_INET; // IPv4.
    srv_addr.sin_port   = htons (port_num); // Port Number.

    rst = inet_pton (AF_INET, ipaddress, &srv_addr.sin_addr); /* To
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
    printf ("Max connections allowed to wait: %d\n", SOMAXCONN);
    rst = listen (sfd, SOMAXCONN);
    if (rst == -1)
    {
        perror ("Server: Listen failed");
        exit (1);
    }

    while (1)
    {
        /***************** accept (): Waiting for connections ************/
        cfd = accept (sfd, (struct sockaddr *) &cli_addr, &addrlen); /*
            * Returns the file descriptor for the client.
            * Stores the address of the client in cli_addr. */
        if (cfd == -1)
        {
            perror ("Server: Accept failed");
            exit (1);
        }
        if (fork() == 0)
        {
            close(sfd);
            printf("Connected to %d\n", cfd);
            char buff[BUFF_LENGTH];
            int cond = 1;
            while (cond)
            {
                cond = 0;
                tcp_read(cfd, buff, BUFF_LENGTH);
                if (strcmp(buff, "reader") == 0)
                    reader(cfd);
                else if (strcmp(buff, "reporter") == 0)
                    reporter(cfd);
                else
                {
                    tcp_write(cfd, "Invalid identity!! Please try Again");
                    cond = 1;
                }
            }
            close(cfd);
            exit(0);
        }
        close(cfd);
    }
    return 0;

}

int sh_mkdir(char *s)
{
    int flag, mode;
    mode = 0777;
    flag = mkdir(s, mode);
    if (flag == -1)
    {
        char buf[1024];
        buf[0] = '\0';
        strcat(buf, "mkdir: cannot create directory '");
        strcat(buf, s);
        strcat(buf, "'");
        perror(buf);
        exit(1);
    }
    return 0;
}

void createDirectories(char * direc)
{
    DIR* dir = opendir(direc);
    if (dir)
    {
        closedir(dir);
    }
    else {
        sh_mkdir(direc);
    }

}


int UDPServer(char* ipaddress, int port_num)
{

    int rst; // Return status of functions.
    /***************** Create a socket *******************************/
    int sfd; // Socket file descriptor.
    sfd = socket (AF_INET, SOCK_DGRAM, 0);
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
    //printf("No\n");




    /****************** Send - receive messages **********************/
    /* Buffer for receiving a message. */
    char buf[BUF_SIZE] = {'\0'};
    int b_recv   = 0; // Number of bytes received.

    // Flags for recvfrom.
    int flags = 0;
    while (1)
    {
        while (1)
        {
            int bo = 0;
            // printf("Yes\n");
            b_recv = recvfrom (sfd, buf, BUF_SIZE, flags,
                               (struct sockaddr *) &cl_addr, &addrlen);
            if (b_recv == -1)
            {
                perror ("Server: recvfrom failed");
                exit (1);
            }
            else if (b_recv == 0)
            {
                bo = 1;
                break;
            }
            buf[b_recv - 1] = '\0';
            char pwd_y[2];
            int flg = 0;
            if (strcmp(buf, "cse13") == 0)
            {
                pwd_y[0] = 'y';
                flg = 1;

            }
            else
            {
                pwd_y[0] = 'n';
            }
            // printf("Yes1\n");
            pwd_y[1] = '\0';
            //rst = sendto (sfd, pwd_y, 2, flags, (struct sockaddr *) &cl_addr, sizeof (struct sockaddr_in));
            rst = sendto (sfd, pwd_y, 2, flags,
                          (struct sockaddr *) &cl_addr,
                          sizeof (struct sockaddr_in));
            if (rst < 0)
            {
                perror ("Client: Sendto function call failed");
                exit (1);
            }
            else if (rst == 0)
            {
                bo = 1;
                break;
            }
            if (flg)
                break;
            if (bo == 1)
                continue;
        }
        // printf("Yes2\n");
        b_recv = recvfrom (sfd, buf, BUF_SIZE, flags,
                           (struct sockaddr *) &cl_addr, &addrlen);
        if (b_recv == -1)
        {
            perror ("Server: recvfrom failed");
            exit (1);
        }
        else if (b_recv == 0)
            continue;
        buf[b_recv - 1] = '\0';
        //int bot=0;
        while (1)
        {
            int yy = 0, mm = 0, dd = 0, hh = 0, mi = 0, ss = 0;
            int i = 0;
            //printf("%s\n",buf);
            while (buf[i] != '\0')
            {
                while (buf[i] != '_' && buf[i] != '\0')
                {
                    yy = yy * 10 + (buf[i] - '0');
                    i++;
                }
                i++;
                while (buf[i] != '_' && buf[i] != '\0')
                {
                    mm = mm * 10 + (buf[i] - '0');
                    i++;
                }
                i++;
                while (buf[i] != '_' && buf[i] != '\0')
                {
                    dd = dd * 10 + (buf[i] - '0');
                    i++;
                }
                i++;
                while (buf[i] != '_' && buf[i] != '\0')
                {
                    hh = hh * 10 + (buf[i] - '0');
                    i++;
                }
                i++;
                while (buf[i] != '_' && buf[i] != '\0')
                {
                    mi = mi * 10 + (buf[i] - '0');
                    i++;
                }
                i++;
                while (buf[i] != '\0')
                {
                    ss = ss * 10 + (buf[i] - '0');
                    i++;
                }
            }
            //printf ("Message received = |%s|\n", buf);
            char rem[1500];
            /* Printing client's address. */
            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir ("dump/academic")) != NULL) {
                /* print all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL) {
                    if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0)
                    {
                        i = 0;
                        int yy2 = 0, mm2 = 0, dd2 = 0, hh2 = 0, mi2 = 0, ss2 = 0;
                        char p[1500];
                        strcpy(p, ent->d_name);
                        //printf("%s\n", p);
                        while (p[i] != '\0' && p[i] != '#')
                        {
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                yy2 = yy2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                mm2 = mm2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                dd2 = dd2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                hh2 = hh2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                mi2 = mi2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '#' && p[i] != '\0')
                            {
                                ss2 = ss2 * 10 + (p[i] - '0');
                                i++;
                            }
                        }
                        printf("to search--> %d %d %d %d %d %d\n", yy, mm, dd, hh, mi, ss);
                        printf("file--> %d %d %d %d %d %d\n", yy2, mm2, dd2, hh2, mi2, ss2);
                        int flag = 0;
                        if (yy2 <= yy)
                        {
                            if (yy2 < yy)
                                flag = 1;
                            else if (mm2 <= mm)
                            {
                                if (mm2 < mm)
                                    flag = 1;
                                else if (dd2 <= dd)
                                {
                                    if (dd2 < dd)
                                        flag = 1;
                                    else if (hh2 <= hh)
                                    {
                                        if (hh2 < hh)
                                            flag = 1;
                                        else if (mi2 <= mi)
                                        {
                                            if (mi2 < mi)
                                                flag = 1;
                                            else if (ss2 < ss)
                                            {
                                                flag = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (flag == 1)
                        {
                            sprintf(rem, "dump/academic/%s", ent->d_name);
                            // sh_rmdir(rem);
                            int status = unlink(rem);

                            if ( status == 0 )
                                printf("%s file deleted successfully.\n", rem);
                            else
                            {
                                printf("Unable to delete the file\n");
                                perror("Error");
                            }
                        }
                    }
                }
                closedir (dir);
            }
            else {
                /* could not open directory */
                perror ("error in deleting files");
                exit(1);
            }
            if ((dir = opendir ("dump/non_academic")) != NULL) {
                /* print all the files and directories within directory */
                while ((ent = readdir (dir)) != NULL) {
                    if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0)
                    {
                        i = 0;
                        int yy2 = 0, mm2 = 0, dd2 = 0, hh2 = 0, mi2 = 0, ss2 = 0;
                        char p[1500];
                        strcpy(p, ent->d_name);
                        //printf("%s\n", p);
                        while (p[i] != '\0' && p[i] != '#')
                        {
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                yy2 = yy2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                mm2 = mm2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                dd2 = dd2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                hh2 = hh2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '_' && p[i] != '\0')
                            {
                                mi2 = mi2 * 10 + (p[i] - '0');
                                i++;
                            }
                            i++;
                            while (p[i] != '#' && p[i] != '\0')
                            {
                                ss2 = ss2 * 10 + (p[i] - '0');
                                i++;
                            }
                        }
                        printf("to search--> %d %d %d %d %d %d\n", yy, mm, dd, hh, mi, ss);
                        printf("file--> %d %d %d %d %d %d\n", yy2, mm2, dd2, hh2, mi2, ss2);
                        int flag = 0;
                        if (yy2 <= yy)
                        {
                            if (yy2 < yy)
                                flag = 1;
                            else if (mm2 <= mm)
                            {
                                if (mm2 < mm)
                                    flag = 1;
                                else if (dd2 <= dd)
                                {
                                    if (dd2 < dd)
                                        flag = 1;
                                    else if (hh2 <= hh)
                                    {
                                        if (hh2 < hh)
                                            flag = 1;
                                        else if (mi2 <= mi)
                                        {
                                            if (mi2 < mi)
                                                flag = 1;
                                            else if (ss2 < ss)
                                            {
                                                flag = 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (flag == 1)
                        {
                            sprintf(rem, "dump/non_academic/%s", ent->d_name);
                            // sh_rmdir(rem);
                            int status = unlink(rem);

                            if ( status == 0 )
                                printf("%s file deleted successfully.\n", rem);
                            else
                            {
                                printf("Unable to delete the file\n");
                                perror("Error");
                            }
                        }
                    }
                }
                closedir (dir);
            }
            else {
                /* could not open directory */
                perror ("error in deleting files");
                exit(1);
            }
            char temp_y[2];
            char temp1[BUF_SIZE] = "Do you want to continue: y or n";
            rst = sendto (sfd, temp1, BUF_SIZE, flags,
                          (struct sockaddr *) &cl_addr,
                          sizeof (struct sockaddr_in));
            if (rst < 0)
            {
                perror ("Client: Sendto function call failed");
                exit (1);
            }

            b_recv = recvfrom (sfd, temp_y, 2, flags,
                               (struct sockaddr *) &cl_addr, &addrlen);
            if (b_recv == -1)
            {
                perror ("Server: recvfrom failed");
                exit (1);
            }
            else if (b_recv == 0)
            {
                // bot=1;
                break;
            }
            temp_y[1] = '\0';
            if (temp_y[0] == 'n')
            {
                break;
            }
            else
            {
                b_recv = recvfrom (sfd, buf, BUF_SIZE, flags,
                                   (struct sockaddr *) &cl_addr, &addrlen);
                if (b_recv == -1)
                {
                    perror ("Server: recvfrom failed");
                    exit (1);
                }
                else if (b_recv == 0)
                    break;
                buf[b_recv - 1] = '\0';
            }
        }
    }

    /****************** Close ****************************************/
    rst = close (sfd); // Close the socket file descriptor.
    if (rst <= 0)
    {
        perror ("Server close failed");
        exit (1);
    }
    return 0;
}

void myexit(int signo)
{
    killpg(0, signo);
}
int main(int argc, char* argv[])
{
    signal(SIGQUIT, myexit);
    signal(SIGTERM, myexit);
    signal(SIGKILL, myexit);
    char ipaddress[50];
    int tcp_port, udp_port;
    printf("Enter IP Addess\n");
    scanf("%s", ipaddress);
    printf("Enter TCP port:");
    scanf("%d", &tcp_port);
    printf("Enter UDP port:");
    scanf("%d", &udp_port);
    createDirectories("dump");
    createDirectories("dump/academic");
    createDirectories("dump/non_academic");
    if (fork() == 0)
    {
        TCPServer(ipaddress, tcp_port);
        exit(0);
    }
    else
    {
        if (fork() == 0)
        {
            UDPServer(ipaddress, udp_port);
            exit(0);
        }
    }
    int status = 0;
    wait(&status);
    wait(&status);
    return 0;


}