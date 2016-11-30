#include <stdlib.h>
#include <iostream>
#include <mysql_connection.h>
#include <driver.h>
#include <exception.h>
#include <resultset.h>
#include <statement.h>
#include <prepared_statement.h>
using namespace std;
using namespace sql;

sql::Connection* connect()
{
	sql::Driver *driver;
  	sql::Connection *con;
	driver = get_driver_instance();
	con = driver->connect("tcp://127.0.0.1:3306", "root", "1234");
	return con;
}

int main(void)
{
  	sql::Connection *con;
	sql::Statement *stmt;
	// sql::ResultSet  *res;
	// sql::PreparedStatement  *prep_stmt;
	con=connect();
	stmt = con->createStatement();
	try{
		stmt->execute("USE storage_abc");
	}
	catch(sql::SQLException &e){
		// default:
		stmt->execute("CREATE DATABASE storage_abc");
		stmt->execute("USE storage_abc");
	}
	stmt->execute("DROP TABLE IF EXISTS data");
	stmt->execute("CREATE TABLE data(id INT NOT NULL AUTO_INCREMENT,sender VARCHAR(64) NOT NULL,  receiver VARCHAR(64) NOT NULL, message LONGTEXT, isread tinyint(1) not null default 0,PRIMARY KEY(id)) DEFAULT CHARSET=utf8");
	stmt->execute("DROP TABLE IF EXISTS users");
	stmt->execute("CREATE TABLE users(uid INT NOT NULL AUTO_INCREMENT, username VARCHAR(64) NOT NULL, password VARCHAR(128) NOT NULL, PRIMARY KEY(uid)) DEFAULT CHARSET=utf8");
	string query = "INSERT INTO users values (1,'alice',md5('alice'));";
	stmt->execute(query);
	query = "INSERT INTO users values (2,'arun',md5('arun'));";
	stmt->execute(query);
	query = "INSERT INTO users values (3,'ananya',md5('ananya'));";
	stmt->execute(query);
	query = "INSERT INTO users values (4,'alex',md5('alex'));";
	stmt->execute(query);
	try{
		stmt->execute("USE storage_xyz");
	}
	catch(sql::SQLException &e){
		// default:
		stmt->execute("CREATE DATABASE storage_xyz");
		stmt->execute("USE storage_xyz");
	}
	stmt->execute("DROP TABLE IF EXISTS data");
	stmt->execute("CREATE TABLE data(id INT NOT NULL AUTO_INCREMENT,sender VARCHAR(64) NOT NULL,  receiver VARCHAR(64) NOT NULL, message LONGTEXT, isread tinyint(1) not null default 0,PRIMARY KEY(id)) DEFAULT CHARSET=utf8");
	stmt->execute("DROP TABLE IF EXISTS users");
	stmt->execute("CREATE TABLE users(uid INT NOT NULL AUTO_INCREMENT, username VARCHAR(64) NOT NULL, password VARCHAR(128) NOT NULL, PRIMARY KEY(uid)) DEFAULT CHARSET=utf8");
	query = "INSERT INTO users values (1,'bob',md5('bob'));";
	stmt->execute(query);
	query = "INSERT INTO users values (2,'bilal',md5('bilal'));";
	stmt->execute(query);
	query = "INSERT INTO users values (3,'bernee',md5('bernee'));";
	stmt->execute(query);
	query = "INSERT INTO users values (4,'alex',md5('alex'));";
	stmt->execute(query);
	// stmt->execute("INSERT INTO test(id, label) VALUES (1, 'a')");
	// res = stmt->executeQuery("SELECT id, label FROM test ORDER BY id ASC");
	// while (res->next()) {
	//   // You can use either numeric offsets...
	//   cout << "id = " << res->getInt(1); // getInt(1) returns the first column
	//   // ... or column names for accessing results.
	//   // The latter is recommended.
	//   cout << ", label = '" << res->getString("label") << "'" << endl;
	// }
	// prep_stmt = con->prepareStatement("INSERT INTO test(id, label) VALUES (?, ?)");

	// prep_stmt->setInt(1, 1);
	// prep_stmt->setString(2, "a");
	// prep_stmt->execute();

	// prep_stmt->setInt(1, 2);
	// prep_stmt->setString(2, "b");
	// prep_stmt->execute();

	// res = stmt->executeQuery("SELECT id, label FROM test ORDER BY id ASC");
	// while (res->next()) {
	//   // You can use either numeric offsets...
	//   cout << "id = " << res->getInt(1); // getInt(1) returns the first column
	//   // ... or column names for accessing results.
	//   // The latter is recommended.
	//   cout << ", label = '" << res->getString("label") << "'" << endl;
	// }
	delete stmt;
	delete con;

  	return 0;
}
