#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pcap.h>
#include <netinet/in.h>
#include "../public_lib/packet.h"
#include "../public_lib/hashTableFlow.h"


#define ETHER_ADDR_LEN 6

/* Ethernet header */
struct sniff_ethernet {
    u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    u_short ether_type; /* IP? ARP? RARP? etc */
    u_char vlan[2];
};

/**
 * IEEE 802.1Q (Virtual Local Area Network) VLAN header, static header
 * size: 8 bytes
 */
struct tcpr_802_1q_hdr
{
    uint8_t vlan_dhost[ETHER_ADDR_LEN];  /**< destination ethernet address */
    uint8_t vlan_shost[ETHER_ADDR_LEN];  /**< source ethernet address */
    uint16_t vlan_tpi;                   /**< tag protocol ID */
    uint16_t vlan_priority_c_vid;        /**< priority | VLAN ID */
#define TCPR_802_1Q_PRIMASK   0x0007    /**< priority mask */
#define TCPR_802_1Q_CFIMASK   0x0001    /**< CFI mask */
#define TCPR_802_1Q_VIDMASK   0x0fff    /**< vid mask */
    uint16_t vlan_len;                   /**< length or type (802.3 / Eth 2) */
};

/* IP header, big endian */
struct sniff_ip {
    u_char ip_vhl;		/* version << 4 | header length >> 2 */
    u_char ip_tos;		/* type of service */
    u_short ip_len;		/* total length */
    u_short ip_id;		/* identification */
    u_short ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
    u_char ip_ttl;		/* time to live */
    u_char ip_p;		/* protocol */
    u_short ip_sum;		/* checksum */
    struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)


/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
    u_short th_sport;	/* source port */
    u_short th_dport;	/* destination port */
    tcp_seq th_seq;		/* sequence number */
    tcp_seq th_ack;		/* acknowledgement number */
    u_char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;		/* window */
    u_short th_sum;		/* checksum */
    u_short th_urp;		/* urgent pointer */
};

hashtable_t *flow_seqid_hashmap;
pcap_t *pd;
pcap_dumper_t *pdumper;

u_int get_seqid_of_flow(packet_s* p_packet) {
    if (ht_get(flow_seqid_hashmap, p_packet) < 0) {
        ht_set(flow_seqid_hashmap, p_packet, 0);
    }
    u_int seqid = ht_get(flow_seqid_hashmap, p_packet) + 1;
    ht_set(flow_seqid_hashmap, p_packet, seqid);

    return seqid;
}

/**
* @brief 
*
* @param p_packet
* @param packet_buff
*
* @return pcap len of the packet
*/
int generate_one_packet(packet_s* p_packet, char* packet_buff) {
    int payload_len = p_packet->len;
    u_short sport = p_packet->src_port;
    u_short dport = p_packet->dst_port;
    u_int srcip = p_packet->srcip;
    u_int dstip = p_packet->dstip;
    //u_char src_mac[6] = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02};
    //u_char src_mac[6] = "abcdef";
    u_char src_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    u_char dst_mac[6] = {0x7c, 0x7a, 0x91, 0x86, 0xb3, 0xe8};
    //u_char dst_mac[6] = {0x03, 0x03, 0x03, 0x04, 0x04, 0x04};

    struct sniff_tcp tcp_header;
    struct sniff_ip ip_header;
    struct sniff_ethernet ethernet_header;
    struct tcpr_802_1q_hdr ethernet_header_vlan;
    memset(&tcp_header, sizeof(tcp_header), 0);
    memset(&ip_header, sizeof(ip_header), 0);
    memset(&ethernet_header_vlan, sizeof(ethernet_header_vlan), 0);

    //tcp header
    tcp_header.th_sport = sport;
    tcp_header.th_dport = dport;
    tcp_header.th_seq = get_seqid_of_flow(p_packet);
    tcp_header.th_offx2 = 5<<4;
    
    //ip header
    ip_header.ip_vhl = 0x45;
    ip_header.ip_len = payload_len + sizeof(tcp_header);
    ip_header.ip_off = 0x4000;
    ip_header.ip_ttl = 0x40;
    ip_header.ip_p = 0x06;  //TCP-0x06, UDP-0x11
    ip_header.ip_src.s_addr = srcip;
    ip_header.ip_dst.s_addr = dstip;
   
    /*
    //ethernet header
    strncpy(ethernet_header.ether_dhost, dst_mac, 6);
    strncpy(ethernet_header.ether_shost, src_mac, 6);
    ethernet_header.ether_type = 0x0081;
    */

    //ethernet header
    strncpy(ethernet_header_vlan.vlan_dhost, dst_mac, 6);
    strncpy(ethernet_header_vlan.vlan_shost, src_mac, 6);
    ethernet_header_vlan.vlan_tpi = 0x0081;
    ethernet_header_vlan.vlan_len = 0x0008;

    //printf("tcp header size:%u, ip header size:%u, ether header size:%u\n",
    //    sizeof(tcp_header), sizeof(ip_header), sizeof(ethernet_header_vlan));
    //printf("src_mac:%s, afterCopy:%s\n", src_mac, ethernet_header_vlan.ether_shost);

    memset(packet_buff, sizeof(ethernet_header_vlan) + sizeof(ip_header) + sizeof(tcp_header)+payload_len, 0);
    memcpy(packet_buff, &ethernet_header_vlan, sizeof(ethernet_header_vlan));
    memcpy(packet_buff+sizeof(ethernet_header_vlan), &ip_header, sizeof(ip_header));
    memcpy(packet_buff+sizeof(ethernet_header_vlan)+sizeof(ip_header), &tcp_header, sizeof(tcp_header));

    return sizeof(ethernet_header_vlan) + sizeof(ip_header) + sizeof(tcp_header);
}

int init_generate_pcpa_file(const char* out_pcap_fname) {
    /*create hashtable*/
    flow_seqid_hashmap = ht_create(HASH_MAP_SIZE);

    pd = pcap_open_dead(DLT_EN10MB, 65535 /* snaplen */);
    if (pd == NULL) {
        return -1;
    }

    /* Create the output file. */
    pdumper = pcap_dump_open(pd, out_pcap_fname);
    if (pdumper == NULL) {
        return -1;
    }
    return 0;
}

void close_generate_pcpa_file() {
    ht_destory(flow_seqid_hashmap, HASH_MAP_SIZE);

    pcap_close(pd);
    pcap_dump_close(pdumper);
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
//    pdumper = pcap_dump_open(pd, "/tmp/capture.pcap");
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
