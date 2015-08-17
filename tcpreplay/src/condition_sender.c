#include <unistd.h>
#include <config.h>
#include "tcpreplay_api.h"
#include "condition_sender.h"
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
int send_udp_condition_pkt(condition_t* p_condition) {
    char src_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    char dst_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    //geneate a udp packet
    struct pcap_pkthdr pcap_header;
    struct tcpr_udp_hdr udp_header;
    struct tcpr_ipv4_hdr ip_header;
    struct tcpr_ethernet_hdr ethernet_header;
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
    ip_header.ip_src.s_addr = htonl(p_condition->srcip);   //set the condition_t.ip
    ip_header.ip_dst.s_addr = 0;
   
    //ethernet header
    strncpy((char*)ethernet_header.ether_dhost, dst_mac, 6);
    strncpy((char*)ethernet_header.ether_shost, src_mac, 6);
    ethernet_header.ether_type = htons(0x0800);

    memcpy(g_pkt_buffer, &ethernet_header, sizeof(ethernet_header));
    memcpy(g_pkt_buffer+sizeof(ethernet_header), &ip_header, sizeof(ip_header));
    memcpy(g_pkt_buffer+sizeof(ethernet_header)+sizeof(ip_header), &udp_header, sizeof(udp_header));
    pkt_len = sizeof(ethernet_header)+sizeof(ip_header)+sizeof(udp_header);

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
    uint64_t sec;
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
        uint64_t current_sec = get_next_interval_start(cm_experiment_setting.condition_sec_freq);
        int condition_pkt_num = 0;

        //lock to send all condition packets
        pthread_mutex_lock(&data_warehouse.packet_send_mutex);
		//lock the data_warehouse.data_warehouse_mutex
		//in order to avoid IntervalRotator thread destory the data
        pthread_mutex_lock(&data_warehouse.data_warehouse_mutex);

        printf("-----start send_condition_to_network, current_sec:%lu-----\n", current_sec);

        hashtable_kfs_vi_t* target_flow_map = data_warehouse_get_target_flow_map();
        entry_kfs_vi_t ret_entry;
        while (ht_kfs_vi_next(target_flow_map, &ret_entry) == 0) {
            //get one target flow, send to the network
            condition.srcip = ret_entry.key->srcip;
            send_udp_condition_pkt(&condition);
            ++condition_pkt_num;
            ++data_warehouse.condition_pkt_num_sent[data_warehouse.active_idx];
            //printf("condition srcip:%u\n", condition.srcip);
            free(ret_entry.key);
        }
        pthread_mutex_unlock(&data_warehouse.data_warehouse_mutex);
        pthread_mutex_unlock(&data_warehouse.packet_send_mutex);

        clock_gettime(CLOCK_REALTIME, &spec);
        sec = (intmax_t)((time_t)spec.tv_sec);
        printf("-----end send_udp_condition_pkt, current_sec:%lu, condition_pkts_sent:%d-----\n", sec, condition_pkt_num);
    }

    return NULL;
}
