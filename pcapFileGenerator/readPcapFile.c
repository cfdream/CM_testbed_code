#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../public_lib/flow.h"
#include "../public_lib/hashTableFlow.h"

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
    u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    u_short ether_type; /* IP? ARP? RARP? etc */
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

void swap_two_char(char* ch1, char* ch2) {
    (*ch1) ^= (*ch2);
    (*ch2) ^= (*ch1);
    (*ch1) ^= (*ch2);
}

void swap_big_endian(void* buffer, int len) {
    if(len == 2) {
        swap_two_char((char*)(buffer), (char*)(buffer+1));
        return;
    }
    if(len == 4) {
        swap_two_char((char*)buffer, (char*)(buffer+3));
        swap_two_char((char*)(buffer+1), (char*)(buffer+2));
        return;
    }
}

pcap_t *handle;			/* Session handle */
hashtable_t *flow_seqid_hashmap;

u_int get_expected_seqid_of_flow(packet_s* p_packet) {
    if (ht_get(flow_seqid_hashmap, p_packet) < 0) {
        ht_set(flow_seqid_hashmap, p_packet, 0);
    }
    u_int seqid = ht_get(flow_seqid_hashmap, p_packet) + 1;
    return seqid;
}

u_int set_seqid_of_flow(packet_s* p_packet, u_int seqid) {
    ht_set(flow_seqid_hashmap, p_packet, seqid);
}

void got_packet(u_char *args, const struct pcap_pkthdr *header,
	const u_char *packet_buffer) 
{
/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14

	const struct sniff_ethernet *ethernet; /* The ethernet header */
	const struct sniff_ip *ip; /* The IP header */
	const struct sniff_tcp *tcp; /* The TCP header */
	const char *payload; /* Packet payload */
    packet_s packet;

	u_int size_ip;
	u_int size_tcp;
    u_char protocol;
    int ether_type;

    printf("pkt_buffer:%x%x%x%x%x\n", *packet_buffer, *(packet_buffer+1), *(packet_buffer+2), *(packet_buffer+3), *(packet_buffer+4));

    //ethernet
	ethernet = (struct sniff_ethernet*)(packet_buffer);
    printf("ether_type:0x%x, pos_16:%x%x\n", ethernet->ether_type, *(packet_buffer+16), *(packet_buffer+17));
	
    //IP header
    ip = (struct sniff_ip*)(packet_buffer + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}
    if (ip->ip_p == 0x06) {
        //TCP header
        tcp = (struct sniff_tcp*)(packet_buffer + SIZE_ETHERNET + size_ip);
        size_tcp = TH_OFF(tcp)*4;
        if (size_tcp < 20) {
            printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
            return;
        }
        
        //init the TCP packet, check whether there are packets lost ahead
        /*
        packet.srcip = ip->ip_src.s_addr;
        packet.dstip = ip->ip_dst.s_addr;
        packet.src_port = tcp->th_sport;
        packet.dst_port = tcp->th_dport;
        packet.protocol = ip->ip_p;
        u_int expect_seqid = get_expected_seqid_of_flow(&packet);
        if (tcp->th_seq != expect_seqid) {
            printf("LOST: flow[%u-%u-%u-%u-%u] expect_seqid[%u], receive_seqid[%u]\n", 
                packet.srcip, packet.dstip, packet.src_port, packet.dst_port, packet.protocol,
                expect_seqid, tcp->th_seq);
        }
        set_seqid_of_flow(&packet, tcp->th_seq);
        */
        
        char* buf = inet_ntoa(ip->ip_src);
        char* srcip_str = malloc(strlen(buf)+1);
        strcpy(srcip_str, buf);
        buf = inet_ntoa(ip->ip_dst);
        const char* dstip_str = strcpy(malloc(strlen(buf)+1), buf);
        printf("tcp pkt, srcip:%s, dstip:%s, sport:%u, dport:%u, seqid=%u\n", 
            srcip_str, dstip_str,
            tcp->th_sport, tcp->th_dport, tcp->th_seq);
        payload = (u_char *)(packet_buffer + SIZE_ETHERNET + size_ip + size_tcp);
        
    } else if (ip->ip_p == 0x11) {
        //UDP header
        printf("udp pkt\n");
    } else {
        //other protocols
        printf("other protocol[%x] pkt\n", ip->ip_p);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: pcapFielGenerator input_pcap_file\n \
                Usage: read pcap file\n");
        return -1;
    }

    char* pcap_fname = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
    struct pcap_pkthdr* header;
    const u_char* packet_buffer;

    /* Open the session in promiscuous mode */
    handle = pcap_open_offline(pcap_fname, errbuf);
    if (handle == NULL) {
       fprintf(stderr, "Couldn't open pcap file %s: %s\n", pcap_fname, errbuf);
       return(2);
    }

    while (pcap_next_ex(handle, &header, &packet_buffer) > 0) {
        printf("%x-len:%u\n", packet_buffer, header->len);
        got_packet(NULL, header, packet_buffer);
    }

    /*
    pcap_loop(handle, 10000000, got_packet, NULL);
    */
    /* And close the session */
    pcap_close(handle);
    return(0);
}
