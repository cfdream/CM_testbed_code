#ifndef SAMPLE_MODEL_H
#define SAMPLE_MODEL_H

#include <stdbool.h>
#include "target_flow_setting.h"
#include "sample_setting.h"
#include "flow.h"

//traditional sample and hold
double pkt_sample_rate_trad(int total_pkt_len);

/*function pointer to get the right byte sampling model*/
//extern double (*get_pkt_sample_rate)(flow_src_t*, packet_t*)

/*
//linear adjusting
double pkt_sample_rate_linear(int total_pkt_len) {
    return 1;
}

//exponential adjusting 
double pkt_sample_rate_exp(int total_pkt_len) {
    return 1;    
}

//log 
double pkt_sample_rate_log(int total_pkt_len) {
    return 1;    
}

//polynominal
double pkt_sample_rate_pol(int total_pkt_len) {
    return 1;
}
*/

#endif
