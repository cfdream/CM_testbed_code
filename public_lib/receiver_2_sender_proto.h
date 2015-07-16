#ifndef __RECEIVER_2_SENDER_PROTO_H__
#define __RECEIVER_2_SENDER_PROTO_H__

#include <stdint.h>

typedef struct recv_2_send_proto_s {
    /*5-tuple identity*/
    uint32_t srcip;
    uint32_t dstip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    /*received seqid of the flow*/
    uint32_t rece_seqid;
} recv_2_send_proto_t;

#endif
