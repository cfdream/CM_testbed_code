#ifndef __SEND_CONDITIOND_THREAD_H__
#define __SEND_CONDITIOND_THREAD_H__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "tcpr.h"
#include "common/sendpacket.h"


char g_pkt_buffer[15000];   //1.5kb

tcpreplay_t* g_tcpreplay_ctx;

typedef struct condition_s {
    uint32_t srcip;
}condition_t;

/**
* @brief 
*
* @param param_ptr
*
* @return 
*/
void *send_udp_condition_pkt(void* param_ptr) {
    condition_t* p_condition = (condition_t*)param_ptr;

    u_char src_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    u_char dst_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    //geneate a udp packet
    struct pcap_pkthdr pcap_header;
    struct tcpr_udp_hdr udp_header;
    struct tcpr_ipv4_hdr ip_header;
    struct tcpr_ethernet_hdr ethernet_header;
    int pkt_len;
    
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
    strncpy(ethernet_header.ether_dhost, dst_mac, 6);
    strncpy(ethernet_header.ether_shost, src_mac, 6);
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
    if(sendpacket(g_tcpreplay_ctx->intf1, g_pkt_buffer, pkt_len, &pcap_header) < pkt_len ) {
        printf("Unable to send packet\n");
    } 

    printf("send_condition_pkt succeeded\n");
}

#endif
