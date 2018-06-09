.PHONY: all log clean

CC = g++
DEBUG = -g -Wall
STD = c++11
DEP_INCLUDE_PATH= -I ../src/

all: log

log: log_test.o log.o
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) $^ -o $@ $(DEBUG)

log_test.o: log_test.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

log.o: ../src/log.cpp
	$(CC) -std=$(STD) $(DEP_INCLUDE_PATH) -c $^ $(DEBUG)

clean:
	rm log *.o *.log