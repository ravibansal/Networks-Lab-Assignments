#include "rltp.h"

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        cerr<<"Invalid Use\nUsage:server sip sport clip clport"<<endl;
        exit(EXIT_FAILURE);
    }
	int temp;
	 if(argc>5)
		temp=atoi(argv[5]);
    else
		temp=DEF_TIMEOUT;
    rltp rltp_srv(argv[1], atoi(argv[2]),temp);
    rltp_srv.set_dest(argv[3],atoi(argv[4]));
    rltp_srv.connect();
    int times = 1;
    int status;
    while(1)
    { 
    	string s="ECHO REQ "+ to_string(times);
        if(rltp_srv.rtlp_send_message(s)==-1)
        {
            perror("Message Not sent successfully\n");
            exit(EXIT_FAILURE);
        }
        cout<<"Message send "<<times<<endl;
        char *msg = rltp_srv.rtlp_receive_message(&status);
        if(status==-1)
        {
            perror("Message Not sent successfully\n");
            exit(EXIT_FAILURE);
        }
        if(status==0)
        {
            printf("Connection closed unexpectedly\n");
            exit(EXIT_FAILURE);
        }
        cout<<"Message received:"<<msg<<endl;
		times++;
        free(msg);
    }
    rltp_srv.close();
    return 0;



}
