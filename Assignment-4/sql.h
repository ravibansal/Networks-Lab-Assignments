#ifndef SQL_H
#define SQL_H


#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <driver.h>
#include <exception.h>
#include <resultset.h>
#include <statement.h>
#include <prepared_statement.h>
#include <string.h>
#include <bits/stdc++.h>
using namespace std;
using namespace sql;

sql::Connection* connect(const char* serv);
int insert(char *from,char* to,char* serv);
void update(int id,char *buf,char* serv);


int getuserid(sql::Connection *conn, string username);
int deletemsg(sql::Connection *conn, vector<int>);
int markread(sql::Connection *conn, set<int>);
int verifyPassword(sql::Connection *conn, string password, string username);
vector< pair<int,double> > getMsgList(sql::Connection *conn, string username, string hostname,int unread = 0, int msgid = -1);
pair<string,string> getMessage(sql::Connection *conn, int mid);
pair<int,double> getStat(sql::Connection *conn,string username,string hostname);


#endif