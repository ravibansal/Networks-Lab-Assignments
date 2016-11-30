#include<bits/stdc++.h>
using namespace std;
//1-lb 2-mb 3-ub 4-lb 5-mb 6-ub 7-sl 8-su
int t1[3][13][73]; //0-12321 AC, 1-12321 Sleeper , 2-12301 AC
int person[10001],coach[10001];

#define SU 1
#define SL 2
#define UB 3
#define LB 4
#define MB 5
#define NA 6

#define TYPES 7
class Request
{
private:
    const static int senior_limit = 60; //inclusive
public:
    string inp, birth_priority, cls, ages;
    long long t_stp;
    int cfd, scz, nop, train, p_id, tstp;
    std::vector< std::pair<int, int> > vt;
    std::vector<int> count;
    Request(int cfd, string inp): inp(inp), cfd(cfd)
    {
        scz = 0;
        nop = 0;
        std::istringstream ss(inp);
        std::string token;
        if (std::getline(ss, token, ','))
        {
            sscanf(token.c_str(), "%d", &p_id);
            //printf("p_id: %d\n", p_id);
        }
        if (std::getline(ss, token, ','))
        {
            sscanf(token.c_str(), "%d", &train);
            ///printf("train: %d\n", train);
        }
        if (std::getline(ss, token, ','))
        {
            cls = token;
            //cout << "class: " << cls << endl;
        }

        if (std::getline(ss, token, ','))
        {
            sscanf(token.c_str(), "%d", &nop);
            //printf("number of passengers: %d\n", nop);
        }
        if (std::getline(ss, token, ','))
        {
            birth_priority = token;
            //cout<<"Birth Prioirty : "<<birth_priority<<endl;
        }
        if (std::getline(ss, token, ','))
        {
            ages = token;
            //cout<<"Ages : "<<ages<<endl;
        }
        if (std::getline(ss, token, ','))
        {
            t_stp = stoll(token);
            //cout<<"Ages : "<<ages<<endl;
        }
        //printf("%lld\n",t_stp);
        std::istringstream ss1(ages);
        int i = 0;
        std::vector< std::pair<int, int> >::iterator it;
        it = vt.begin();
        int ghu=0;
        while (std::getline(ss1, token, '-'))
        {
            ghu=1;
            for (i = 0; i < token.length(); i++)
            {
                if (!('0' <= token[i] && token[i] <= '9'))
                    token.erase(i--, 1);
            }
            int temp;
            temp = atoi(token.c_str());
            //cout<<token.c_str()<<"::"<<temp<<endl;
            if (temp >= senior_limit)
            {
                scz++;
            }
            ///cout << "1234Age:" << token << "  " << temp << endl;
            //vt[i]=make_pair(par,temp);
            vt.push_back(make_pair(6, temp));
        }
        if(ghu==0)
        {
            int temp;
            //printf("HOLAADSADSADSASAdas\n");
            temp=atoi(ages.c_str());
            vt.push_back(make_pair(6, temp));
        }
        std::istringstream ss2(birth_priority);
        i = 0;
        count = vector<int>(TYPES, 0);
        while (std::getline(ss2, token, '-'))
        {
            ghu=1;
            for (int j = 0; j < token.length(); j++)
            {
                if (!('A' <= token[j] && token[j] <= 'Z'))
                    token.erase(j--, 1);
            }
            //cout <<"here" << token << endl;
            if (token == "")
                continue;
            if (token == "SU")
            {
                count[SU] += 1;
                vt[i].first = SU;
            }
            else if (token == "SL")
            {
                count[SL] += 1;
                vt[i].first = SL;
            }
            else if (token == "UB")
            {
                count[UB] += 1;
                vt[i].first = UB;
            }
            else if (token == "LB")
            {
                count[LB] += 1;
                vt[i].first = LB;
            }
            else if (token == "MB")
            {
                count[MB] += 1;
                vt[i].first = MB;
            }
            else
            {
                //cout<<"hello\n";
                count[NA] += 1;
                vt[i].first = NA;
            }
            //cout<<"Pref = "<<vt[i].first<<endl;
            i++;
        }
        if(ghu==0)
        {
            string token=birth_priority;
            i=0;
            if (token == "SU")
            {
                count[SU] += 1;
                vt[i].first = SU;
            }
            else if (token == "SL")
            {
                count[SL] += 1;
                vt[i].first = SL;
            }
            else if (token == "UB")
            {
                count[UB] += 1;
                vt[i].first = UB;
            }
            else if (token == "LB")
            {
                count[LB] += 1;
                vt[i].first = LB;
            }
            else if (token == "MB")
            {
                count[MB] += 1;
                vt[i].first = MB;
            }
            else
            {
                //cout<<"hello\n";
                count[NA] += 1;
                vt[i].first = NA;
            }
        }
        //printf("\n");
    }
    friend bool operator <(const Request& lhs, const Request&rhs)
    {
        if ((rhs.t_stp - lhs.t_stp) > 0)
            return true;
        else if ((rhs.t_stp - lhs.t_stp) == 0)
        {
            if (rhs.nop > lhs.nop)
            {
                return true;
            }
            else if (rhs.nop == lhs.nop)
            {
                if (rhs.scz > lhs.scz)
                {
                    return true;
                }
            }
        }
        return false;
    }
};

int maxind(int flag[],int j)
{
    int maxv=0,maxi;
    for(int i=0; i<j; i++)
    {
        if(flag[i]>maxv)
        {
            maxv=flag[i];
            maxi=i;
        }
    }
    return maxi;
}

struct sort_pred
{
    bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right)
    {
        return left.second < right.second;
    }
};
int allocator_berth(Request r1)
{
    //According to preferences
    int rem1=1,rem2=1;
    int mod=8,done=0;
    /*string s1="1,12321,AC,20,SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL,17-64-68-65-17-64-68-65-17-64-17-64-68-65-17-64-68-65-17-64";
    string s2="2,12321,Sleeper,4,SU-UB,24-28-60-65";
    time_t now;
    Request r1(1,s1,now),r2(2,s2,now);*/
    memset(person,0,sizeof(person));
    memset(coach,0,sizeof(person));
    for(int i=0;i<r1.nop;i++)
    {
        person[i]=0;
        coach[i]=0;
    }
    int q1,q2,q3;
    if(r1.train==12321)
    {
        if(r1.cls=="AC")
        {
            q1=0;
            q2=3;
            q3=72;
        }
        else{
            q1=1;
            q2=12;
            q3=72;
        }
    }
    else{
        q1=2;
        q2=13;
        q3=54;
    }
    int flag[q2];
    for(int i=0;i<q2;i++)
        flag[i]=0;
    std::vector < std::pair<int,int> > flg;
    for(int j=0; j<q2; j++)
    {
        for(int i=0; i<r1.nop; i++)
        {
            mod=8;
            if(q1==2)
                mod=6;
            if(r1.vt[i].first==1)//SU
            {
                rem1=0;
                rem2=0;
            }
            else if(r1.vt[i].first==2)//SL
            {
                rem1=7;
                rem2=7;
                if(q1==2)
                {
                    rem1=5;
                    rem2=5;
                }
            }
            else if(r1.vt[i].first==3)//UB
            {
                rem1=3;
                rem2=6;
                if(q1==2)
                {
                    rem1=2;
                    rem2=4;
                }
            }

            else if(r1.vt[i].first==4)//LB
            {
                rem1=1;
                rem2=4;
                if(q1==2)
                {
                    rem1=1;
                    rem2=3;
                }
            }
            else if(r1.vt[i].first==5)//MB
            {
                rem1=2;
                rem2=5;
            }
            else
            {
                mod=1;
                rem1=0;
                rem2=0;
            }
            for(int k=1; k<=q3; k++)
            {
                if(k%mod==rem1||k%mod==rem2)
                {
                    if(t1[q1][j][k]==0)
                    {
                        flag[j]++;
                        t1[q1][j][k]=2;
                        person[i]=k;
                        coach[i]=j;
                        break;
                    }
                }
            }
        }
        flg.push_back(std::make_pair(j,flag[j]));
        if(flag[j]==r1.nop)
        {
            done=1;
            break;
        }
        else{
            for(int k=1; k<=q3; k++)
            {
                if(t1[q1][j][k]==2)
                    t1[q1][j][k]=0;
            }
        }
    }
    if(done==0)
    {
        //According to one berth
        //int j=maxind(flag,3);
        memset(person,0,r1.nop);
        memset(coach,0,r1.nop);
        for(int i=0;i<r1.nop;i++)
        {
            person[i]=0;
            coach[i]=0;
        }
        int dony=0;
        for(int i=0; i<q2; i++)
        {
            for(int j=1; j<=q3; j++)
            {
                if(t1[q1][i][j]==2)
                    t1[q1][i][j]=0;
            }
        }
        memset(flag,0,sizeof(flag));
        sort(flg.begin(),flg.end(),sort_pred());
        std::vector < std::pair<int, int> >::iterator it;
        for(it=flg.begin(); it!=flg.end(); it++)
        {
            mod=8;
            if(q1==2)
                mod=6;
            for(int i=0; i<r1.nop; i++)
            {
                if(r1.vt[i].first==1)
                {
                    rem1=0;
                    rem2=0;
                }
                 else if(r1.vt[i].first==2)//SL
                {
                    rem1=7;
                    rem2=7;
                    if(q1==2)
                    {
                        rem1=5;
                        rem2=5;
                    }
                }
                else if(r1.vt[i].first==3)//UB
                {
                    rem1=3;
                    rem2=6;
                    if(q1==2)
                    {
                        rem1=2;
                        rem2=4;
                    }
                }

                else if(r1.vt[i].first==4)//LB
                {
                    rem1=1;
                    rem2=4;
                    if(q1==2)
                    {
                        rem1=1;
                        rem2=3;
                    }
                }
                else if(r1.vt[i].first==5)
                {
                    rem1=2;
                    rem2=5;
                }
                else
                {
                    mod=1;
                    rem1=0;
                    rem2=0;
                }
                for(int k=1; k<=q3; k++)
                {
                    if(k%mod==rem1||k%mod==rem2)
                    {
                        if(t1[q1][(*it).first][k]==0)
                        {
                            flag[(*it).first]++;
                            t1[q1][(*it).first][k]=2;
                            person[i]=k;
                            coach[i]=(*it).first;
                            break;
                        }
                    }
                }
            }
            for(int i=0; i<r1.nop; i++)
            {
                if(person[i]==0)
                {
                    for(int k=1; k<=q3; k++)
                    {
                        if(t1[q1][(*it).first][k]==0)
                        {
                            flag[(*it).first]++;
                            t1[q1][(*it).first][k]=2;
                            person[i]=k;
                            coach[i]=(*it).first;
                            break;
                        }
                    }

                }
            }
            if(flag[(*it).first]==r1.nop)
            {
                dony=1;
                break;
            }
            else{
            for(int k=1; k<=q3; k++)
            {
                if(t1[q1][(*it).first][k]==2)
                    t1[q1][(*it).first][k]=0;
            }
        }
        }
        if(dony==0)//to give them different berths
        {
            //printf("HDASDSADASDAS\n");
            int cnp=0;
            memset(person,0,r1.nop);
            memset(coach,0,r1.nop);
            for(int i=0;i<r1.nop;i++)
            {
                person[i]=0;
                coach[i]=0;
            }
            for(int i=0; i<q2; i++)
            {
                for(int j=1; j<=q3; j++)
                {
                    if(t1[q1][i][j]==2)
                        t1[q1][i][j]=0;
                }
            }
            for(int i=0; i<r1.nop; i++)
            {
                mod=8;
                if(q1==2)
                    mod=6;
                if(r1.vt[i].first==1)
                {
                    rem1=0;
                    rem2=0;
                }
                else if(r1.vt[i].first==2)//SL
                {
                    rem1=7;
                    rem2=7;
                    if(q1==2)
                    {
                        rem1=5;
                        rem2=5;
                    }
                }
                else if(r1.vt[i].first==3)//UB
                {
                    rem1=3;
                    rem2=6;
                    if(q1==2)
                    {
                        rem1=2;
                        rem2=4;
                    }
                }

                else if(r1.vt[i].first==4)//LB
                {
                    rem1=1;
                    rem2=4;
                    if(q1==2)
                    {
                        rem1=1;
                        rem2=3;
                    }
                }
                else if(r1.vt[i].first==5)
                {
                    rem1=2;
                    rem2=5;
                }
                else
                {
                    mod=1;
                    rem1=0;
                    rem2=0;
                }
                for(int j=0; j<q2; j++)
                {
                    for(int k=1; k<=q3; k++)
                    {
                        if(k%mod==rem1||k%mod==rem2)
                        {
                            if(t1[q1][j][k]==0)
                            {
                                cnp++;
                                t1[q1][j][k]=2;
                                //printf("dapo %d %d\n",j,k);
                                person[i]=k;
                                coach[i]=j;
                                break;
                            }
                        }
                    }
                    if(person[i]!=0)
                        break;
                }
            }
            int noberth=0;
            for(int i=0; i<r1.nop; i++)
            {
                //printf("Person : %d\n",person[i]);
                if(person[i]==0)
                {
                    //cout<<"Hello\n";
                    for(int j=0; j<q2; j++)
                    {
                        for(int k=1; k<=q3; k++)
                        {
                            if(t1[q1][j][k]==0)
                            {
                                t1[q1][j][k]=2;
                                //printf("da %d %d\n",j,k);
                                person[i]=k;
                                coach[i]=j;
                                break;
                            }
                        }
                        if(person[i]!=0)
                            break;
                    }
                    if(person[i]==0)
                    {
                        noberth=1;
                        break;
                    }
                }
            }
            if(noberth==1)
            {
                for(int i=0; i<q2; i++)
                {
                    for(int j=1; j<=q3; j++)
                    {
                        if(t1[q1][i][j]==2)
                            t1[q1][i][j]=0;
                    }
                }
                return -1;
            }
            else
            {
                for(int i=0; i<q2; i++)
                {
                    for(int j=1; j<=q3; j++)
                    {
                        if(t1[q1][i][j]==2)
                        {
                            //printf("%d %d\n",i,j);
                            t1[q1][i][j]=1;
                        }
                    }
                }
                //printf("Hello\n");
                return 1;
            }
        }
        else
        {
            //printf("Hello\n");
            for(int i=0; i<q2; i++)
            {
                for(int j=1; j<=q3; j++)
                {
                    if(t1[q1][i][j]==2)
                        t1[q1][i][j]=1;
                }
            }
            /*for(int i=0; i<r1.nop; i++)
            {
                printf("Coach: A%d  Seat: %d\n",coach[i]+1,person[i]);
            }*/
            return 1;
        }
    }
    else
    {
        for(int i=0; i<q2; i++)
        {
            for(int j=1; j<=q3; j++)
            {
                if(t1[q1][i][j]==2)
                    t1[q1][i][j]=1;
            }
        }
        /*for(int i=0; i<r1.nop; i++)
        {
            printf("Coach: A%d  Seat: %d\n",coach[i]+1,person[i]);
        }*/
        return 1;
    }
}
void avl()
{
    int a=0,b=0,c=0;
    for(int i=0;i<3;i++)
    {
        for(int j=1;j<=72;j++)
        {
            if(t1[0][i][j]==1)
            {
                a++;
            }
        }
    }
    for(int i=0;i<12;i++)
    {
        for(int j=1;j<=72;j++)
        {
            if(t1[1][i][j]==1)
            {
                b++;
            }
        }
    }
    for(int i=0;i<13;i++)
    {
        for(int j=1;j<=54;j++)
        {
            if(t1[2][i][j]==1)
            {
                c++;
            }
        }
    }
    cout<<"Train        "<<"\t\t"<<"#seats Total[Booked/Available](AC)"<<"\t\t"<<"#seats Total[Booked/Available](Sleeper)"<<endl;
    cout<<"Superfast Exp"<<"\t\t"<<"    216["<<a<<"/"<<(216-a)<<"]"<<"      "<<"\t\t"<<"      864["<<b<<"/"<<(864-b)<<"]"<<endl;
    cout<<"Rajdhani Exp "<<"\t\t"<<"    702["<<c<<"/"<<(216-c)<<"]"<<"      "<<"\t\t"<<"      -"<<endl;
}
/*int main()
{
    memset(t1,0,sizeof(t1));
    string s3="1,12321,AC,1,SL,45,12345";
    string s1="1,12321,AC,20,SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL-SL,17-64-68-65-17-64-68-65-17-64-17-64-68-65-17-64-68-65-17-64,12345";
    string s2="2,12321,AC,4,SL-SL,24-28-60-65,12345";
    Request r1(1,s1),r2(2,s2),r3(3,s3);
    memset(person,0,sizeof(person));
    memset(coach,0,sizeof(person));
    if(allocator_berth(r3)==1)
    {
        for(int i=0; i<r3.nop; i++)
        {
            printf("Coach: A%d  Seat: %d\n",coach[i]+1,person[i]);
        }
    }
    else{
        printf("No berths Available\n");
    }
    //printf("%d\n",r3.nop);
}*/
