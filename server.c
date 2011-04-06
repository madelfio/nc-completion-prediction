// server:
//   server <filename to serve>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "def.h"
#include "sockets.h"
#include "file.h"
#include "peer.h"
#include "pack.h"
#include "matrix/gf_math.h"

/* GLOBALS */
char *arg_filename;
size_t nsegs = 0; // number of segments in file
int listen_socket = 0;
int send_socket = 0;
File file_info;
PeerNode peer_list;
int32_t server_cnt = 0;

/* LOCAL FUNCTION DECLARATIONS */
void usage() {fprintf(stderr, "usage: server <filename>\n");}
void parse_args(int argc, char **argv);
void serve_loop();
void process_incoming(PeerNode *peer_list);
void set_peer_status(struct sockaddr *peeraddr, 
    PeerNode *peer_list, char *buf);
void set_peer_estimate(struct sockaddr *peeraddr,
    PeerNode *peer_list, char *buf); 
void serve_peers(PeerNode *peer_list);
void serve_peer(Peer *peer);
void send_block(char *buffer, int s_idx, int b_idx, struct sockaddr* peer_addr);
void send_file_info(struct sockaddr *remote_addr); 
void encode_packet(byte *buf, byte *coeff, int seg_idx);
void send_packet(char *buf, int seg_idx, int coeff_seed,
    struct sockaddr *peer_addr);

int main(int argc, char **argv) {
  parse_args(argc, argv);

  gf_init();
  listen_socket = get_listen_socket(SERVERPORT);
  send_socket = get_socket();
  get_read_fp(&file_info, arg_filename);
  init_file(&file_info, SEGMENT_SIZE, BLOCK_SIZE);

  DLOG("listen_socket: %d\n", listen_socket);
  DLOG("send_socket: %d\n", send_socket);
  DLOG("got file!%s\n","");

  serve_loop();

  return 0;
}

// read hostname and filename, save to global variables
void parse_args(int argc, char **argv) {
  if (argc < 2) {
    usage();
    exit(1);
  }
  arg_filename = argv[1];
}


void serve_loop() {
  while(1) {
    process_incoming(&peer_list);
    serve_peers(&peer_list);
  }
}

void process_incoming(PeerNode *peer_list) {
  // see if we have any new incoming packets on the listen_socket
  // if we have new connections:
  // - add them to peer_list
  // if we have "STOP" messages:
  // - remove them from peer_list
  struct timeval tv;
  fd_set readfds;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  int numbytes;
  char buf[MAX_BUF_LEN];

  FD_ZERO(&readfds);
  FD_SET(listen_socket, &readfds);

  tv.tv_sec = 0;
  //tv.tv_usec = 500000;
  tv.tv_usec = 1;

  select((listen_socket+1), &readfds, NULL, NULL, &tv);

  if (FD_ISSET(listen_socket, &readfds)) {
    DLOG("%s","Got Something!!!\n");
    addrlen = sizeof(remoteaddr);
    numbytes = recvfrom(listen_socket, buf, MAX_BUF_LEN-1, 0,
        (struct sockaddr *)&remoteaddr, &addrlen);

    if (numbytes == -1) {
      perror("recvfrom");
    } 
    buf[numbytes] = '\0';
#ifdef DEBUG
    char remoteIP[INET6_ADDRSTRLEN];
    DLOG("new message from %s on socket %d, it sent %d bytes\n",
        inet_ntop(remoteaddr.ss_family, 
          get_in_addr((struct sockaddr *)&remoteaddr),
          remoteIP, INET6_ADDRSTRLEN),
        listen_socket,
        numbytes);
    DLOG("received message: %s\n", buf);
#endif

    if (strncmp((char *)buf, "S", 1) == 0) {
      DLOG("%s","client finished a segment!\n");
      set_peer_status((struct sockaddr *)&remoteaddr, peer_list, buf);
    } else if (strncmp((char *)buf, "E", 1) == 0) {
      DLOG("%s","client sent new segment estimate\n");
      set_peer_estimate((struct sockaddr *)&remoteaddr, peer_list, buf);
    } else if (strcmp((char *)buf, FI_REQ) == 0) {
      // Add new client to peer list
      add_to_peer_list((struct sockaddr *)&remoteaddr, peer_list);
      // Send file info
      send_file_info((struct sockaddr *)&remoteaddr);
      printf("file download initiated...\n");
    } else if (strcmp((char *)buf, DL_REQ) == 0) {
      DLOG("%s","sending file...\n");
      activate_peer((struct sockaddr *)&remoteaddr, peer_list);
    } else if (strcmp((char *)buf, END_REQ) == 0) {
      printf("%s","done sending file...\n");
      deactivate_peer((struct sockaddr *)&remoteaddr, peer_list);
    } else {
      printf("received unknown message: %s\n", buf);
    }
  } else {
    //DLOG("%s","Nothin...\n");
  }
}

void set_peer_status(struct sockaddr *peeraddr, 
    PeerNode *peer_list, char *buf) {
  int seg_idx;
  unpack((unsigned char *)(buf+1),"l",&seg_idx);
  set_peer_segment_idx(peeraddr, peer_list, seg_idx);
}

void set_peer_estimate(struct sockaddr *peeraddr,
    PeerNode *peer_list, char *buf) {
  int seg_idx, est_server_cnt;
  unpack((unsigned char *)(buf+1),"ll",&seg_idx,&est_server_cnt);
  //printf("setting peer estimate for seg %d to %d\n", seg_idx, est_server_cnt);
  set_peer_seg_estimate(peeraddr, peer_list, seg_idx, est_server_cnt);
}

void serve_peers(PeerNode *peer_list) {
  PeerNode *p;
  p = peer_list;
  
  for (p = peer_list; p-> next; p = p->next) {
    if (is_active(p->peer)) {
      serve_peer(p->peer); 
    }
  }
}

void serve_peer(Peer *peer) {
  int seg_idx = get_seg_idx(peer, server_cnt);
  int num_blocks = get_num_blocks(&file_info, seg_idx);
  byte coeff[num_blocks];
  int r_seed = rand();
  byte msg[BLOCK_SIZE];
  struct sockaddr_in *peer_addr;
  get_rand_bytes_s(coeff, num_blocks, r_seed);

  encode_packet(msg, coeff, seg_idx);
  peer_addr = &(peer->p_sin);
  send_packet((char *)msg, seg_idx, r_seed, (struct sockaddr *)peer_addr);
  incr_packets_sent(peer);
}

void encode_packet(byte *msg, byte *coeff, int seg_idx) {
  char buf[MAX_BUF_LEN];
  int num_blocks = get_num_blocks(&file_info, seg_idx);
  int32_t len; 
  // get segment string
  len = get_segment(&file_info, buf, seg_idx);

  // get packet using coeff, save in msg
  int i, j;
  for (i = 0; i < BLOCK_SIZE; i++) {
    msg[i] = 0;
    for (j = 0; j < num_blocks; j++) {
      msg[i] = Sum(msg[i], Product(buf[j*BLOCK_SIZE+i], coeff[j]));
    }
  }
/*  printf("Block 0:\n");
  for (i = 0; i < BLOCK_SIZE; i++) {
    printf("%d, ", (unsigned char)buf[i]);
  }
  printf("\n");*/
}


void send_packet(char *buf, int seg_idx, int coeff_seed,
    struct sockaddr *peer_addr) {
  char raw_msg[MAX_BUF_LEN];
  int32_t len;
  char pack_string[30];
  sprintf(pack_string, "slll%ds", BLOCK_SIZE);
  len = pack((unsigned char *)raw_msg, pack_string, DL_RSP, 
      (int32_t)seg_idx, (int32_t)coeff_seed, server_cnt++, buf);
  //DLOG("server_cnt: %d\n", server_cnt);
  if (sendto(send_socket, raw_msg, len, 0,
        peer_addr, sizeof(*peer_addr)) == -1) {
    perror("sendto");
  }

  // following is all for DEBUG
  //int i;
  //DLOG("send segment %d, coefficient seed %d\n", seg_idx, coeff_seed);
  /*printf("packet: ");
  for (i = 0; i < BLOCK_SIZE; i++) {
    printf("%d, ", (unsigned char)buf[i]);
  }
  printf("\n");*/
}


// file_info message:
// SIZE.SEGS
void send_file_info(struct sockaddr *remote_addr) {
  char raw_msg[MAX_BUF_LEN];
  int32_t len;
  raw_msg[0] = 'I'; // sending file Info
  len = pack((unsigned char *)(raw_msg + 1), "lll", 
      (int32_t)file_info.size, (int32_t)file_info.segment_size, 
      (int32_t)file_info.block_size);

  unsigned short int client_port = (unsigned short int)atoi(CLIENTPORT);
  ((struct sockaddr_in *)remote_addr)->sin_port = htons(client_port);

#ifdef DEBUG
  char remoteIP[INET6_ADDRSTRLEN];
  DLOG("sending to %s, port %d\n", 
      inet_ntop(remote_addr->sa_family, 
        get_in_addr(remote_addr), remoteIP, 
        INET6_ADDRSTRLEN), 
      get_in_port(remote_addr)); 
  DLOG("%d, %d, %d\n", (int32_t)file_info.size, (int32_t)file_info.segment_size, (int32_t)file_info.block_size);
#endif

  if (sendto(send_socket, raw_msg, len + 1, 0,
        remote_addr, sizeof(*remote_addr)) == -1) {
    perror("error sending file info");
    exit(1);
  }
  DLOG("%s","sent file info...\n");
}
