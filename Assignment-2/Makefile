server: server.o allocate.o server.cpp allocate.cpp
	g++ -pthread server.o -o server
server.o: server.cpp
	g++ -std=c++11 -pthread -c server.cpp
allocate.o: allocate.cpp
	g++ -std=c++11 -pthread -c allocate.cpp
clean:
	rm server server.o allocate.o
