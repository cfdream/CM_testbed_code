#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <netinet/in.h>
#include "../public_lib/packet.h"
#include "../public_lib/hashTableFlow.h"
#include "../tcpreplay/src/tcpr.h"

hashtable_kf_t *flow_seqid_hashmap;
pcap_t *pd;
pcap_dumper_t *pdumper;
FILE* fp;

u_int get_seqid_of_flow(packet_s* p_packet) {
    if (ht_kf_get(flow_seqid_hashmap, p_packet) < 0) {
        ht_kf_set(flow_seqid_hashmap, p_packet, 0);
    }
    u_int seqid = ht_kf_get(flow_seqid_hashmap, p_packet) + 1;
    ht_kf_set(flow_seqid_hashmap, p_packet, seqid);

    return seqid;
}

/**
* @brief 
*
* @param p_packet
* @param packet_buff
*
* @return pcap len of the packet, i.e., header length of the packet
*/
int generate_one_packet(packet_s* p_packet, char* packet_buff) {
    int payload_len = p_packet->len;
    u_short sport = p_packet->src_port;
    u_short dport = p_packet->dst_port;
    u_int srcip = p_packet->srcip;
    u_int dstip = p_packet->dstip;
    u_int seqid = 0;
    u_char src_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    u_char dst_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    
    struct tcpr_tcp_hdr tcp_header;
    struct tcpr_ipv4_hdr ip_header;
    struct tcpr_802_1q_hdr ethernet_header_vlan;

    memset(&tcp_header, sizeof(tcp_header), 0);
    memset(&ip_header, sizeof(ip_header), 0);
    memset(&ethernet_header_vlan, sizeof(ethernet_header_vlan), 0);

    //tcp header
    tcp_header.th_sport = htons(sport);
    tcp_header.th_dport = htons(dport);
    seqid = get_seqid_of_flow(p_packet);
    tcp_header.th_seq = htonl(seqid);
    tcp_header.th_off = 5;
    
    //ip header
    ip_header.ip_v = 0x4;
    ip_header.ip_hl = 0x5;
    ip_header.ip_len = htons(sizeof(tcp_header) + sizeof(ip_header));
    ip_header.ip_off = 0x4000;
    ip_header.ip_ttl = 0x40;
    ip_header.ip_p = 0x06;  //TCP-0x06, UDP-0x11
    ip_header.ip_src.s_addr = htonl(srcip);
    ip_header.ip_dst.s_addr = htonl(dstip);

    //ethernet header
    strncpy(ethernet_header_vlan.vlan_dhost, dst_mac, 6);
    strncpy(ethernet_header_vlan.vlan_shost, src_mac, 6);
    ethernet_header_vlan.vlan_tpi = htons(0x8100);
    ethernet_header_vlan.vlan_len = htons(0x0800);

    //printf("tcp header size:%u, ip header size:%u, ether header size:%u\n",
    //    sizeof(tcp_header), sizeof(ip_header), sizeof(ethernet_header_vlan));
    //printf("src_mac:%s, afterCopy:%s\n", src_mac, ethernet_header_vlan.ether_shost);

    memcpy(packet_buff, &ethernet_header_vlan, sizeof(ethernet_header_vlan));
    memcpy(packet_buff+sizeof(ethernet_header_vlan), &ip_header, sizeof(ip_header));
    memcpy(packet_buff+sizeof(ethernet_header_vlan)+sizeof(ip_header), &tcp_header, sizeof(tcp_header));

    /*debug 168379437-197988225-60826-1455-6*/
    //if (srcip == 168379437 && dstip == 197988225 && sport== 60826 && dport==1455) {
    //    printf("%d\n", seqid);
    //}
    /*end debug*/

    return sizeof(ethernet_header_vlan) + sizeof(ip_header) + sizeof(tcp_header);
}

int init_generate_pcpa_file(const char* out_pcap_fname) {
    /*create hashtable*/
    flow_seqid_hashmap = ht_kf_create(HASH_MAP_SIZE);

    pd = pcap_open_dead(DLT_EN10MB, 65535 /* snaplen */);
    if (pd == NULL) {
        return -1;
    }

    /* Create the output file. */
    pdumper = pcap_dump_open(pd, out_pcap_fname);
    if (pdumper == NULL) {
        return -1;
    }
    fp = fopen("trace.csv", "w");
    if (fp == NULL) {
        return -1;
    }
    return 0;
}

void close_generate_pcpa_file() {
    ht_kf_destory(flow_seqid_hashmap, HASH_MAP_SIZE);

    pcap_close(pd);
    pcap_dump_close(pdumper);
    fclose(fp);
}

void generate_one_pcap_pkt(packet_s* p_packet) {
    struct pcap_pkthdr pcap_header;
    int pkt_pcap_len = 0;
    char* packet_buffer = (char*) malloc(5000);

    /* generate a packet*/
    pkt_pcap_len = generate_one_packet(p_packet, packet_buffer);

    /* write packet_buffer to savefile */
    pcap_header.ts.tv_sec = 0xAA779F47;
    pcap_header.ts.tv_usec = 0x90A20400;
    pcap_header.caplen = pkt_pcap_len;
    pcap_header.len = pkt_pcap_len + p_packet->len;

    pcap_dump(pdumper, &pcap_header, packet_buffer);
   
    fprintf(fp, ",%u,%u,%u,%u,%d,%d\n", p_packet->srcip, p_packet->dstip, p_packet->src_port, p_packet->dst_port, p_packet->protocol, p_packet->len);

    free(packet_buffer);
}

//int main() {
//    pcap_t *pd;
//    pcap_dumper_t *pdumper;
//    struct pcap_pkthdr pcap_header;
//
//    int i = 0;
//    int pkt_pcap_len = 0;
//
//    pd = pcap_open_dead(DLT_EN10MB, 65535 /* snaplen */);
//
//    /* Create the output file. */
//    pdumper = pcap_dump_open(pd, "data/capture.pcap");
//
//    char* packet_buffer = (char*) malloc(5000);
//
//    //while (1) {
//    for (i = 0; i < 10; i++) {
//        /*
//         ** Create fake IP header and put UDP header
//         ** and payload in place
//         **/
//
//        /* generate a packet*/
//        pkt_pcap_len = generate_one_packet(packet_buffer);
//
//        /* write packet_buffer to savefile */
//        pcap_header.ts.tv_sec = 0xAA779F47;
//        pcap_header.ts.tv_usec = 0x90A20400;
//        pcap_header.caplen = pkt_pcap_len;
//        pcap_header.len = pkt_pcap_len;
//
//        pcap_dump(pdumper, &pcap_header, packet_buffer);
//    }
//
//
//    pcap_close(pd);
//    pcap_dump_close(pdumper);
//
//    return 0;
//}
