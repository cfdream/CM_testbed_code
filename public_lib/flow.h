#ifndef __FLOW_H__
#define __FLOW_H__

#include "stdlib.h"
#include "packet.h"

/*
typedef struct flow {
   u_int srcip;
   u_int dstip;
   u_short src_port;
   u_short dst_port;
   u_short protocol;
} flow_s;
*/

/**
* @brief 5-tuple flow
*/
typedef struct flow_ss {
    uint32_t srcip;
    uint32_t dstip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;  //l4 protocol
} flow_t, flow_s;

/*
 * <src> flow
 *
 */
typedef struct flow_src_s {
    uint32_t srcip;
} flow_src_t;

/*
 * the return value should be released by the call function
 */
flow_s* deep_copy_flow(flow_s* input);

int flow_compare(flow_s* flow1, flow_s* flow2);

/*
 * <src> flow
 *
 */

int flow_src_compare(flow_src_t* flow1, flow_src_t* flow2);

uint32_t flow_src_hash(flow_src_t* p_flow, uint32_t map_size);

uint32_t flow_5tuple_hash(flow_t* p_flow);

#endif

