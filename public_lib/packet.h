#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_MTU_SIZE 5000
#define MAX_PAYLOAD_SIZE (5000-58)
#define TAG_VLAN_PACKET_SAMPLED_VAL 0X01 
#define TAG_VLAN_TARGET_FLOW_VAL 0X02

typedef struct packet {
    uint32_t srcip;
    uint32_t dstip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;  //l4 protocol
    uint32_t seqid;
    uint16_t len;   //head len + payload len
    bool sampled;
    bool is_target_flow;
}packet_s;

typedef packet_s packet_t;

#endif
