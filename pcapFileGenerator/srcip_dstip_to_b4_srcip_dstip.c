#include "srcip_dstip_to_b4_srcip_dstip.h"

int init_srcip_dstip_to_b4_srcip_dstip() {
    /*init hashmaps */
    srcip_to_node_idx_map = ht_kfs_vi_create();
    srcip_to_node_srcip_map = ht_kfs_vi_create();
    dstip_to_node_idx_map = ht_kfs_vi_create();
    dstip_to_node_dstip_map = ht_kfs_vi_create();
    if (srcip_to_node_idx_map == NULL ||
        srcip_to_node_srcip_map == NULL ||
        dstip_to_node_idx_map == NULL ||
        dstip_to_node_dstip_map == NULL) {
        printf("FAIL ht_kfs_vi_create hashtables\n");
        return -1;
    }

    /* unique srcip/dstip num */
    unique_srcip_num = 0;
    unique_dstip_num = 0;
    unique_ip_num = 0;

    return 0;
}

void destory_srcip_dstip_to_b4_srcip_dstip() {
    ht_kfs_vi_destory(srcip_to_node_idx_map);
    ht_kfs_vi_destory(srcip_to_node_srcip_map);
    ht_kfs_vi_destory(dstip_to_node_idx_map);
    ht_kfs_vi_destory(dstip_to_node_dstip_map);
}

/* get the mapped ip Nodeid, mapped srcip */
void get_mapped_info(uint32_t ip, uint32_t* p_mapped_nodeid, uint32_t* p_mapped_ip) {
    flow_src_t flow;
    flow.srcip = ip;
    flow.dstip = 0;
    
    int ret = ht_kfs_vi_get(srcip_to_node_idx_map, &flow);
    if (ret < 0) {
        //---a new ip appears
        //get the mapped node id
        ++unique_ip_num;
        *p_mapped_nodeid = unique_ip_num % NUM_SENDERS;
        ht_kfs_vi_set(srcip_to_node_idx_map, &flow, *p_mapped_nodeid);
        //get the mapped ip
        int num_ip_per_node = (unique_ip_num-1) / NUM_SENDERS;
        *p_mapped_ip = ITH_SENDER_START_IP(*p_mapped_nodeid) + num_ip_per_node;
        ht_kfs_vi_set(srcip_to_node_srcip_map, &flow, *p_mapped_ip);
    } else {
        *p_mapped_nodeid = ret;
        *p_mapped_ip = ht_kfs_vi_get(srcip_to_node_srcip_map, &flow);
    }
}

/* get the mapped ip Nodeid, mapped srcip */
uint32_t get_mapped_nodeid(hashtable_kfs_vi_t* ip_to_node_idx_map, uint32_t ip, uint64_t* p_unique_ip_num) {
    flow_src_t flow;
    flow.srcip = ip;
    flow.dstip = 0;
    
    int ret = ht_kfs_vi_get(ip_to_node_idx_map, &flow);
    if (ret < 0) {
        //a new ip appears
        ++(*p_unique_ip_num);
        int node_idx = *p_unique_ip_num % NUM_SENDERS;
        ht_kfs_vi_set(ip_to_node_idx_map, &flow, node_idx);
        return node_idx;
    } else {
        return ret;
    }
}

/*
 * unique_ip_num is increased in get_mapped_nodeid(), [1, ... ,]
 * */
uint32_t get_mapped_ip(hashtable_kfs_vi_t* ip_to_node_ip_map, uint32_t ip, uint64_t unique_ip_num) {
    flow_src_t flow;
    flow.srcip = ip;
    flow.dstip = 0;

    int ret = ht_kfs_vi_get(ip_to_node_ip_map, &flow); 
    if (ret < 0) {
        //---a new ip appears
        int node_idx = unique_ip_num % NUM_SENDERS;
        int num_ip_per_node = (unique_ip_num-1) / NUM_SENDERS;
        uint32_t mapped_ip = ITH_SENDER_START_IP(node_idx) + num_ip_per_node;
        ht_kfs_vi_set(ip_to_node_ip_map, &flow, mapped_ip);
        return mapped_ip;
    } else {
        return ret;
    }
}

