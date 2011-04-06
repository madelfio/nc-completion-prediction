#ifndef PEER_H
#define PEER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

typedef struct _peer {
  struct sockaddr_in p_sin;
  int segment_idx;
  int block_idx;
  int packets_sent;
  int est_server_cnt;
  int est_for_seg;
} Peer;

typedef struct _peer_node {
  Peer *peer;
  struct _peer_node *next;
} PeerNode;

void set_file_segments(size_t num_segments);
Peer *get_peer_by_addr(struct sockaddr *peer_addr, PeerNode *peer_list); 

void add_to_peer_list(struct sockaddr *peer_addr, PeerNode *peer_list);
void activate_peer(struct sockaddr *peer_addr, PeerNode *peer_list);
void deactivate_peer(struct sockaddr *peer_addr, PeerNode *peer_list);
void set_peer_segment_idx(struct sockaddr *peer_addr, PeerNode *peer_list, 
    int seg_idx);
void set_peer_seg_estimate(struct sockaddr *peer_addr, PeerNode *peer_list,
    int seg_idx, int est_server_cnt); 
int  is_active(Peer *peer);

void send_to_peer(Peer *peer, int send_socket, char *msg, int32_t msg_len);
void incr_packets_sent(Peer *peer);

size_t get_seg_idx(Peer *peer, int server_cnt);
size_t get_block_idx(Peer *peer);
void incr_seg_idx(Peer *peer, File *f);
void incr_block_idx(Peer *peer, File *f);
#endif
