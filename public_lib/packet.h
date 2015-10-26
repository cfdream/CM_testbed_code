#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stdbool.h>

#define IP_MASK 0xFF000000
#define START_IP 0x0A000000
#define ONE_SENDER_IP_NUM 0x01000000 //(0x00FFFFFF+1)
#define ITH_SENDER_START_IP(i) (START_IP+ONE_SENDER_IP_NUM*(i)) //i=0,1,...,SENDERS-1

#define MAX_MTU_SIZE 5000
#define MAX_PAYLOAD_SIZE (5000-58)

//16 bits can be used at most
#define TAG_VLAN_PACKET_SAMPLED_VAL 1
#define TAG_VLAN_TARGET_FLOW_VAL (1<<1)

#define TAG_VLAN_FOR_SWITCH_I(i) (1<<(i+2))  //i=0,1,...,SENDERS-1

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
