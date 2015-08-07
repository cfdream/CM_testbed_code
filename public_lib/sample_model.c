#include "sample_model.h"

//double (*get_pkt_sample_rate)(flow_src_t*, packet_t*) = &pkt_sample_rate_trad;

//traditional sample and hold
double pkt_sample_rate_trad(int total_pkt_len) {
    double rate = DEFAULT_BYTE_SAMPLE_RATE * total_pkt_len;            
    return rate > 1 ? 1 : rate;
}

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
