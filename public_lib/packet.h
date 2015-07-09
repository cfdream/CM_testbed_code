#ifndef PACKET_H
#define PACKET_H

#define MAX_MTU_SIZE 5000
#define MAX_PAYLOAD_SIZE (5000-58)

#define u_int uint32_t
#define u_short uint16_t

typedef struct packet {
   u_int srcip;
   u_int dstip;
   u_short src_port;
   u_short dst_port;
   u_short protocol;
   u_int len;
}packet_s;

typedef packet_s packet_t;

#endif
