#ifndef PACKET_H
#define PACKET_H

typedef struct packet {
   u_int srcip;
   u_int dstip;
   u_short src_port;
   u_short dst_port;
   u_short protocol;
   u_int len;
}packet_s;

#endif
