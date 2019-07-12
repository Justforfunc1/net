#LANG=C
SRC_PATH=./
BIN_PATH=./
INC_PATH=-I. -I ./
LIB_PATH=-L./
SHRLIB_PATH = ./

LIB= -lpthread
SHRLIB =
STLIB =

CC=g++
CPPFLAGS=-Wall -g  -std=c++11

OBJS =
INDEX_OBJS =

all: clean client server

compile:
	
	
client:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) $(STLIB) client.cpp -o client

server:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) $(STLIB) epoll_server.cpp -o server

run:
	cd $(BIN)
	./test
	cd $(SRC)
	@printf "\033[32m RUN EXIT!!! \033[0m\n"

clean:
	rm -f *.o client  server
