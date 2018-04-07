CC = g++
DEBUG = -g
STD = c++11
OBJS = server.o
OBJC = client.o
SRCS = server.cpp
SRCC = client.cpp

ALL: server client

server: $(OBJS)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)
$(OBJS): $(SRCS)
	$(CC) -std=$(STD) -c $^ -o $@ $(DEBUG)

client: $(OBJC)
	$(CC) -std=$(STD) $^ -o $@ $(DEBUG)
$(OBJC): $(SRCC)
	$(CC) -std=$(STD) -c $^ -o $@ $(DEBUG)

.PHONY:
clean:
	@rm -f *.o client server 
 
