#ifndef RLTP_H
#define RLTP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <iostream>

using namespace std;

#define IP4_HDRLEN (sizeof(struct iphdr))
#define DEF_TIMEOUT 5000
#define MAX_RETRY 3


#define ACK 1
#define FIN 2
#define SYN 3
#define DATA 4
#define SYN_ACK 5
#define FIN_ACK 6

struct rltp_header
{
    unsigned short int source_port;
    unsigned short int dest_port;
    unsigned short int type;
    unsigned int seq_no;
    unsigned int ack_no;
    unsigned int checksum;

    void print();
    void printh();
};
#define RTLPHDR_LEN (sizeof(rltp_header))


class rltp
{
    static const int MOD_ADLER;
    int sockfd;
    char *sourceip,*destip;
    short int src_port,destport;
    struct sockaddr_in src_addr,dest_addr;
    bool connected;
	int RTLP_TIMEOUT;
    unsigned int src_seq_no, dest_seq_no;

public:
    rltp(char *sourceip,short int srcport,int timeout=DEF_TIMEOUT);
    void set_dest(char *clientip,short int port);
    void connect();
    void accept_connect();
    void close();
    void accept_close();
    int rtlp_send_message(string message);
    char* rtlp_receive_message(int* status);


protected:
    unsigned int calc_checksum(rltp_header* rltphdr,uint8_t* data, size_t datalen);
    int verify_checksum(uint8_t* data, size_t datalen);
    uint8_t* send_SYN(size_t* len);
    uint8_t* send_FIN(size_t* len);
    int recv_SYN_ACK();
    int recv_SYN();
    int recv_ACK(uint16_t type=ACK);
    uint8_t* send_ACK(size_t *len, uint16_t type);
    struct sockaddr_in get_saddr(char *sourceip,int port);
    struct iphdr get_iphdr(int msg_len=0);
    uint8_t * allocate_ustrmem (int len);
    int send_message(uint8_t* packet, size_t len);
    int recv_message(uint8_t* packet);

private:
    int create_socket();
};

#endif // RLTP_H
