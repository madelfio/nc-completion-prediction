#include <stdlib.h>
#include <stdio.h>

#include "def.h"
#include "peer.h"

void add_to_peer_list(struct sockaddr *peer_addr, PeerNode *peer_list) {
  Peer *new_p;
  PeerNode *new_pn;
  PeerNode *cur_p;
  struct sockaddr_in *remote = (struct sockaddr_in *)peer_addr;
  
  new_p = (Peer *)malloc(sizeof(Peer));
  new_p->p_sin = *remote;
  new_p->p_sin.sin_port = htons((unsigned short int)atoi(CLIENTPORT));
  new_p->segment_idx = -1;
  new_pn = (PeerNode *)malloc(sizeof(PeerNode));

  cur_p = peer_list;
  while (cur_p->next) {
    DLOG("%s","loopin\n");
    cur_p = cur_p->next;
  }
  cur_p->peer = new_p;
  cur_p->next = new_pn;
}

void activate_peer(struct sockaddr *peer_addr, PeerNode *peer_list) {
  PeerNode *cur_p;
  for (cur_p = peer_list; cur_p->next; cur_p = cur_p->next) {
    if (((struct sockaddr_in *)peer_addr)->sin_addr.s_addr ==
      (cur_p->peer->p_sin).sin_addr.s_addr) {
      DLOG("%s","activated peer\n");
      cur_p->peer->segment_idx = 0;
      cur_p->peer->block_idx = 0;
      cur_p->peer->packets_sent = 0;
      cur_p->peer->est_server_cnt = 0;
      break;
    }
  }
}

void deactivate_peer(struct sockaddr *peer_addr, PeerNode *peer_list) {
  PeerNode *cur_p;
  for (cur_p = peer_list; cur_p->next; cur_p = cur_p->next) {
    if (((struct sockaddr_in *)peer_addr)->sin_addr.s_addr ==
      (cur_p->peer->p_sin).sin_addr.s_addr) {
      cur_p->peer->segment_idx = -2;
      DLOG("%s","deactivated peer\n");
      printf("sent peer %d blocks\n", cur_p->peer->packets_sent);
      break;
    }
  }
}

void set_peer_segment_idx(struct sockaddr *peer_addr, PeerNode *peer_list, 
    int seg_idx) {
  Peer *peer;
  if ((peer = get_peer_by_addr(peer_addr, peer_list))) {
    peer->segment_idx = seg_idx;
    DLOG("set segment_idx to %d\n", seg_idx);
  }
  /*
  PeerNode *cur_p;
  for (cur_p = peer_list; cur_p->next; cur_p = cur_p->next) {
    if (((struct sockaddr_in *)peer_addr)->sin_addr.s_addr ==
      (cur_p->peer->p_sin).sin_addr.s_addr) {
      cur_p->peer->segment_idx = seg_idx;
      DLOG("set segment_idx to %d\n", seg_idx);
      break;
    }
  }*/
}

void set_peer_seg_estimate(struct sockaddr *peer_addr, PeerNode *peer_list,
    int seg_idx, int est_server_cnt) {
  Peer *peer;
  if ((peer = get_peer_by_addr(peer_addr, peer_list))) {
    if (seg_idx <= peer->segment_idx) {
      peer->est_server_cnt = est_server_cnt;
      peer->est_for_seg = seg_idx;
    }
  }
}

Peer *get_peer_by_addr(struct sockaddr *peer_addr, PeerNode *peer_list) {
  PeerNode *cur_p;
  for (cur_p = peer_list; cur_p->next; cur_p = cur_p->next) {
    if (((struct sockaddr_in *)peer_addr)->sin_addr.s_addr ==
      (cur_p->peer->p_sin).sin_addr.s_addr) {
        return cur_p->peer;
    }
  }
  return NULL;
}

void send_to_peer(Peer *peer, int send_socket, char *msg, int32_t msg_len) {
  struct sockaddr *addr = (struct sockaddr *)&(peer->p_sin);
  if (sendto(send_socket, msg, msg_len, 0,
        addr, sizeof(*addr)) == -1)
    perror("sendto");
}

void incr_packets_sent(Peer *peer) {
  peer->packets_sent++;
}
  
int is_active(Peer *peer) {
  return (peer->segment_idx >= 0);
}

size_t get_seg_idx(Peer *peer, int server_cnt) {
#ifdef USE_EST
  // rand() % 10 gives a 90% chance of increasing seg_idx
  if (peer->est_server_cnt > 0 && 
      server_cnt > (peer->est_server_cnt) && 
      peer->est_for_seg == peer->segment_idx &&
      (rand() % 10)) {
    //printf("using increased seg_idx (%d). est: %d, server_cnt: %d\n",
    //    peer->segment_idx + 1, peer->est_server_cnt, server_cnt);
  
    return peer->segment_idx + 1;
  }
#endif
  return peer->segment_idx;
}

void incr_seg_idx(Peer *peer, File *f) {
  peer->segment_idx++;
  peer->segment_idx %= get_num_segments(f);
  peer->est_server_cnt = 0;
}

size_t get_block_idx(Peer *peer) {
  return peer->block_idx;
}

void incr_block_idx(Peer *peer, File *f) {
  DLOG("segment_idx: %d\n", peer->segment_idx);
  DLOG("block_idx: %d\n", peer->block_idx);
  DLOG("num_blocks: %zu\n", get_num_blocks(f, peer->segment_idx));

  peer->block_idx++;
  peer->block_idx %= get_num_blocks(f, peer->segment_idx);
}
