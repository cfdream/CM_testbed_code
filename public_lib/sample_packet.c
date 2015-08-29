#include "sample_packet.h"

int flow_src_already_sampled(flow_src_t* p_flow_src, hashtable_kfs_vi_t* flow_sample_map) {
    if (ht_kfs_vi_get(flow_sample_map, p_flow_src) >= 0) {
        return 1;
    }
    return 0;
}

int sample_packet(packet_t* p_packet, int total_pkt_len, struct drand48_data* p_rand_buffer, hashtable_kfs_vi_t* flow_sample_map) {
    double pkt_sample_rate;
    double rand_float;

    flow_src_t* p_flow_src = p_packet;

    if (flow_src_already_sampled(p_flow_src, flow_sample_map)) {
        //printf("srcip:%u already in flow_sample_map\n", p_flow_src->srcip);
        return 1;
    }
    //packet sample rate
    pkt_sample_rate = pkt_sample_rate_trad(total_pkt_len);
    drand48_r(p_rand_buffer, &rand_float);    //[0,1)
    //printf("pkt_sample_rate:%f, rand_float:%f\n", pkt_sample_rate, rand_float);        
    if (rand_float < pkt_sample_rate) {
        //mark the flow as sampled
        ht_kfs_vi_set(flow_sample_map, p_flow_src, 1);
        return 1;
    }
    return 0;
}


int flow_src_already_sampled_fixSize_map(flow_src_t* p_flow_src, hashtable_kfs_fixSize_t* flow_sample_map) {
    if (ht_kfs_fixSize_get(flow_sample_map, p_flow_src) >= 0) {
        return 1;
    }
    return 0;
}

int sample_packet_fixSize_map(packet_t* p_packet, 
    int total_pkt_len, 
    struct drand48_data* p_rand_buffer, 
    hashtable_kfs_fixSize_t* flow_sample_map) {

    double pkt_sample_rate;
    double rand_float;

    flow_src_t* p_flow_src = p_packet;

    if (flow_src_already_sampled_fixSize_map(p_flow_src, flow_sample_map)) {
        //printf("srcip:%u already in flow_sample_map\n", p_flow_src->srcip);
        return 1;
    }
    //packet sample rate
    pkt_sample_rate = pkt_sample_rate_trad(total_pkt_len);
    drand48_r(p_rand_buffer, &rand_float);    //[0,1)
    //printf("pkt_sample_rate:%f, rand_float:%f\n", pkt_sample_rate, rand_float);        
    if (rand_float < pkt_sample_rate) {
        //mark the flow as sampled
        //NOTE: set the value as 0, as this the flow_sample_volume map
        ht_kfs_fixSize_set(flow_sample_map, p_flow_src, 0);
        return 1;
    }
    return 0;
}
