#ifndef __WRITE_PKT_TO_PCAP_FILE_H__
#define __WRITE_PKT_TO_PCAP_FILE_H__

#include <stdlib.h>
#include <stdio.h>
#include <pcap/pcap.h>
#include <netinet/in.h>
#include "../public_lib/packet.h"
#include "../public_lib/hashTableFlow.h"
#include "../public_lib/cm_experiment_setting.h"
#include "../tcpreplay/src/tcpr.h"

#define ENABLE_WRITE_TO_PLAINTEXT true

hashtable_kf_t *flow_seqid_hashmap;

/* pcapdump files for all nodes */
pcap_t *pd[NUM_SENDERS];
pcap_dumper_t *pdumper[NUM_SENDERS];
FILE* fp[NUM_SENDERS];

int init_write_pkts_to_files(const char* out_pcap_fname);
void close_write_pkts_to_files();

/**
* @brief 
*
* @param p_packet
* @param packet_buff
*
* @return pcap len of the packet, i.e., header length of the packet
*/
int generate_one_packet(packet_s* p_packet, u_char* packet_buff);

void generate_one_pcap_pkt(packet_s* p_packet, int node_idx);

#endif
