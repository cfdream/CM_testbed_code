#include "sample_model.h"

//double (*get_pkt_sample_rate)(flow_src_t*, packet_t*) = &pkt_sample_rate_trad;

//traditional sample and hold
double pkt_sample_rate_trad(flow_src_t* flow_key, packet_t* p_pkt) {
    double rate = DEFAULT_BYTE_SAMPLE_RATE * p_pkt->len;            
    return rate > 1 ? 1 : rate;
}

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
