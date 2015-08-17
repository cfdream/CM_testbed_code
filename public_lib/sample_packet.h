#ifndef __SAMPLOR_PACKET_H__
#define __SAMPLOR_PACKET_H__

#include <assert.h>
#include <stdbool.h>
#include "flow.h"
#include "sample_model.h"
#include "mt_hashtable_kFlowSrc_vInt.h"
#include "mt_hashtable_kFlowSrc_vInt_fixSize.h"

#define RAND_MOD_NUMBER 1000000

int flow_src_already_sampled(flow_src_t* p_flow_src, hashtable_kfs_vi_t* flow_sample_map);
int sample_packet(packet_t* p_packet, int total_pkt_len, struct drand48_data* p_rand_buffer, hashtable_kfs_vi_t* flow_sample_map);

int flow_src_already_sampled_fixSize_map(flow_src_t* p_flow_src, hashtable_kfs_vi_fixSize_t* flow_sample_map);
int sample_packet_fixSize_map(packet_t* p_packet, 
    int total_pkt_len, 
    struct drand48_data* p_rand_buffer, 
    hashtable_kfs_vi_fixSize_t* flow_sample_map, 
    hashtable_kfs_vi_fixSize_t* target_flow_map);
#endif
