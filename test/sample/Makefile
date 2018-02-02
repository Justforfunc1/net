#LANG=C
SRC_PATH=../sample
BIN_PATH=../bin
INC_PATH=-I. -I../include
LIB_PATH=-L../lib
SHRLIB_PATH = ../lib

LIB= -lpthread -lboost_system -lboost_thread -lboost_regex -lrt -llpsys
SHRLIB = $(SHRLIB_PATH)/liblpsys.so
STLIB = 

CC=g++
CPPFLAGS=-Wall -g 

OBJS = epoll_connection.o
INDEX_OBJS = 

all: clean compile test

compile:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c epoll_connection.cpp -o epoll_connection.o
	
	
test:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) test.cpp -g -o $(BIN_PATH)/test
	@printf "\033[32m SUCCESS \033[0m\n"

log_test:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) log_test.cpp -g -o test_log
	@printf "\033[32m SUCCESS \033[0m\n"

run:
	cd $(BIN)
	./test
	cd $(SRC)
	@printf "\033[32m RUN EXIT!!! \033[0m\n"

clean:
	rm -f *.o test 
