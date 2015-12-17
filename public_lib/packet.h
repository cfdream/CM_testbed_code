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
//bit 1
#define TAG_VLAN_PACKET_SAMPLED_VAL 1
//bit 2
//#define TAG_VLAN_TARGET_FLOW_VAL (1<<1)
//bit 2, 3 (TAG_VLAN_TARGET_FLOW_VAL is invalid if this is valid)
#define SELECTED_FLOW_VAL_MASK 0x00000006
//selected_level: (0, 1, 2, 3)
#define TAG_VLAN_SELECTED_FLOW_VAL(selected_level) ((selected_level << 1) & SELECTED_FLOW_VAL_MASK)
#define GET_VLAN_SELECTED_FLOW_VAL(vlan_val) ((vlan_val & SELECTED_FLOW_VAL_MASK) >> 1)

//skip the 5th bit of vlan_id, it changes even reset
//use the bits 4, no 5, 6, 7, 8, ..., 16
#define TAG_VLAN_FOR_SWITCH_I(i) ( (i < 1 ? 1<<(i+3) : 1<<(i+4)))  //i=0,1,...,SENDERS-1 ( 12-1=11 )

enum selected_level_e {
    NO_SELECTED_LEVEL = 0,
    SELECTED_LEVEL = 1,
    TARGET_LEVEL = 2
    // 3 is vacant now
};

typedef struct packet {
    uint32_t srcip;
    uint32_t dstip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;  //l4 protocol
    uint32_t seqid;
    uint16_t len;   //head len + payload len
    bool sampled;
    uint32_t selected_level;
}packet_s;

typedef packet_s packet_t;

#endif
