all:client.o smtp.o debug.o POPClient.o TCPClient.o POPClient1.cpp
	g++ -Wall -std=c++11 -g POPClient.o TCPClient.o POPClient1.cpp -o build/popclient
	g++ -Wall -std=c++11  -o build/client_smtp client.o smtp.o debug.o POPClient.o TCPClient.o 
client_smtp:client.o smtp.o sql.o debug.o POPClient.o TCPClient.o
	g++ -Wall -std=c++11  -o build/client_smtp client.o smtp.o sql.o debug.o POPClient.o TCPClient.o 
client.o:client.cpp smtp.h
	g++ -c -g -std=c++11 -Wall client.cpp
server.o:server.cpp smtp.h
	g++ -c -g -std=c++11 -Wall  server.cpp
smtp.o:smtp.cpp smtp.h debug.h
	g++ -Wall -std=c++11  -c -g smtp.cpp 
debug.o:debug.cpp debug.h
	g++ -c -g debug.cpp
clean:
	rm smtp.o  client.o  debug.o POPClient.o TCPClient.o  build/client_smtp build/popclient


POPClient.o: POPClient.cpp POPClient.hxx
	g++ -Wall -std=c++11  -c POPClient.cpp
TCPClient.o: TCPClient.cpp TCPClient.hxx
	g++ -Wall -std=c++11 -c TCPClient.cpp 
popclient: POPClient.o TCPClient.o POPClient1.cpp
	g++ -Wall -std=c++11  POPClient.o TCPClient.o POPClient1.cpp -o build/popclient
