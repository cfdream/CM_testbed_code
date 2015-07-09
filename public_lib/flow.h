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
typedef packet_s flow_s;

flow_s* deep_copy_flow(flow_s* input) {
    flow_s* output = malloc(sizeof(flow_s));
    *output = *input;
    return output;
}

int flow_compare(flow_s* flow1, flow_s* flow2) {
    //srcip not equal
    if (flow1->srcip != flow2->srcip) {
        return flow1->srcip - flow2->srcip;
    }

    //srcip equal, check dstip
    if (flow1->dstip != flow2->dstip) {
        return flow1->dstip - flow2->dstip;
    }

    //srcip, dstip equal, check src_port
    if (flow1->src_port != flow2->src_port) {
        return flow1->src_port - flow2->src_port;
    }

    //srcip, dstip, src_port equal, check dst_port
    if (flow1->dst_port != flow2->dst_port) {
        return flow1->dst_port - flow2->dst_port;
    }

    //srcip, dstip, src_port, dst_port equal, check protocol
    if (flow1->protocol != flow2->protocol) {
        return flow1->protocol - flow2->protocol;
    }
    return 0;
}

/*
 * <src> flow
 *
 */
typedef flow_s flow_src_t;

int flow_src_compare(flow_src_t* flow1, flow_src_t* flow2) {
    return flow1->srcip - flow2->srcip;
}

#endif

