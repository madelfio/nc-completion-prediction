#ifndef SOCKETS_H
#define SOCKETS_H

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string.h>

int get_socket();
int get_send_socket(char *hostname, char *portnumber);
int get_listen_socket(char *portnumber);

void *get_in_addr(struct sockaddr *sa);
unsigned short int get_in_port(struct sockaddr *sa);

#endif
