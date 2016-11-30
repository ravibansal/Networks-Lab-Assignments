#include "rltp.h"


const int rltp::MOD_ADLER = 65521;

void rltp_header::print()
{
    cerr<<"Source Port:"<<source_port<<endl;
    cerr<<"Dest Port:"<<dest_port<<endl;
    cerr<<"Type:"<<type<<endl;
    cerr<<"seq_no:"<<seq_no<<endl;
    cerr<<"ack_no:"<<ack_no<<endl;
    cerr<<"checksum"<<checksum<<endl;
}

void rltp_header::printh()
{
    cerr<<"Source Port:"<<ntohs(source_port)<<endl;
    cerr<<"Dest Port:"<<ntohs(dest_port)<<endl;
    cerr<<"Type:"<<ntohs(type)<<endl;
    cerr<<"seq_no:"<<ntohl(seq_no)<<endl;
    cerr<<"ack_no:"<<ntohl(ack_no)<<endl;
    cerr<<"checksum"<<ntohl(checksum)<<endl;
}


rltp::rltp(char* sourceip,short int srcport,int timeout):sockfd(0),sourceip(sourceip),
    src_port(srcport),destport(0),src_seq_no(0),dest_seq_no(0),connected(false),RTLP_TIMEOUT(timeout)
{
    create_socket();
    src_addr = get_saddr(sourceip,srcport);
}

struct iphdr rltp::get_iphdr(int msg_len)
{
    struct iphdr ipp;
    struct iphdr *ip = &ipp;

    // IPv4 header length (4 bits): Number of 32-bit words in header = 5
    ip->ihl = IP4_HDRLEN / sizeof (uint32_t);

    // Internet Protocol version (4 bits): IPv4
    ip->version = 4;

    // Type of service (8 bits)
    ip->tos = 0;

    ip->tot_len = htons(IP4_HDRLEN + RTLPHDR_LEN + msg_len);

    ip->frag_off = 0;       /* no fragment */

    //Time to live
    ip->ttl = 64;           /* default value */

    // Transport layer protocol (8 bits)
    ip->protocol = IPPROTO_RAW; /* protocol at L4 */

    // IPv4 header checksum (16 bits)
    ip->check = 0;

    // Source IPv4 address (32 bits)
    ip->saddr = src_addr.sin_addr.s_addr;

    // Destination IPv4 address (32 bits)
    ip->daddr = dest_addr.sin_addr.s_addr;

    return ipp;

}

int rltp::rtlp_send_message(string message)
{
    if(!connected)
    {
        cerr<<"No active connection exists"<<endl;
        return -1;
    }
    int len = message.size()+1;
    src_seq_no += len;
    struct iphdr iphr = get_iphdr(len);
    struct rltp_header rltphdr;

    rltphdr.source_port = htons(src_port);
    rltphdr.dest_port = htons(destport);
    rltphdr.type = htons(DATA);
    rltphdr.seq_no = htonl(src_seq_no);
    rltphdr.ack_no = htonl(dest_seq_no);
    rltphdr.checksum = 0;

    uint32_t check_temp = calc_checksum(&rltphdr,(uint8_t *)message.c_str(),len);
    rltphdr.checksum = htonl(check_temp);

    uint8_t *packet = allocate_ustrmem(IP_MAXPACKET);

    // First part is an IPv4 header.
    memcpy (packet, &iphr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((packet + IP4_HDRLEN), &rltphdr, RTLPHDR_LEN * sizeof (uint8_t));

    // Next is the data to be sent
    memcpy ((packet + IP4_HDRLEN + RTLPHDR_LEN), message.c_str(), len * sizeof (uint8_t));

    /**Packet contructed now send and wait for an ACK*/
    fd_set readfds;
    int retry = 0,rst =0;
    while(retry < MAX_RETRY)
    {
        send_message(packet,len+IP4_HDRLEN+RTLPHDR_LEN);
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        struct timeval timeout;
        memset(&timeout,0,sizeof(timeout));
        timeout.tv_sec = RTLP_TIMEOUT /1000;
        timeout.tv_usec = (RTLP_TIMEOUT%1000) * 1000; 
        rst = select(sockfd + 1,&readfds,NULL,NULL,&timeout);
        if(rst > 0 && FD_ISSET(sockfd,&readfds))
        {
            int status = recv_ACK();
            if(status > 0)
                break;
        }
        else if(rst == -1)
        {
            return -1;
            perror("select()");
            exit(EXIT_FAILURE);
        }
        retry++;
    }
    free(packet);
    if(retry >= MAX_RETRY)
    {
        cerr<<"No ACK received after "<<MAX_RETRY<<" exiting"<<endl;
        return -1;
        exit(EXIT_FAILURE);
    }
    return 1;
}

char* rltp::rtlp_receive_message(int* status)
{
    if(!connected)
    {
        cerr<<"No active connection exists"<<endl;
        *status = -2;
        return NULL;
    }
    uint8_t* packet = allocate_ustrmem(IP_MAXPACKET);
    int bytes = recv_message(packet);
    int databytes = bytes - IP4_HDRLEN - RTLPHDR_LEN;
    cout<<"Databytes="<<databytes<<endl;

    struct iphdr* iphr = (struct iphdr*)packet;
    struct rltp_header* rlthdr = (struct rltp_header*)malloc(RTLPHDR_LEN);
    memcpy(rlthdr,packet+IP4_HDRLEN,RTLPHDR_LEN);
    //struct rltp_header* rlthdr = (struct rltp_header*)(packet + IP4_HDRLEN);
    uint8_t* data = packet+IP4_HDRLEN+RTLPHDR_LEN;
    int bytesToRead = 0;

    if(verify_checksum(packet+IP4_HDRLEN, bytes-IP4_HDRLEN))
    {
        if(ntohs(rlthdr->type) != DATA && ntohs(rlthdr->type) != FIN)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohs(rlthdr->dest_port) != src_port)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohs(rlthdr->source_port) != destport)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohl(rlthdr->ack_no) != src_seq_no)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohs(rlthdr->type) == FIN && ntohl(rlthdr->seq_no) != dest_seq_no)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohs(rlthdr->type) == DATA && ntohl(rlthdr->seq_no) - dest_seq_no != databytes)
        {
            free(packet);
            *status = -1;
            return NULL;
        }
        if(ntohs(rlthdr->type) == FIN)
        {
            accept_close();
            *status = 0;
            return NULL;
        }
        bytesToRead = ntohl(rlthdr->seq_no) - dest_seq_no;
        dest_seq_no = ntohl(rlthdr->seq_no);
        char *msg = (char *)malloc((bytesToRead+1)*sizeof(uint8_t));
        memcpy(msg,data,bytesToRead);
        msg[bytesToRead] = '\0';
        *status = bytesToRead+1;
        free(packet);
        size_t len=0;
        uint8_t *ack_packet = send_ACK(&len,ACK);
        send_message(ack_packet,len);
        free(ack_packet);
        return msg;
    }
    free(packet);
    *status = -1;
    return NULL;
}

void rltp::connect()
{
    if(connected)
    {
        cerr<<"A connection already exists"<<endl;
        return;
    }
    size_t len = 0;

    //SEND SYN
    uint8_t *syn_packet = send_SYN(&len);

    fd_set readfds;
    //WAIT FOR SYN_ACK and accept it
    
    int retry = 0,rst =0;
    while(retry < MAX_RETRY)
    {
        send_message(syn_packet,len);
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        struct timeval timeout;
        memset(&timeout,0,sizeof(timeout));
        timeout.tv_sec = RTLP_TIMEOUT /1000;
        timeout.tv_usec = (RTLP_TIMEOUT%1000) * 1000; 
        rst = select(sockfd + 1,&readfds,NULL,NULL,&timeout);
        if(rst > 0 && FD_ISSET(sockfd,&readfds))
        {
            int status = recv_SYN_ACK();
            if(status > 0)
                break;
        }
        else if(rst == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        retry++;
    }
    if(retry >= MAX_RETRY)
    {
        cerr<<"No SYN_ACK received after "<<MAX_RETRY<<" exiting"<<endl;
        exit(EXIT_FAILURE);
    }
    free(syn_packet);
    //SEND ACK
    len = 0;
    uint8_t *ack_packet = send_ACK(&len,ACK);
    send_message(ack_packet,len);
    free(ack_packet);
    connected = true;
    cout<<"Successfully Handsheked and connected"<<endl;
}

void rltp::accept_connect()
{
    if(connected)
    {
        cerr<<"A connection already exists"<<endl;
        return;
    }
    //WAIT FOR A SYN
    size_t len = 0;
    while(recv_SYN() <= 0);

    //SEND SYN_ACK and wait for ACK
    uint8_t *syn_ack_packet = send_ACK(&len,SYN_ACK);
    fd_set readfds;
    
    int retry = 0,rst =0;
    while(retry < MAX_RETRY)
    {
        send_message(syn_ack_packet,len);
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        struct timeval timeout;
        timeout.tv_sec = RTLP_TIMEOUT / 1000;
        timeout.tv_usec = (RTLP_TIMEOUT % 1000)*1000;
        rst = select(sockfd + 1,&readfds,NULL,NULL,&timeout);
        if(rst > 0)
        {
            int status = recv_ACK();
            if(status > 0)
                break;
        }
        else if(rst == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        retry++;
    }
    if(retry >= MAX_RETRY)
    {
        cerr<<"No ACK received after "<<MAX_RETRY<<" exiting"<<endl;
        exit(EXIT_FAILURE);
    }
    //cout<<"ACK recv Successfully"<<endl;
    free(syn_ack_packet);
    connected = true;
    cout<<"Successfully Handsheked and connected"<<endl;

}

int rltp::send_message(uint8_t* packet, size_t len)
{
    int rst;
    rst = sendto(sockfd,packet,len,0,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr_in));
    if(rst < 0)
    {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
    return rst;
}

int rltp::recv_message(uint8_t* packet)
{
    int rst;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    rst = recvfrom(sockfd,packet,IP_MAXPACKET,0,(struct sockaddr*)&dest_addr,&addrlen);
    if(rst < 0)
    {
        perror("Error receiving message");
        exit(EXIT_FAILURE);
    }
    return rst;
}


uint8_t* rltp::send_SYN(size_t *len)
{
    struct iphdr iphr = get_iphdr();
    struct rltp_header rltphdr;

    rltphdr.source_port = htons(src_port);
    rltphdr.dest_port = htons(destport);
    rltphdr.type = htons(SYN);
    rltphdr.seq_no = htonl(src_seq_no);
    rltphdr.ack_no = htonl(0);
    rltphdr.checksum = 0;

    uint32_t check_temp = calc_checksum(&rltphdr,NULL,0);
    rltphdr.checksum = htonl(check_temp);

    uint8_t *packet = allocate_ustrmem(IP_MAXPACKET);
    // First part is an IPv4 header.
    memcpy (packet, &iphr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((packet + IP4_HDRLEN), &rltphdr, RTLPHDR_LEN * sizeof (uint8_t));
    *len = IP4_HDRLEN + RTLPHDR_LEN;
    return packet;
}

uint8_t* rltp::send_ACK(size_t *len,uint16_t type)
{
    if(dest_seq_no <= 0)
    {
        cerr<<"No active connection has been established.. Exiting"<<endl;
        exit(EXIT_FAILURE);
    }
    struct iphdr iphr = get_iphdr();
    struct rltp_header rltphdr;

    rltphdr.source_port = htons(src_port);
    rltphdr.dest_port = htons(destport);
    rltphdr.type = htons(type);
    rltphdr.seq_no = htonl(src_seq_no);
    rltphdr.ack_no = htonl(dest_seq_no);
    rltphdr.checksum = 0;

    uint32_t check_temp = calc_checksum(&rltphdr,NULL,0);
    rltphdr.checksum = htonl(check_temp);

    uint8_t *packet = allocate_ustrmem(IP_MAXPACKET);
    // First part is an IPv4 header.
    memcpy (packet, &iphr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((packet + IP4_HDRLEN), &rltphdr, RTLPHDR_LEN * sizeof (uint8_t));
    *len = IP4_HDRLEN + RTLPHDR_LEN;
    return packet;
}

uint8_t* rltp::send_FIN(size_t *len)
{
    struct iphdr iphr = get_iphdr();
    struct rltp_header rltphdr;

    rltphdr.source_port = htons(src_port);
    rltphdr.dest_port = htons(destport);
    rltphdr.type = htons(FIN);
    rltphdr.seq_no = htonl(src_seq_no);
    rltphdr.ack_no = htonl(dest_seq_no);
    rltphdr.checksum = 0;

    uint32_t check_temp = calc_checksum(&rltphdr,NULL,0);
    rltphdr.checksum = htonl(check_temp);

    uint8_t *packet = allocate_ustrmem(IP_MAXPACKET);
    // First part is an IPv4 header.
    memcpy (packet, &iphr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((packet + IP4_HDRLEN), &rltphdr, RTLPHDR_LEN * sizeof (uint8_t));
    *len = IP4_HDRLEN + RTLPHDR_LEN;
    return packet;
}


int rltp::recv_SYN()
{
    //TODO : change litlle bit if want to connect to any client
    uint8_t* packet = allocate_ustrmem(IP_MAXPACKET);
    int bytes = recv_message(packet);
    int databytes = bytes - IP4_HDRLEN - RTLPHDR_LEN;

    struct iphdr* iphr = (struct iphdr*)packet;
    struct rltp_header* rlthdr = (struct rltp_header*)malloc(RTLPHDR_LEN);
    memcpy(rlthdr,packet+IP4_HDRLEN,RTLPHDR_LEN);
    //struct rltp_header* rlthdr = (struct rltp_header*)(packet + IP4_HDRLEN);
    uint8_t* data = packet+IP4_HDRLEN+RTLPHDR_LEN;
    int retval = -1;
    if(verify_checksum(packet+IP4_HDRLEN, bytes-IP4_HDRLEN))
    {
        if(ntohs(rlthdr->type) != SYN)
            retval = -1;
        else if(ntohs(rlthdr->dest_port) != src_port)
            retval = -1;
        else if(ntohs(rlthdr->source_port) < 0)
            retval = -1;
        else if(ntohl(rlthdr->seq_no) <= 0)
            retval = -1;
        else
        {
            destport = ntohs(rlthdr->source_port);
            dest_seq_no = ntohl(rlthdr->seq_no);
            retval = 1;
        }
    }
    free(packet);
    return retval;
}

int rltp::recv_SYN_ACK()
{
    uint8_t* packet = allocate_ustrmem(IP_MAXPACKET);
    int bytes = recv_message(packet);
    int databytes = bytes - IP4_HDRLEN - RTLPHDR_LEN;

    struct iphdr* iphr = (struct iphdr*)packet;
    struct rltp_header* rlthdr = (struct rltp_header*)(packet + IP4_HDRLEN);
    uint8_t* data = packet+IP4_HDRLEN+RTLPHDR_LEN;
	int retval = -1;
    if(verify_checksum(packet+IP4_HDRLEN, bytes-IP4_HDRLEN))
    {
        if(ntohs(rlthdr->type) != SYN_ACK)
            retval = -1;
        else if(ntohl(rlthdr->ack_no) != src_seq_no)
            retval = -1;
        else if(ntohs(rlthdr->dest_port) != src_port)
            retval = -1;
        else if(ntohs(rlthdr->source_port) != destport)
            retval = 1;
        else if(ntohl(rlthdr->seq_no) <= 0)
            retval = -1;
        else
        {
            dest_seq_no = ntohl(rlthdr->seq_no);
            retval = 1;
        }
    }
    free(packet);
    return retval;
}

int rltp::recv_ACK(uint16_t type)
{
    uint8_t* packet = allocate_ustrmem(IP_MAXPACKET);
    int bytes = recv_message(packet);
    int databytes = bytes - IP4_HDRLEN - RTLPHDR_LEN;

    struct iphdr* iphr = (struct iphdr*)packet;
    struct rltp_header* rlthdr = (struct rltp_header*)(packet + IP4_HDRLEN);
    uint8_t* data = packet+IP4_HDRLEN+RTLPHDR_LEN;
    int retval = -1;
    if(verify_checksum(packet+IP4_HDRLEN, bytes-IP4_HDRLEN))
    {
        if(ntohs(rlthdr->type) != type)
            retval = -1;
        else if(ntohl(rlthdr->ack_no) != src_seq_no)
            retval = -1;
        else if(ntohs(rlthdr->dest_port) != src_port)
            retval = -1;
        else if(ntohs(rlthdr->source_port) != destport)
            retval = -1;
        else if(ntohl(rlthdr->seq_no) != dest_seq_no)
            retval = -1;
        else
            retval = 1;
    }
    free(packet);
    return retval;
}


void rltp::set_dest(char* client_ip, short int port)
{
    if(connected)
    {
        cerr<<"A connection already exists"<<endl;
        return;
    }
    destip = client_ip;
    destport = port;
    dest_addr = get_saddr(client_ip,port);
    cout<<"Note setting destination for server side connection does not have any effect"<<endl;
}

int rltp::create_socket()
{
    int sfd;

    sfd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);

    if(sfd < 0)
    {
        perror("Error creating scoket");
        exit(EXIT_FAILURE);
    }
    cout<<"Successfully created socket : "<<sfd<<endl;

    sockfd = sfd;
    src_seq_no = rand()%200 + 1;
    setuid(getuid());
    return sfd;
}

struct sockaddr_in rltp::get_saddr(char *ipaddress,int port_num)
{
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof (struct sockaddr_in); // size of the addresses.
    memset (&src_addr, 0, addrlen);

    // Assign values to the server address.
    src_addr.sin_family = AF_INET; // IPv4.
    src_addr.sin_port   = htons (port_num); // Port Number.

    int rst = inet_pton (AF_INET, ipaddress ,(struct in_addr*) &src_addr.sin_addr);
    if (rst <= 0)
    {
        perror ("Presentation to network address conversion.\n");
        exit(EXIT_FAILURE);
    }
    return src_addr;
}

int rltp::verify_checksum(uint8_t* data, size_t datalen)
{
    struct rltp_header* rlhdr = (struct rltp_header*)data;
    uint8_t* datar = data + RTLPHDR_LEN ;
    uint32_t checksum = calc_checksum(rlhdr,datar,datalen-RTLPHDR_LEN );
    if(checksum == ntohl(rlhdr->checksum))
        return 1;
    else
        return 0;
}

uint32_t rltp::calc_checksum(struct rltp_header* rlhdr,uint8_t* data, size_t datalen)
{
    uint32_t a = 1, b = 0;
    size_t index;
    a = (a + rlhdr->source_port) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    a = (a + rlhdr->dest_port) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    a = (a + rlhdr->seq_no) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    a = (a + rlhdr->ack_no) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    a = (a + rlhdr->type) % MOD_ADLER;
    b = (b + a) % MOD_ADLER;
    for (index = 0; index < datalen; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}


// Allocate memory for an array of unsigned chars.
uint8_t * rltp::allocate_ustrmem (int len)
{
    uint8_t *tmp;

    if (len <= 0)
    {
        fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
        exit (EXIT_FAILURE);
    }

    tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
    if (tmp != NULL)
    {
        memset (tmp, 0, len * sizeof (uint8_t));
        return (tmp);
    }
    else
    {
        fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
        exit (EXIT_FAILURE);
    }
}

void rltp::close()
{
    if(!connected)
    {
        cerr<<"No open connection exists"<<endl;
        return;
    }
    size_t len = 0;

    //SEND FIN
    uint8_t *fin_packet = send_FIN(&len);

    fd_set readfds;
    //WAIT FOR FIN_ACK and accept it
   
    int retry = 0,rst =0;
    while(retry < MAX_RETRY)
    {
        send_message(fin_packet,len);
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        struct timeval timeout;
        memset(&timeout,0,sizeof(timeout));
        timeout.tv_sec = RTLP_TIMEOUT /1000;
        timeout.tv_usec = (RTLP_TIMEOUT % 1000) * 1000;
        rst = select(sockfd + 1,&readfds,NULL,NULL,&timeout);
        if(rst > 0 && FD_ISSET(sockfd,&readfds))
        {
            int status = recv_ACK(FIN_ACK);
            if(status > 0)
                break;
        }
        else if(rst == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        retry++;
    }
    if(retry >= MAX_RETRY)
    {
        cerr<<"No FIN_ACK received after "<<MAX_RETRY<<" exiting"<<endl;
        exit(EXIT_FAILURE);
    }
    free(fin_packet);
    //SEND ACK
    len = 0;
    uint8_t *ack_packet = send_ACK(&len,ACK);
    send_message(ack_packet,len);
    free(ack_packet);
    connected = false;
    cout<<"Successfully Handsheked and disconnected"<<endl;
}

void rltp::accept_close()
{
    if(!connected)
    {
        cerr<<"Not connected to any client"<<endl;
        return;
    }
    //FIN HAS BEEN RECEIVED
    size_t len = 0;

    //SEND FIN_ACK and wait for ACK
    uint8_t *fin_ack_packet = send_ACK(&len,FIN_ACK);
    fd_set readfds;
    int retry = 0,rst =0;
    while(retry < MAX_RETRY)
    {
        send_message(fin_ack_packet,len);
        struct timeval timeout;
        timeout.tv_sec = RTLP_TIMEOUT / 1000;
        timeout.tv_usec = (RTLP_TIMEOUT % 1000)*1000;
        FD_ZERO(&readfds);
        FD_SET(sockfd,&readfds);
        rst = select(sockfd + 1,&readfds,NULL,NULL,&timeout);
        if(rst > 0)
        {
            int status = recv_ACK();
            if(status > 0)
                break;
        }
        else if(rst == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }
        retry++;
    }
    if(rst < 0)
    {
        cerr<<"No ACK received after "<<MAX_RETRY<<" exiting"<<endl;
        exit(EXIT_FAILURE);
    }
    connected = false;
    cout<<"Successfully Handsheked and disconnected"<<endl;

}

