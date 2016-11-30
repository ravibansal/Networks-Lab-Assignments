#include "sql.h"
sql::Connection* connect(const char* serv)
{
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	// sql::ResultSet  *res;
	// sql::PreparedStatement  *prep_stmt;
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
	stmt = con->createStatement();
	if (strcmp(serv, "abc.com") == 0)
	{
		try {
			stmt->execute("USE storage_abc");
		}
		catch (sql::SQLException &e) {
			cout << "Please run sql_init_first";
			exit(1);
		}
	}
	else {
		try {
			stmt->execute("USE storage_xyz");
		}
		catch (sql::SQLException &e) {
			cout << "Please run sql_init_first";
			exit(1);
		}
	}
	delete stmt;
	return con;
}

int insert(char *from, char* to, char* serv)
{
	sql::Connection *con;
	con = connect(serv);
	sql::Statement *stmt;
	sql::ResultSet  *res;
	sql::PreparedStatement  *prep_stmt;
	prep_stmt = con->prepareStatement("INSERT INTO data(sender,receiver,message) VALUES(?,?,?)");
	prep_stmt->setString(1, from);
	prep_stmt->setString(2, to);
	prep_stmt->setString(3, "");
	prep_stmt->execute();
	stmt = con->createStatement();
	res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
	int id;
	while (res->next())
	{
		id = res->getInt(1);
	}
	delete stmt;
	delete con;
	return id;
}

void update(int id, char *buf, char* serv)
{
	sql::Connection *con;
	con = connect(serv);
	// sql::Statement *stmt;
	sql::PreparedStatement *prep_stmt;
	//sql::ResultSet  *res;
	//stringstream stmtvar;
	//stmtvar << "UPDATE data set message=CONCAT(message," << "'" << buf << "'" << ") where id=" << id;
	//cout<<stmtvar.str()<<endl;
	prep_stmt = con->prepareStatement("UPDATE data set message=CONCAT(message,?) where id=?");
	prep_stmt->setString(1,buf);
	prep_stmt->setInt(2,id);
	prep_stmt->execute();
	//stmt = con->createStatement();
	//stmt->executeUpdate(stmtvar.str());
	delete prep_stmt;
	delete con;
}


int getuserid(sql::Connection* con, string username)
{
	sql::PreparedStatement  *prep_stmt;
	sql::ResultSet  *res;
	try 
	{
		int uid = 0;
		cout<<username<<endl;
		prep_stmt = con->prepareStatement("SELECT uid from users WHERE username=LOWER(?)");
		prep_stmt->setString(1, username);
		res = prep_stmt->executeQuery();
		while (res->next())
		{
			uid = res->getInt("uid");
		}
		cout<<uid<<endl;
		return uid;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	delete res;
	return 0;
}
int deletemsg(sql::Connection *con, vector<int> list)
{
	string liststr = "";
	std::vector<int>::iterator i;
	std::vector<int>::iterator end_list = list.end();
	advance(end_list,-1);
	for (i = list.begin(); i != end_list; ++i)
	{
		liststr += to_string(*i) + ",";
	}
	liststr += to_string(*i);

	sql::PreparedStatement  *prep_stmt;
	try 
	{
		prep_stmt = con->prepareStatement("DELETE FROM data where id in ("+liststr+")");
		prep_stmt->execute();
		return 1;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	return 0;
}
int markread(sql::Connection *con, set<int> list)
{
	string liststr = "";
	std::set<int>::iterator i;
	std::set<int>::iterator end_list = list.end();
	advance(end_list,-1);
	for (i = list.begin(); i != end_list; ++i)
	{
		liststr += to_string(*i) + ",";
	}
	liststr += to_string(*i);

	sql::PreparedStatement  *prep_stmt;
	try 
	{
		prep_stmt = con->prepareStatement("UPDATE data SET isread=1 where id in ("+liststr+")");
		prep_stmt->executeUpdate();
		return 1;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}

	delete prep_stmt;
	return 0;
}

int verifyPassword(sql::Connection* con, string password, string username)
{
	sql::PreparedStatement  *prep_stmt;
	sql::ResultSet  *res;
	try 
	{
		int uid = 0;
		prep_stmt = con->prepareStatement("SELECT uid from users WHERE password=md5(?) and username= ?");
		prep_stmt->setString(1, password);
		prep_stmt->setString(2, username);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			uid = res->getInt("uid");
		}
		return uid;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	delete res;
	return 0;
}

vector< pair<int,double> > getMsgList(sql::Connection* con, string username, string hostname,int unread,int msgid)
{
	sql::PreparedStatement  *prep_stmt;
	sql::ResultSet  *res;
	vector< pair<int,double> > msg;
	string extra  = "";
	if(unread == 0)
	{
		extra = " and isread = 0";
	}
	try 
	{
		//cout<<"Here:"<<username<<hostname<<msgid<<endl;
		if(msgid == -1)
		{
			string query = "SELECT id,length(message)/1000 as size from data WHERE receiver=?"+extra+" ORDER BY id DESC";
			cout<<query<<endl;
			prep_stmt = con->prepareStatement(query);
			prep_stmt->setString(1, username+"@"+hostname);
		}
		else
		{
			prep_stmt = con->prepareStatement("SELECT id,length(message)/1000 as size from data WHERE receiver=? and id=?");
			prep_stmt->setString(1, username+"@"+hostname);
			prep_stmt->setInt(2,msgid);
		}
		res = prep_stmt->executeQuery();
		int  num = 0;
		double size = 0;
		while (res->next())
		{
			int id = res->getInt("id");
			double len = res->getDouble("size");
			size += len;
			num++;
			msg.push_back(make_pair(id,len));
		}
		msg.push_back(make_pair(num,size));
		cout<<"Returnming<< num = "<<num << " " <<size<<endl;
		return msg;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	delete res;
	return msg;

}

pair<string,string> getMessage(sql::Connection* con, int mid)
{
	sql::PreparedStatement  *prep_stmt;
	sql::ResultSet  *res;
	pair<string,string> msg;
	try 
	{
		prep_stmt = con->prepareStatement("SELECT sender,message from data WHERE id=?");
		prep_stmt->setInt(1, mid);
		res = prep_stmt->executeQuery();
		if (res->next())
		{
			string sender = res->getString("sender");
			string message = res->getString("message");
			msg = make_pair(sender,message);
		}
		return msg;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	delete res;
	return msg;

}

pair<int,double> getStat(sql::Connection *con,string username,string hostname)
{
	sql::PreparedStatement  *prep_stmt;
	sql::ResultSet  *res;
	pair<int,double> msg; 
	try 
	{
		prep_stmt = con->prepareStatement("SELECT count(id) as cnt,sum(length(message)/1000) as total from data WHERE receiver=?");
		prep_stmt->setString(1, username+"@"+hostname);
		res = prep_stmt->executeQuery();
		msg = make_pair(0,0);
		while (res->next())
		{
			int cnt = res->getInt("cnt");
			double total = res->getDouble("total");
			msg = make_pair(cnt, total);
		}
		return msg;
	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " 
		     << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << 
		     " )" << endl;
	}
	delete prep_stmt;
	delete res;
	return msg;

}

// int main()
// {

// 	// stmt->execute("INSERT INTO test(id, label) VALUES (1, 'a')");
// 	// res = stmt->executeQuery("SELECT id, label FROM test ORDER BY id ASC");
// 	// while (res->next()) {
// 	//   // You can use either numeric offsets...
// 	//   cout << "id = " << res->getInt(1); // getInt(1) returns the first column
// 	//   // ... or column names for accessing results.
// 	//   // The latter is recommended.
// 	//   cout << ", label = '" << res->getString("label") << "'" << endl;
// 	// }
// 	// prep_stmt = con->prepareStatement("INSERT INTO test(id, label) VALUES (?, ?)");

// 	// prep_stmt->setInt(1, 1);
// 	// prep_stmt->setString(2, "a");
// 	// prep_stmt->execute();

// 	// prep_stmt->setInt(1, 2);
// 	// prep_stmt->setString(2, "b");
// 	// prep_stmt->execute();

// 	// res = stmt->executeQuery("SELECT id, label FROM test ORDER BY id ASC");
// 	// while (res->next()) {
// 	//   // You can use either numeric offsets...
// 	//   cout << "id = " << res->getInt(1); // getInt(1) returns the first column
// 	//   // ... or column names for accessing results.
// 	//   // The latter is recommended.
// 	//   cout << ", label = '" << res->getString("label") << "'" << endl;
// 	// }
// 	// char from[64]="Alice@abc.com";
// 	// char to[64]="Bob@xyz.com";
// 	// cout<<insert(from,to,2)<<endl;
// 	update(2,"PAN LOOP FAST FOOD",1);
//   	return 0;
// }
