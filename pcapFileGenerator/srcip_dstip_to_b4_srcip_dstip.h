#ifndef __GENERATOR2_H__
#define __GENERATOR2_H__

/*
 * Preprocessing caida traffic
 * The source IP set; destination IP set
 * source IP set divide to 12 nodes equally based on #IPs
 * destination set divide to 12 nodes equally based on #IPs
 * Each node has a set of source IPs and a set of destination IPs
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "../public_lib/flow.h"
#include "../public_lib/cm_experiment_setting.h"
#include "../public_lib/mt_hashtable_kFlowSrc_vInt.h"
#include "writePacketToPcapFile.h"

#define START_IP 0x0A000000
#define ONE_SENDER_IP_NUM 0x01000000 //(0x00FFFFFF+1)
#define ITH_SENDER_START_IP(i) (START_IP+ONE_SENDER_IP_NUM*(i)) //i=0,1,...,SENDERS-1

#define START_FILE_NO 130000
#define DELTA_FILE_NO 100

#define LINE_BUFFER_LEN 1024

/* hashmaps for mapping from srcip/dstip in caida trace to srcip/dstip in B4 topo */
hashtable_kfs_vi_t* srcip_to_node_idx_map;
hashtable_kfs_vi_t* srcip_to_node_srcip_map;
hashtable_kfs_vi_t* dstip_to_node_idx_map;
hashtable_kfs_vi_t* dstip_to_node_dstip_map;
/* unique srcip/dstip num */
uint64_t unique_srcip_num;
uint64_t unique_dstip_num;
uint64_t unique_ip_num;

int init_srcip_dstip_to_b4_srcip_dstip();
void destory_srcip_dstip_to_b4_srcip_dstip();

/* get the mapped ip Nodeid, mapped srcip */
void get_mapped_info(uint32_t ip, uint32_t* p_mapped_nodeid, uint32_t* p_mapped_ip);
uint32_t get_mapped_nodeid(hashtable_kfs_vi_t* ip_to_node_idx_map, uint32_t ip, uint64_t* p_unique_ip_num);
uint32_t get_mapped_ip(hashtable_kfs_vi_t* ip_to_node_ip_map, uint32_t ip, uint64_t unique_ip_num);

#endif
