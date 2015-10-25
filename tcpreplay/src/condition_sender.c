#include <unistd.h>
#include <config.h>
#include "tcpreplay_api.h"
#include "condition_sender.h"
#include "vlan_tag.h"
#include "../public_lib/time_library.h"
#include "../public_lib/general_functions.h"
#include "../public_lib/cm_experiment_setting.h"

char g_pkt_buffer[15000];   //1.5kb

tcpreplay_t* g_tcpreplay_ctx;
extern cm_experiment_setting_t cm_experiment_setting;

/**
* @brief 
*
* @param param_ptr
*
* @return 
*/
int send_udp_condition_pkt(condition_t* p_condition, bool is_target_flow) {
    char src_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    char dst_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    //geneate a udp packet
    struct pcap_pkthdr pcap_header;
    struct tcpr_udp_hdr udp_header;
    struct tcpr_ipv4_hdr ip_header;
    //struct tcpr_ethernet_hdr ethernet_header;
    struct tcpr_802_1q_hdr ethernet_header_vlan;
    int pkt_len;
   
    if (g_tcpreplay_ctx == NULL || g_tcpreplay_ctx->intf1 == NULL) {
        printf("g_tcpreplay_ctx or intf1 NULL");
        return -1;
    } 

    assert(p_condition != NULL);
    assert(g_tcpreplay_ctx != NULL);
    
    //udp header
    udp_header.uh_sport = 0x01;
    udp_header.uh_dport = 0x01;
    udp_header.uh_ulen = htons(sizeof(struct tcpr_udp_hdr));
    
    //ip header
    ip_header.ip_v = 0x4;
    ip_header.ip_hl = 0x5;
    ip_header.ip_len = htons(sizeof(struct tcpr_ipv4_hdr) + sizeof(struct tcpr_udp_hdr));
    ip_header.ip_off = 0x4000;
    ip_header.ip_ttl = 0x40;
    ip_header.ip_p = 0x11;  //TCP-0x06, UDP-0x11
    #ifdef FLOW_SRC
    ip_header.ip_src.s_addr = htonl(p_condition->srcip);   //set the condition_t.ip
    #endif
    #ifdef FLOW_SRC_DST
    ip_header.ip_src.s_addr = htonl(p_condition->srcip);   //set the condition_t.ip
    ip_header.ip_dst.s_addr = htonl(p_condition->dstip);
    #endif
    ip_header.ip_dst.s_addr = 0;
   
    //ethernet header
    strncpy((char*)ethernet_header_vlan.vlan_dhost, dst_mac, 6);
    strncpy((char*)ethernet_header_vlan.vlan_shost, src_mac, 6);
    ethernet_header_vlan.vlan_tpi = htons(0x8100);
    ethernet_header_vlan.vlan_len = htons(0x0800);
    //ethernet_header.ether_type = htons(0x0800);
    if (is_target_flow) {
        // + delta info
        // now target flow, last non-target flow
        //tag one VLAN bit to mark the flow as target_flow
        tag_packet_as_target_flow(&ethernet_header_vlan);
    } else {
        // - delta info
        // now non-target flow, last target flow
        // not tagged flow is treated as non-target flow defaultly
        tag_packet_as_not_target_flow(&ethernet_header_vlan);
    }

    memcpy(g_pkt_buffer, &ethernet_header_vlan, sizeof(ethernet_header_vlan));
    memcpy(g_pkt_buffer+sizeof(ethernet_header_vlan), &ip_header, sizeof(ip_header));
    memcpy(g_pkt_buffer+sizeof(ethernet_header_vlan)+sizeof(ip_header), &udp_header, sizeof(udp_header));
    pkt_len = sizeof(ethernet_header_vlan)+sizeof(ip_header)+sizeof(udp_header);

    //pcap header
    pcap_header.ts.tv_sec = 0xAA779F47;
    pcap_header.ts.tv_usec = 0x90A20400;
    pcap_header.caplen = pkt_len;
    pcap_header.len = pkt_len;

    //send the packet 
    if(sendpacket(g_tcpreplay_ctx->intf1, (u_char*)g_pkt_buffer, pkt_len, &pcap_header) < pkt_len ) {
        printf("Unable to send packet\n");
        return -1;
    } 
    return 0;
}

void* send_condition_to_network(void* param_ptr) {
    struct timespec spec;
    uint64_t msec;
    condition_t condition;

    /* switch_sample needs to now the target flows as well
    if (cm_experiment_setting.host_or_switch_sample != HOST_SAMPLE) {
        return NULL;
    }
    */

    /* no replament needs to now the target flows as well
     * as switches needs to know which are the target flows
    if (!cm_experiment_setting.replacement) {
        //if replament is not setup, no need to send condition packets
        return NULL;
    }
    */

    /* debug */
    /* condition packet */
    /*
    int i = 0;
    for (; i < 500; i++) {
        condition.srcip = 0x0a000001;
        send_udp_condition_pkt(&condition);
    }
    */
    /* end debug */


    while (1) {
        // postpone till the next timestamp that condition should be sent 
        uint64_t current_msec = get_next_interval_start(cm_experiment_setting.condition_msec_freq);
        int plus_condition_pkt_num = 0;
        int minus_condition_pkt_num = 0;

        /* postpone CM_CONDITION_MILLISECONDS_POSTPOINE_FOR_SWITCH
         * This is to make sure that the switches can receive all the condition informational-         
         * This should be large enough to make sure the condition infor is used at switches in time.
         */
        usleep(1000*CM_CONDITION_MILLISECONDS_POSTPOINE_FOR_SWITCH);

        //lock to send all condition packets
        pthread_mutex_lock(&data_warehouse.packet_send_mutex);
		//lock the data_warehouse.data_warehouse_mutex
		//in order to avoid IntervalRotator thread destory the data
        pthread_mutex_lock(&data_warehouse.data_warehouse_mutex);

        printf("-----start send_condition_to_network, current_msec:%lu ms-----\n", current_msec);
        hashtable_kfs_vi_t* target_flow_map = data_warehouse_get_target_flow_map();
        entry_kfs_vi_t ret_entry;
        //1. for one flow in target_flow_map, if it does not exist in the last_sent_target_flow_map, send the + information
        while (ht_kfs_vi_next(target_flow_map, &ret_entry) == 0) {
            if (ht_kfs_vi_get(data_warehouse.last_sent_target_flow_map, &ret_entry.key) > 0) {
                //the flow is target flow now and last time
                continue;
            }
            //1.1 new target flow not sent last time
            //send the + delta info
            //get one target flow, send to the network
            #ifdef FLOW_SRC
            condition.srcip = ret_entry.key.srcip;
            #endif
            #ifdef FLOW_SRC_DST
            condition.srcip = ret_entry.key.srcip;
            condition.dstip = ret_entry.key.dstip;
            #endif

            send_udp_condition_pkt(&condition, true);
            ++plus_condition_pkt_num;
            ++data_warehouse.condition_pkt_num_sent[data_warehouse.active_idx];
            //printf("plus condition srcip:%u\n", condition.srcip);
        }

        //2. for one flow in last_sent_target_flow_map, if it does not exist in target_flow_map, send the - information
        while (ht_kfs_vi_next(data_warehouse.last_sent_target_flow_map, &ret_entry) == 0) {
            if (ht_kfs_vi_get(target_flow_map, &ret_entry.key) > 0) {
                //the flow is target flow now and last time
                continue;
            }
            //2.1 target flow sent last time but now not target flow
            //send the - delta info
            #ifdef FLOW_SRC
            condition.srcip = ret_entry.key.srcip;
            #endif
            #ifdef FLOW_SRC_DST
            condition.srcip = ret_entry.key.srcip;
            condition.dstip = ret_entry.key.dstip;
            #endif
            send_udp_condition_pkt(&condition, false);
            ++minus_condition_pkt_num;
            ++data_warehouse.condition_pkt_num_sent[data_warehouse.active_idx];
            //printf("minus condition srcip:%u\n", condition.srcip);
        }

        //3. copy the current target_flow_map into last_sent_target_flow_map
        ht_kfs_vi_destory(data_warehouse.last_sent_target_flow_map);
        data_warehouse.last_sent_target_flow_map = ht_kfs_vi_create();
        if (data_warehouse.last_sent_target_flow_map == NULL) {
            return NULL;
        }
        while (ht_kfs_vi_next(target_flow_map, &ret_entry) == 0) {
            //add the flow into last_sent_target_flow_map
            ht_kfs_vi_set(data_warehouse.last_sent_target_flow_map, &ret_entry.key, ret_entry.value);
        }

        pthread_mutex_unlock(&data_warehouse.data_warehouse_mutex);
        pthread_mutex_unlock(&data_warehouse.packet_send_mutex);

        clock_gettime(CLOCK_REALTIME, &spec);
        msec = (intmax_t)((time_t)spec.tv_sec*1000 + spec.tv_nsec/1000000);
        printf("-----end send_udp_condition_pkt, current_msec:%lu, plus_condition_pkts_sent:%d, minus_condition_pkt_num:%d-----\n", msec, plus_condition_pkt_num, minus_condition_pkt_num);
    }

    return NULL;
}
