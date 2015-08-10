#include "flow.h"

flow_s* deep_copy_flow(flow_s* input) {
    flow_s* output = malloc(sizeof(flow_s));
    *output = *input;
    return output;
}

int flow_compare(flow_s* flow1, flow_s* flow2) {
    //srcip not equal
    if (flow1->srcip != flow2->srcip) {
        if (flow1->srcip > flow2->srcip) {
            return 1;
        } else {
            return -1;
        }
    }

    //srcip equal, check dstip
    if (flow1->dstip != flow2->dstip) {
        if (flow1->dstip > flow2->dstip) {
            return 1;
        } else {
            return -1;
        }
    }

    //srcip, dstip equal, check src_port
    if (flow1->src_port != flow2->src_port) {
        if(flow1->src_port > flow2->src_port) {
            return 1;
        } else {
            return -1;
        }
    }

    //srcip, dstip, src_port equal, check dst_port
    if (flow1->dst_port != flow2->dst_port) {
        if (flow1->dst_port > flow2->dst_port) {
            return 1;
        } else {
            return -1;
        }
    }

    //srcip, dstip, src_port, dst_port equal, check protocol
    //if (flow1->protocol != flow2->protocol) {
    //    return flow1->protocol - flow2->protocol;
    //}
    return 0;
}

int flow_src_compare(flow_src_t* flow1, flow_src_t* flow2) {
    if (flow1->srcip == flow2->srcip) {
        return 0;
    } else if (flow1->srcip > flow2->srcip) {
        return 1;
    } else {
        return -1;
    }
}
