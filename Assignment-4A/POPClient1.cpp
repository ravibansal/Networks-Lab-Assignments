#include "POPClient.hxx"
int main(int argc, char const *argv[])
{
	cout<<"Ia in pop cleint"<<endl;
	if(argc < 5)
	{
		cout<<"Usage Error: client ipaddress domain username port";
		exit(EXIT_FAILURE);
	}
	string ip = string(argv[1]);
	string domain = string(argv[2]);
	string username = string(argv[3]);
	int port = atoi(argv[4]);
	retrieveMails(ip,port,domain,username);
	return 0;
}