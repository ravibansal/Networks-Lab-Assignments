all:client.o smtp.o server.o sql.o debug.o POPServer.o POPServerSession.o TCPServer.o POPClient.o TCPClient.o POPClient1.cpp
	g++ -Wall -std=c++11 -g POPClient.o TCPClient.o POPClient1.cpp -o build/popclient
	g++ -Wall -std=c++11 -I/usr/include/cppconn -o build/client_smtp client.o smtp.o sql.o debug.o POPClient.o TCPClient.o -L/usr/lib -lmysqlcppconn
	g++ -Wall -std=c++11 -I/usr/include/cppconn -o build/server_smtp server.o smtp.o sql.o debug.o POPServer.o POPServerSession.o TCPServer.o -L/usr/lib -lmysqlcppconn
client_smtp:client.o smtp.o sql.o debug.o POPClient.o TCPClient.o
	g++ -Wall -std=c++11 -I/usr/include/cppconn -o build/client_smtp client.o smtp.o sql.o debug.o POPClient.o TCPClient.o -L/usr/lib -lmysqlcppconn
server_smtp:server.o smtp.o sql.o debug.o POPServer.o POPServerSession.o TCPServer.o
	g++ -Wall -std=c++11 -I/usr/include/cppconn -o build/server_smtp server.o smtp.o sql.o debug.o POPServer.o POPServerSession.o TCPServer.o -L/usr/lib -lmysqlcppconn
client.o:client.cpp smtp.h
	g++ -c -g -std=c++11 -Wall client.cpp
server.o:server.cpp smtp.h
	g++ -c -g -std=c++11 -Wall -I/usr/include/cppconn server.cpp -L/usr/lib -lmysqlcppconn
smtp.o:smtp.cpp smtp.h sql.h debug.h
	g++ -Wall -std=c++11 -I/usr/include/cppconn -c -g smtp.cpp -L/usr/lib -lmysqlcppconn
sql.o:sql.cpp sql.h
	g++ -Wall -std=c++11 -I/usr/include/cppconn -c sql.cpp -L/usr/lib -lmysqlcppconn
sql_init:sql_init.cpp
	g++ -Wall -std=c++11 -I/usr/include/cppconn -o build/database sql_init.cpp -L/usr/lib -lmysqlcppconn
debug.o:debug.cpp debug.h
	g++ -c -g debug.cpp
clean:
	rm smtp.o server.o client.o sql.o debug.o POPServer.o POPClient.o POPServerSession.o TCPServer.o TCPClient.o build/server_smtp build/client_smtp build/popclient


POPClient.o: POPClient.cpp POPClient.hxx
	g++ -Wall -std=c++11  -c POPClient.cpp
POPServer.o: POPServer.cpp POPServer.hxx
	g++ -Wall -I/usr/include/cppconn -std=c++11  -c POPServer.cpp  -L/usr/lib -lmysqlcppconn
POPServerSession.o: POPServerSession.cpp POPServerSession.hxx
	g++ -Wall -I/usr/include/cppconn -std=c++11  -c POPServerSession.cpp  -L/usr/lib -lmysqlcppconn
TCPClient.o: TCPClient.cpp TCPClient.hxx
	g++ -Wall -std=c++11 -c TCPClient.cpp 
TCPServer.o: TCPServer.cpp TCPServer.hxx
	g++ -Wall -std=c++11  -c TCPServer.cpp 
popclient: POPClient.o TCPClient.o POPClient1.cpp
	g++ -Wall -std=c++11  POPClient.o TCPClient.o POPClient1.cpp -o build/popclient
