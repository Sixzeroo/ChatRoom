CC = g++
DEBUG = -g
OBJS = server.o
OBJC = client.o
SRCS = server.cpp
SRCC = client.cpp

ALL: server client

server: $(OBJS)
	$(CC) $^ -o $@ $(DEBUG)
$(OBJS): $(SRCS)
	$(CC) -c $^ -o $@ $(DEBUG)

client: $(OBJC)
	$(CC) $^ -o $@ $(DEBUG)
$(OBJC): $(SRCC)
	$(CC) -c $^ -o $@ $(DEBUG)

.PHONY:
clean:
	@rm -f *.o client server 
 
