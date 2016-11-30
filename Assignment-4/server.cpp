#include "smtp.h"
#include "POPServer.hxx"
#include <sys/types.h>
#include <sys/wait.h>

int port_numi = 23465;
int port_numi_2=25000;
int popabcport = 27001;
int popxyzport = 26002;
char ip1[30]="127.0.0.1";
char ip2[30]="127.0.0.1";
int main(int argc,char *argv[])
{
	char srv[10];
	if(argc<5)
	{
		cout<<"Usage: ./server_smtp <server> <debug> <ip_abc> <ip_xyz>"<<endl;
		exit(0);
	}
	strcpy(srv,argv[1]);
	//cout<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<endl
	if(strcmp(argv[2],"d")==0)
	{
		debug_on();
		//cout<<"debug mode on\n";
	}
	strcpy(ip1,argv[3]);
	strcpy(ip2,argv[4]);
	if(fork() == 0)
	{
		int sfd;
		if(strcmp(srv,"abc.com")==0)
			sfd=connect_server(ip1,port_numi);
		else
			sfd=connect_server(ip2,port_numi_2);
		while(1)
		{
			int cfd;
			cfd=server_accept(sfd);
			if(fork()==0)
			{
				close(sfd);
				while(initiate_server(srv,cfd)!=0);
				while(env_server(srv,cfd,ip1,ip2)!=0);
				close(cfd);
				break;
			}
			else
			{
				close(cfd);
				continue;
			}
		}
	}
	else
	{
		if(fork() == 0)
		{
			string ipaddress, domain;
			int port;
			domain = string(srv);
			if(domain == "abc.com")
			{
				ipaddress = ip1;
				port = popabcport;
			}
			else
			{
				ipaddress = ip2;
				port = popxyzport;
			}
			POPServer popser(ipaddress,port,domain);
			popser.connect();
			popser.runForever();
		}
		else
		{
			int status12;
			wait(&status12);
			wait(&status12);
		}

	}

	
	return 0;
}