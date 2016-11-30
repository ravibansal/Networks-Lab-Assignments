#ifndef SMTP_H
#define SMTP_H

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
#include "debug.h"
// #include <iostream>


using namespace std;
#define BUFF_SZ 512



int tcp_read(int Socket, char* buff, int len);
int tcp_write(int Socket, char* message);
bool isCharacter(const char Character);
bool isNumber(const char Character);
bool isValidEmailAddress(const char * EmailAddress);
int connect_client(char *ip,int port_num);
int connect_server(char *ip,int port_num);
int server_accept(int sfd);
int initiate_client(char *id,int sfd);
void RemoveSpaces(char* source);
int search(char *id);
int check_valid(char *id,char *domain);
int check_valid_rcpt(char *id,char *domain,char *ip1,char *ip2,int vrfy);
void initialize_data_abc();
void initialize_data_xyz();
int verify(char *domain);
int initiate_server(char* domain,int cfd);
int env_server(char *srv,int cfd,char *ip1,char *ip2);
int env_client_from(char *from,int sfd);
int env_client_to(char *to,int sfd);
int rset_toserver(int sfd);
int start_data(int sfd);
int send_data(int sfd,char* data);
int quit(int sfd);
int checkStatus(char *buff,int);

void debug_on();
#endif

