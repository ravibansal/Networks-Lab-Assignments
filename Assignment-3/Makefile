all: server_test.o rltp.o client_test.o
	g++ server_test.o rltp.o -o server
	g++ client_test.o rltp.o -o client
server: server_test.o rltp.o
	g++ server_test.o rltp.o -o server
client: client_test.o rltp.o
	g++ client_test.o rltp.o -o client
client_test.o: client_test.cpp
	g++ -c -std=c++11 client_test.cpp
server_test.o: server_test.cpp
	g++ -c -std=c++11 server_test.cpp
rltp.o: rltp.cpp rltp.h
	g++ -c -std=c++11 rltp.cpp
clean:
	rm server client client_test.o server_test.o rltp.o

