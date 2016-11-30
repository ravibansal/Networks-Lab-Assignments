#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/sem.h>
#include <poll.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netdb.h> //hostent
#include <errno.h>
#include <sys/un.h>
#define SA (struct sockaddr*)
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define TIMEOUT  100000//usec
unsigned short csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL) 
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++) 
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}
void set_ip_hdr(struct ip *ip_hdr,char *buf,int hop,char *ip,char *dest)
{
    ip_hdr->ip_hl = 5;
    ip_hdr->ip_v = 4;
    ip_hdr->ip_tos = 0;
    ip_hdr->ip_len = 20 + 8;
    ip_hdr->ip_id = 10000;
    ip_hdr->ip_off = 0;
    ip_hdr->ip_ttl = hop;
    ip_hdr->ip_p = IPPROTO_ICMP;
    inet_pton (AF_INET, dest, &(ip_hdr->ip_src));
    inet_pton (AF_INET, ip, &(ip_hdr->ip_dst));
    ip_hdr->ip_sum = csum ((unsigned short *) buf, 9);
}

void set_icmp_hdr(struct icmphdr *icmphd, char*buf, int hop)
{
    icmphd->type = ICMP_ECHO;
    icmphd->code = 0;
    icmphd->checksum = 0;
    icmphd->un.echo.id = 0;
    icmphd->un.echo.sequence = hop + 1;
    icmphd->checksum = csum ((unsigned short *) (buf + 20), 4);
}

int main (int argc, char *argv[])
{
  if (argc != 3)
  {
    printf ("Usage: <dest_addr> <source_addr>\n");
    exit (0);
  }
  char *hostname = argv[1];
  char ip[100];
  hostname_to_ip(hostname , ip);
  // printf("%s resolved to %s\n" , hostname , ip);
  printf("traceroute to %s (%s), 30 hops max, 28 bytes packets\n",argv[1],ip);
  int sfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
  char buf[4096] = { 0 };
  struct ip *ip_hdr = (struct ip *) buf;
  int hop = 1;

  int one = 1;
  const int *val = &one;
  if (setsockopt (sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    printf ("Cannot set HDRINCL!\n");

  struct sockaddr_in addr;
  addr.sin_port = htons (7);
  addr.sin_family = AF_INET;
  inet_pton (AF_INET, ip, &(addr.sin_addr));


  while (hop<=30)
  {
      // ip_hdr->ip_hl = 5;
      // ip_hdr->ip_v = 4;
      // ip_hdr->ip_tos = 0;
      // ip_hdr->ip_len = 20 + 8;
      // ip_hdr->ip_id = 10000;
      // ip_hdr->ip_off = 0;
      // ip_hdr->ip_ttl = hop;
      // ip_hdr->ip_p = IPPROTO_ICMP;
      // inet_pton (AF_INET, argv[2], &(ip_hdr->ip_src));
      // inet_pton (AF_INET, ip, &(ip_hdr->ip_dst));
      // ip_hdr->ip_sum = csum ((unsigned short *) buf, 9);
      set_ip_hdr(ip_hdr,buf,hop,ip,argv[2]);
      struct icmphdr *icmphd = (struct icmphdr *) (buf + 20);
      set_icmp_hdr(icmphd,buf,hop);
      // icmphd->type = ICMP_ECHO;
      // icmphd->code = 0;
      // icmphd->checksum = 0;
      // icmphd->un.echo.id = 0;
      // icmphd->un.echo.sequence = hop + 1;
      // icmphd->checksum = csum ((unsigned short *) (buf + 20), 4);

      struct timeval sendt,recvt,rest;
      int count=3;
      int flag=0;
      while(count--)
      {
          gettimeofday(&sendt,NULL);
          // printf("%d\n",sizeof(struct ip) + sizeof(struct icmphdr));
          sendto (sfd, buf, sizeof(struct ip) + sizeof(struct icmphdr), 0, SA & addr, sizeof addr);
          char buff[4096] = { 0 };
          struct sockaddr_in addr2;
          socklen_t len = sizeof (struct sockaddr_in);
          fd_set rd_set;
          FD_ZERO(&rd_set);
          FD_SET(sfd, &rd_set);
          struct timeval timeout;
          timeout.tv_sec = 0;
          timeout.tv_usec = TIMEOUT; 
          int sel = select(sfd + 1, &rd_set, NULL, NULL, &timeout);
          if (sel == -1)
          {
            perror("Server: Error in select");
            exit(1);
          }
          if(FD_ISSET(sfd, &rd_set))
          {
              recvfrom (sfd, buff, sizeof(buff), 0, SA & addr2, &len);
              gettimeofday(&recvt,NULL);
              timersub(&recvt,&sendt,&rest);
              double elapsed=(rest.tv_sec)*1000+((double)(rest.tv_usec))/1000;
              struct icmphdr *icmphd2 = (struct icmphdr *) (buff + 20);

              if(count==2)
              {
                int name=0;
                char hostname[101];
                char servername[101];

                if(getnameinfo((struct sockaddr*)&addr2, sizeof addr2, hostname, sizeof hostname, servername, sizeof servername, NI_NAMEREQD)==0)
                {
                    name=1;
                }
                if (icmphd2->type != 0 && strcmp(inet_ntoa(addr2.sin_addr),ip)!=0)
                {
                  if(hop<=9)
                  {
                    if(name==1)
                    {
                    printf (" %d  %s (%s) %lf ms  ", hop, hostname,inet_ntoa (addr2.sin_addr),elapsed);
                    }
                    else{
                    printf (" %d  %s (%s) %lf ms  ", hop, inet_ntoa (addr2.sin_addr),inet_ntoa (addr2.sin_addr),elapsed);
                    }
                  }
                  else
                  {
                    if(name==1)
                    {
                      printf ("%d  %s (%s) %lf ms  ", hop, hostname,inet_ntoa (addr2.sin_addr),elapsed);
                    }
                    else{
                      printf ("%d  %s (%s) %lf ms  ", hop, inet_ntoa (addr2.sin_addr),inet_ntoa (addr2.sin_addr),elapsed);
                    }
                  }
                }
                else
                {
                   if(hop<=9)
                   {
                    if(name==1)
                    {
                      printf (" %d  %s (%s) %lf ms  ", hop, hostname,inet_ntoa (addr2.sin_addr),elapsed);

                    }
                    else{
                      printf (" %d  %s (%s) %lf ms  ", hop, inet_ntoa (addr2.sin_addr),inet_ntoa (addr2.sin_addr),elapsed);
                    }
                  }
                   else{
                      if(name==1)
                      {
                        printf ("%d  %s (%s) %lf ms  ",hop,hostname,inet_ntoa (addr2.sin_addr),elapsed);
                      }
                      else{
                        printf ("%d  %s (%s) %lf ms  ",hop,inet_ntoa (addr2.sin_addr),inet_ntoa (addr2.sin_addr),elapsed);
                      }
                   }
                    flag=1;
                }
              }
              else
              {
                  printf("%lf ms  ",elapsed);
              }
          }
          else
          {
              if(count==2)
              {
                  if(hop<=9)
                    printf(" %d  * ",hop);
                  else
                    printf("%d  * ",hop);
              }
              else
              {
                  printf("* ");
              }
          }
      }
      printf("\n");
      if(flag==1)
      {
        exit(0);
      }
      hop++;
  }

  return 0;
}