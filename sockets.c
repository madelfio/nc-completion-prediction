#include "def.h"
#include "sockets.h"

int get_client_socket(char *portnumber) {
  struct addrinfo hints, *servinfo, *p;
  int rv, sockfd;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(NULL, portnumber, &hints, &servinfo)) != 0) {
    fprintf(stderr, "get_send_socket: getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("get_send_socket: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("get_send_socket: connect");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "get_send_socket: failed to bind socket\n");
    exit(1);
  }
  
  return sockfd;
}

int get_socket() {
  int sock_fd;
  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    perror("socket");
  return sock_fd;
}


int get_send_socket(char *hostname, char *portnumber) {
  struct addrinfo hints, *servinfo, *p;
  int rv, sockfd;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(hostname, portnumber, &hints, &servinfo)) != 0) {
    fprintf(stderr, "get_send_socket: getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("get_send_socket: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("get_send_socket: connect");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "get_send_socket: failed to bind socket\n");
    exit(1);
  }

  
  return sockfd;
}

int get_listen_socket(char *portnumber) {
  struct addrinfo hints, *ai, *p;
  int rv, listener;
  int yes = 1; // this is weird... why?  from beej's guide
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  if ((rv = getaddrinfo(NULL, portnumber, &hints, &ai)) != 0) {
    fprintf(stderr, "get_listen_socket: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      perror("get_listen_socket: socket");
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      perror("get_listen_socket: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "get_listen_socket: failed to bind socket\n");
    exit(1);
  }
   
#ifdef DEBUG
  char remoteIP[INET6_ADDRSTRLEN];

  DLOG("got listen socket on ip %s\n",
      inet_ntop(AF_INET,
        get_in_addr(p->ai_addr),
        remoteIP, INET6_ADDRSTRLEN)
      );
#endif

  printf("listening on port %d\n",
      get_in_port(p->ai_addr));

  freeaddrinfo(ai);
  return listener;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

unsigned short int get_in_port(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return ntohs(((struct sockaddr_in*)sa)->sin_port);
  }
  return ntohs(((struct sockaddr_in6*)sa)->sin6_port);
}
