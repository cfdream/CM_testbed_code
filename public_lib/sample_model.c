#include "sample_model.h"

extern cm_experiment_setting_t cm_experiment_setting;

//double (*get_pkt_sample_rate)(flow_src_t*, packet_t*) = &pkt_sample_rate_trad;

//traditional sample and hold
double pkt_sample_rate_trad(int total_pkt_len) {
    double rate = cm_experiment_setting.sample_hold_setting.default_byte_sampling_rate * total_pkt_len;            
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
