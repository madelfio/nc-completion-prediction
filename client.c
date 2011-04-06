// client:
//   client <server name> <filename to write>
#include <stdio.h>
#include <sys/time.h>

#include "def.h"
#include "sockets.h"
#include "file.h"
#include "pack.h"
#include "bitfield.h"
#include "segment.h"

/* GLOBALS */
char *arg_hostname;
char *arg_filename;
int listen_socket = 0;
int send_socket = 0;
File file_info;
struct timeval last_request;
int last_segment_requested = -1;
int useful_packet_cnt = 0;
int redundant_packet_cnt = 0;
int early_packet_cnt = 0;
int lindep_packet_cnt = 0;

/* LOCAL FUNCTION DECLARATIONS */
void usage() {fprintf(stderr, "usage: client <hostname> <filename>\n");}
void parse_args(int argc, char **argv);
void send_to_server(char *msg); 
void n_send_to_server(char *msg, int len); 
void request_file_info();
void wait_for_file_info();
void request_file();
void download_loop();
void process_incoming(Segment seg[], int low_seg_idx);
void check_est(Segment *seg, int seg_idx, int server_cnt);
void send_est_to_server(int segment_idx, int est_server_cnt); 
void request_end_of_transmission();
void print_status(BitField *bf);

int main(int argc, char **argv) {
  // read argv, initialize server name, filename to write to
  parse_args(argc, argv);

  gf_init();
  listen_socket = get_listen_socket(CLIENTPORT);
  send_socket = get_send_socket(arg_hostname, SERVERPORT);
  get_write_fp(&file_info, arg_filename);
  gettimeofday(&last_request, NULL);

  DLOG("send_socket: %d\n", send_socket);
  DLOG("listen_socket: %d\n", listen_socket);

  // request file information and wait for it
  request_file_info();
  DNOTE();
  wait_for_file_info();
  DNOTE();

  // request file pieces and download them
  request_file();
  download_loop();

  // send DONE message to server
  request_end_of_transmission();

  return 0;
}

// read hostname and filename, save to global variables
void parse_args(int argc, char **argv) {
  if (argc < 3) {
    usage();
    exit(1);
  }
  arg_hostname = argv[1];
  arg_filename = argv[2];
}

// request file information
void request_file_info() {
  DLOG("%s","requesting file info\n");
  char req[] = FI_REQ;
  send_to_server(req);
}

// request file download
void request_file(int send_socket) {
  DLOG("%s","requesting file download\n");
  char req[] = DL_REQ;
  send_to_server(req);
}


void request_segment(int segment_idx) {
  struct timeval now;
  gettimeofday(&now, NULL);
  int elap = ((now.tv_sec - last_request.tv_sec)*1000 +
      (now.tv_usec - last_request.tv_usec)/1000); 
  if (elap < 150 && last_segment_requested == segment_idx) {
    return;
  }
  last_request = now;
  last_segment_requested = segment_idx;
  DLOG("requesting segment %d from server\n", segment_idx);
  char req[50] = "S";
  int32_t len;
  len = pack((unsigned char *)(req + 1),"l",(int32_t)segment_idx);
  DLOG("len: %d\n", len);
  //req[len+1] = '\0';
  n_send_to_server(req, len+1);
}

void send_to_server(char *msg) {
  int numbytes;
  if ((numbytes = send(send_socket, msg, strlen(msg), 0)) == -1) {
    perror("client: send");
  }
  DLOG("sent numbytes: %d\n", numbytes);
}

void n_send_to_server(char *msg, int len) {
  int numbytes;
  if ((numbytes = send(send_socket, msg, len, 0)) == -1) {
    perror("client: send");
  }
  DLOG("sent numbytes: %d\n", numbytes);
}

void get_message(char *raw_msg) {
  struct sockaddr_storage remote_addr;
  int numbytes;
  socklen_t addr_len;
  addr_len = sizeof(remote_addr);
  if ((numbytes = recvfrom(listen_socket, raw_msg, MAX_BUF_LEN - 1, 0,
          (struct sockaddr *)&remote_addr, &addr_len)) == -1) {
    perror("recvfrom");
  }
  raw_msg[numbytes] = '\0';
  DLOG("received packet of %d bytes\n", numbytes);
}

void get_message_type(char *msg_type, char *raw_msg) {
  *msg_type = raw_msg[0];
}

void wait_for_file_info() {
  char raw_msg[MAX_BUF_LEN];
  char msg_type;
  get_message(raw_msg);
  get_message_type(&msg_type, raw_msg);

  if (msg_type == 'I') {
    int file_size, seg_size, block_size;
    unpack((unsigned char *)(raw_msg + 1), "lll", 
        &file_size, &seg_size, &block_size);
    prepare_download_file(&file_info, arg_filename, file_size);
    DLOG("file size: %d, seg size: %d, block size: %d\n",
        file_size, seg_size, block_size);
    init_file(&file_info, seg_size, block_size);
  }
}

void download_loop() {
  int num_segs = 0;
  BitField seg_bf;
  num_segs = get_num_segments(&file_info);
  Segment seg_info[num_segs];

  char buf[SEGMENT_SIZE + 1];
  buf[SEGMENT_SIZE] = '\0';

  int segment_idx = 0; // store the current minimum unfilled segment
  DLOG("%s", "downloading file\n");
  init_bf(&seg_bf, num_segs);

  int i;
  for (i = 0; i < num_segs; i++) {
    clear_segment(&seg_info[i]);
  }

  while (!isfull_bf(&seg_bf)) {
    print_status(&seg_bf);
    segment_idx = first_empty(&seg_bf);
    request_segment(segment_idx);
    init_segment(&seg_info[segment_idx], BLOCKS_PER_SEGMENT, BLOCK_SIZE);
    while (!is_segment_done(&seg_info[segment_idx])) {
      process_incoming(seg_info, segment_idx);
      request_segment(segment_idx);
    }
    DLOG("segment %d is done\n", segment_idx);
    get_decoded_segment(&seg_info[segment_idx], buf);
    write_segment(&file_info, buf, segment_idx); 
    free_segment(&seg_info[segment_idx]);

    set_bf(&seg_bf, segment_idx);
  }
  print_status(&seg_bf);
  printf("\n");
  del_bf(&seg_bf);

  int total_packet_cnt = (useful_packet_cnt + redundant_packet_cnt + 
      lindep_packet_cnt);
  printf("useful_packet_cnt: %d (%d%%)\n", 
      useful_packet_cnt, 100*useful_packet_cnt/total_packet_cnt);
  printf("redundant_packet_cnt: %d (%d%%)\n", 
      redundant_packet_cnt, 100*redundant_packet_cnt/total_packet_cnt);
  printf("early_packet_cnt: %d (%d%%)\n", 
      early_packet_cnt, 100*early_packet_cnt/total_packet_cnt);
  printf("lindep_packet_cnt: %d (%d%%)\n", 
      lindep_packet_cnt, 100*lindep_packet_cnt/total_packet_cnt);
  printf("total_packet_cnt: %d\n", total_packet_cnt);
}

int select_from_socket(char *buf) {
  struct timeval tv;
  fd_set readfds;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;
  int numbytes;

  FD_ZERO(&readfds);
  FD_SET(listen_socket, &readfds);

  tv.tv_sec = 2;
  tv.tv_usec = 500000;

  select((listen_socket+1), &readfds, NULL, NULL, &tv);

  if (FD_ISSET(listen_socket, &readfds)) {
    addrlen = sizeof(remoteaddr);
    numbytes = recvfrom(listen_socket, buf, MAX_BUF_LEN-1, 0,
        (struct sockaddr *)&remoteaddr, &addrlen);

    if (numbytes == -1) {
      perror("recvfrom");
    }
#ifdef DEBUG_BLAH
    char remoteIP[INET6_ADDRSTRLEN];
    DLOG("new message from %s on socket %d, it sent %d bytes\n",
        inet_ntop(remoteaddr.ss_family, 
          get_in_addr((struct sockaddr *)&remoteaddr),
          remoteIP, INET6_ADDRSTRLEN),
        listen_socket,
        numbytes);
#endif
    return 1;
  }
  return 0;
}

void process_incoming(Segment seg[], int low_seg_idx) {
  char buf[MAX_BUF_LEN];
  char msg_type[10];
  if (select_from_socket(buf)) {
    char unpack_str[30];
    int seg_idx, coeff_seed, server_cnt;
    byte row[BLOCKS_PER_SEGMENT + BLOCK_SIZE];
    byte *coeff = &row[0];
    byte *msg = &row[BLOCKS_PER_SEGMENT];
    sprintf(unpack_str,"10slll%ds",(MAX_BUF_LEN));
    unpack((unsigned char *)buf, unpack_str, &msg_type, &seg_idx, 
        &coeff_seed, &server_cnt, msg);
    if (strncmp(msg_type, DL_RSP, strlen(DL_RSP)) == 0) {
      //DLOG("segment index: %d, coefficient seed: %d, server count: %d\n", 
      //    seg_idx, coeff_seed, server_cnt);
      if (is_uninitialized(&seg[seg_idx])) {
        DLOG("activating segment %d\n", seg_idx);
        init_segment(&seg[seg_idx], BLOCKS_PER_SEGMENT, BLOCK_SIZE);
      }
      if (is_active(&seg[seg_idx])) {
        get_rand_bytes_s(coeff, BLOCKS_PER_SEGMENT, coeff_seed);
        if (add_new_packet(&seg[seg_idx], (unsigned char *)row)) {
          useful_packet_cnt++;
        } else {
          lindep_packet_cnt++;
        }
        if (seg_idx == low_seg_idx) {
          check_est(&seg[seg_idx], seg_idx, server_cnt);
        } else if (seg_idx > low_seg_idx) {
          early_packet_cnt++;
        }
      } else {
        redundant_packet_cnt++;
      }
    } else {
      printf("%s","received unknown message\n");
    }
  } else {
    DLOG("%s","Nothin...\n");
  }
}

void check_est(Segment *seg, int seg_idx, int server_cnt) {
  // if time since last est > X, update stats, send to server
  struct timeval now;
  gettimeofday(&now, NULL);
  if (seg->last_dof <= 0) {
    DLOG("%s","init'ing estimate\n");
    init_est(seg, server_cnt, now);
    return;
  }
  int elap = ((now.tv_sec - seg->last_time.tv_sec)*1000 +
      (now.tv_usec - seg->last_time.tv_usec)/1000);
  //printf("elap: %d\n", elap);
  if (elap < 50) {
    //printf("waiting till next time to update estimate\n");
    return;
  }
  if (update_stats(seg, server_cnt, now)) {
    int est_server_cnt = get_est(seg);
    send_est_to_server(seg_idx, est_server_cnt);
  }
}

void send_est_to_server(int segment_idx, int est_server_cnt) {
  DLOG("sending new estimate for segment %d to server\n", segment_idx);
  char req[50] = "E";
  int32_t len;
  len = pack((unsigned char *)(req + 1),"ll",
      (int32_t)segment_idx,est_server_cnt);
  DLOG("len: %d\n", len);
  DLOG("new est: %d\n", est_server_cnt);
  //req[len+1] = '\0';
  n_send_to_server(req, len+1);
}

void request_end_of_transmission() {
  DLOG("%s","requesting end of transmission\n");
  int numbytes;
  char req[] = END_REQ;
  if ((numbytes = send(send_socket, req, strlen(req), 0)) == -1) {
    perror("client: send");
    exit(1);
  }
  DLOG("sent numbytes: %d\n", numbytes);
}

int last_pct = -1;

void print_status(BitField *bf) {
  DLOG("count_bf(bf): %zu, numbits_bf(bf): %zu\n",
      count_bf(bf), numbits_bf(bf));
  int pct = 100 * count_bf(bf) / numbits_bf(bf);
  int seg_idx = first_empty(bf);
  if (pct > last_pct) {
#ifndef DEBUG
    printf("\r");
    printf("[%d%% complete, waiting for segment %d]",pct, seg_idx);
    fflush(stdout);
#else
    printf("[%d%% complete, waiting for segment %d]\n",pct, seg_idx);
#endif
    last_pct = pct;
  }
}
