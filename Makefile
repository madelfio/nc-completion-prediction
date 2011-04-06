CC=gcc
CFLAGS=-Wall

all: server client

server: server.o sockets.o file.o peer.o pack.o \
	matrix/gf_math.o matrix/matrix_inverse.o

client: client.o sockets.o file.o pack.o bitfield.o \
	segment.o matrix/gf_math.o matrix/matrix_inverse.o 

server.o: server.c def.h sockets.h file.h peer.h pack.h matrix/gf_math.h

client.o: client.c def.h sockets.h file.h pack.h bitfield.h

sockets.o: sockets.c sockets.h def.h

file.o: file.c file.h def.h

peer.o: peer.c peer.h def.h

pack.o: pack.c pack.h def.h

bitfield.o: bitfield.c bitfield.h def.h 

segment.o: segment.c segment.h def.h

clean: 
	rm *.o
	rm matrix/*.o
