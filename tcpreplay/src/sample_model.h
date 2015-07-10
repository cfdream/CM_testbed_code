#ifndef SAMPLE_MODEL_H
#define SAMPLE_MODEL_H

#include <stdbool.h>
#include "../../public_lib/target_flow_setting.h"
#include "../../public_lib/flow.h"

#define OVER_SAMPLING_RATIO 4
#define DEFAULT_BYTE_SAMPLE_RATE (1.0 / TARGET_FLOW_VOLUME * OVER_SAMPLING_RATIO)

//traditional sample and hold
double pkt_sample_rate_trad(flow_src_t* flow_key, packet_t* p_pkt);

/*function pointer to get the right byte sampling model*/
//extern double (*get_pkt_sample_rate)(flow_src_t*, packet_t*)

/*
//linear adjusting
double pkt_sample_rate_linear(flow_src_t* flow_key, packet_t* p_pkt) {
    return 1;
}

//exponential adjusting 
double pkt_sample_rate_exp(flow_src_t* flow_key, packet_t* p_pkt) {
    return 1;    
}

//log 
double pkt_sample_rate_log(flow_src_t* flow_key, packet_t* p_pkt) {
    return 1;    
}

//polynominal
double pkt_sample_rate_pol(flow_src_t* flow_key, packet_t* p_pkt) {
    return 1;    
}
*/

#endif
