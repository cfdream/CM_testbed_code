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
typedef flow_s flow_t;
/*
 * <src> flow
 *
 */
typedef flow_s flow_src_t;

flow_s* deep_copy_flow(flow_s* input);

int flow_compare(flow_s* flow1, flow_s* flow2);

/*
 * <src> flow
 *
 */

int flow_src_compare(flow_src_t* flow1, flow_src_t* flow2);

#endif

