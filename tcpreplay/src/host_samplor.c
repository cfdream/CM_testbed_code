#include "host_samplor.h"

int flow_src_already_sampled(flow_src_t* p_flow_src) {
    hashtable_kfs_t* flow_sample_map = data_warehouse_get_flow_sample_map();
    assert(flow_sample_map != NULL);
    if (ht_kfs_get(flow_sample_map, p_flow_src) >= 0) {
        return 1;
    }
    return 0;
}

int sample_packet(packet_t* p_packet) {
    double pkt_sample_rate;
    int pkt_sample_num;
    int rand_num;

    flow_src_t* p_flow_src = p_packet;

    if (flow_src_already_sampled(p_flow_src)) {
        return 1;
    }
    //packet sample rate
    pkt_sample_rate = pkt_sample_rate_trad(p_flow_src, p_packet);
    pkt_sample_num = (int)(pkt_sample_rate * RAND_MOD_NUMBER);
    //rand number
    rand_num = rand() % RAND_MOD_NUMBER + 1;

    if (rand_num <= pkt_sample_num) {
        //mark the flow as sampled
        hashtable_kfs_t* flow_sample_map = data_warehouse_get_flow_sample_map();
        assert(flow_sample_map != NULL);
        ht_kfs_set(flow_sample_map, p_flow_src, 1);

        return 1;
    }
    return 0;
}
