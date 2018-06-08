CC = g++
DEBUG = -g -Wall
STD = c++11
OBJS = server.o parse.o socket_epoll.o
OBJC = client.o parse.o

ALL: server client

server: $(OBJS)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)

client: $(OBJC)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)

server.o: server.cpp server.h parse.h
	$(CC) -std=$(STD) -c $^ $(DEBUG)

client.o: client.cpp client.h
	$(CC) -std=$(STD) -c $^ $(DEBUG)

parse.o: parse.cpp parse.h
	$(CC) -std=$(STD) -c $^ $(DEBUG)

socket_epoll.o: socket_epoll.cpp socket_epoll.h
	$(CC) -std=$(STD) -c $^ $(DEBUG)


.PHONY:
clean:
	@rm -f *.o *.h.gch client server
 
