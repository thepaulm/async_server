CC=clang++
CFLAGS= -g -c -std=c++11
LFLAGS=

all: async_server

async_server: main.o sockets.o ioctx.o
	${CC} ${LFLAGS} -o async_server main.o sockets.o ioctx.o

main.o: main.cc
	${CC} ${CFLAGS} main.cc

sockets.o: sockets.cc
	${CC} ${CFLAGS} sockets.cc

ioctx.o: ioctx.cc
	${CC} ${CFLAGS} ioctx.cc

clean:
	rm -f *.o
	rm -f async_server
