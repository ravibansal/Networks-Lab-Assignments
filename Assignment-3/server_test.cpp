#include "rltp.h"
#define dest_ip "127.0.0.1"
#define dest_port 12345
int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        cerr<<"Invalid Use\nUsage:server sip sport"<<endl;
        exit(EXIT_FAILURE);
    }
	int temp;
	 if(argc>3)
		temp=atoi(argv[3]);
	else
		temp=DEF_TIMEOUT;
    rltp rltp_srv(argv[1], atoi(argv[2]),temp);
    //rltp_srv(argv[1], atoi(argv[2]),atoi(argv[3]));
    //rltp_srv.set_dest(argv[3],atoi(argv[4]));
    while(1)
    {
        rltp_srv.accept_connect();
        int status = 1;
        int temp,size;
        while(status != 0)
        { 
            char *msg = rltp_srv.rtlp_receive_message(&status);
            if(status==-1)
            {
                perror("Error in received\n");
                exit(EXIT_FAILURE);
            }
            if(status==0)
            {
                break;
            }
            cout<<"Message received:"<<msg<<endl;
            size=strlen(msg);
            //cout<<"Size: "<<size<<endl;
            temp=atoi(&msg[9])+1;
            string s="ECHO RES "+ to_string(temp+1);
            if(rltp_srv.rtlp_send_message(s)==-1)
            {
                perror("Message Not sent successfully\n");
                exit(EXIT_FAILURE);
            }
            cout<<"Message send "<<s<<endl;
            free(msg);
        }
        printf("Do you want to exit (y or n)\n");
        string c;
        cin>>c;
        if(c== "y")
        {
            break;
        }
    }
    return 0;



}
