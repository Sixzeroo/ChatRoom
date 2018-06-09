CC = g++
DEBUG = -g -Wall
STD = c++11
OBJS = server.o parse.o socket_epoll.o log.o
OBJC = client.o parse.o log.o
DEP_INCLUDE_PATH= -I ./

ALL: server client

server: $(OBJS)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)

client: $(OBJC)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)

server.o: server.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

client.o: client.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

log.o: log.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

parse.o: parse.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

socket_epoll.o: socket_epoll.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)


.PHONY:
clean:
	@rm -f *.o *.h.gch client server
 
