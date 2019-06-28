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

OBJS = socket_handler.o accept_handler.o epoll_demultiplexer.o reactor_impl.o reactor.o

all: clean compile main client server

compile:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c socket_handler.cpp -o socket_handler.o
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c accept_handler.cpp -o accept_handler.o
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c epoll_demultiplexer.cpp -o epoll_demultiplexer.o
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c reactor_impl.cpp -o reactor_impl.o
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) -c reactor.cpp -o reactor.o

main:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) $(STLIB) main.cpp -o main

client:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) $(STLIB) client.cpp -o client

server:
	$(CC) $(CPPFLAGS) $(INC_PATH) $(LIB_PATH) $(SHRLIB) $(LIB) -Wl,-rpath=$(SHRLIB_PATH) $(OBJS) $(STLIB) epoll_server.cpp -o server



	#mv $@ $(BIN_PATH)
	#rm -f *.o 
	@printf "\033[32m SUCCESS \033[0m\n"


run:
	cd $(BIN)
	./ams
	cd $(SRC)
	@printf "\033[32m RUN EXIT!!! \033[0m\n"


clean:
	rm -f *.o main client server
