#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "../public_lib/flow.h"
#include "../public_lib/hashtable/hashTableFlow.h"
#include "../tcpreplay/src/tcpr.h"
#include "../public_lib/fifo/senderFIFOManager.h"


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

/*
 * Function returns the Layer 3 protocol type of the given packet, or TCPEDIT_ERROR on error
 * from tcpreplay/src/tcpedit/plugins/dlt_en10mb/en10mb.c
 */
int 
get_ethernet_header_len(const u_char *packet)
{
/* ethernet headers are l3 proto not 0x8100*/
#define SIZE_ETHERNET 14
/* ethernet headers are l3 proto 0x8100*/
#define SIZE_ETHERNET_VLAN 18

    struct tcpr_ethernet_hdr *eth = NULL;
    struct tcpr_802_1q_hdr *vlan = NULL;
    
    eth = (struct tcpr_ethernet_hdr *)packet;
    switch (ntohs(eth->ether_type)) {
        case ETHERTYPE_VLAN:
            vlan = (struct tcpr_802_1q_hdr *)packet;
            return SIZE_ETHERNET_VLAN;
            break;
        
        default:
            return SIZE_ETHERNET;
            break;
    }
    return -1;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header,
	const u_char *packet_buffer) 
{
    struct tcpr_tcp_hdr *tcp;
    struct tcpr_ipv4_hdr *ip;
    struct tcpr_ethernet_hdr *ethernet;

	const char *payload; /* Packet payload */
    packet_s packet;

	u_int size_ip;
	u_int size_tcp;
    u_char protocol;
    u_int seqid;
    int ethernet_header_len = get_ethernet_header_len(packet_buffer);
    if(ethernet_header_len < 0) {
        printf("no ethernet header in the packet\n");
        return;
    }

    condition_t condition;

    //ethernet
	ethernet = (struct tcpr_ethernet_hdr *)(packet_buffer);
	
    //IP header
    ip = (struct tcpr_ipv4_hdr *)(packet_buffer + ethernet_header_len);
	size_ip = (ip->ip_hl)*4;
	if (size_ip < 20) {
		printf("   * Invalid IP header length: %u bytes\n", size_ip);
		return;
	}
    if (ip->ip_p == 0x06) {
        //TCP header
        tcp = (struct tcpr_tcp_hdr *)(packet_buffer + ethernet_header_len + size_ip);
        size_tcp = (tcp->th_off)*4;
        if (size_tcp < 20) {
            printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
            return;
        }
        
        //init the TCP packet, check whether there are packets lost ahead
        packet.srcip = ntohl(ip->ip_src.s_addr);
        packet.dstip = ntohl(ip->ip_dst.s_addr);
        packet.src_port = ntohs(tcp->th_sport);
        packet.dst_port = ntohs(tcp->th_dport);
        packet.protocol = ip->ip_p;
        seqid = ntohl(tcp->th_seq);
        u_int expect_seqid = get_expected_seqid_of_flow(&packet);

        /*DEBUG: print packet info*/
        struct in_addr src_addr;
        struct in_addr dst_addr;
        char src_str[100];
        char dst_str[100];
        src_addr.s_addr = htonl(packet.srcip);
        char* temp = (char*)inet_ntoa(src_addr);
        memcpy(src_str, temp, strlen(temp));
        src_str[strlen(temp)] = 0;
        dst_addr.s_addr = htonl(packet.dstip);
        temp = (char*)inet_ntoa(dst_addr);
        memcpy(dst_str, temp, strlen(temp));
        dst_str[strlen(temp)] = 0;

        printf("RECE: flow[%s-%s-%u-%u-%u] expect_seqid[%u], receive_seqid[%u]\n", 
            src_str, dst_str, packet.src_port, packet.dst_port, packet.protocol,
            expect_seqid, seqid);
        /*DEBUG END: print packet info*/
        if (seqid > expect_seqid) {
            printf("LOST: flow[%s-%s-%u-%u-%u] expect_seqid[%u], receive_seqid[%u]\n", 
                src_str, dst_str, packet.src_port, packet.dst_port, packet.protocol,
                expect_seqid, seqid);

            condition.srcip = packet.srcip;
            /*send [expect_seqid, seqid) to related sender*/
            uint32_t i_seqid = 0;
            for (i_seqid = expect_seqid; i_seqid < seqid; ++i_seqid) {
                //write the condition information to the FIFO
                condition.lost_seqid = i_seqid;
                writeConditionToFIFO(
                    get_sender_fifo_handler(condition.srcip), 
                    &condition);
            }
        }
        //set the received seqid
        set_seqid_of_flow(&packet, seqid);

        /*
        const char* buf = inet_ntoa(ip->ip_src);
        char* srcip_str = malloc(strlen(buf)+1);
        strcpy(srcip_str, buf);
        buf = inet_ntoa(ip->ip_dst);
        const char* dstip_str = strcpy(malloc(strlen(buf)+1), buf);
        printf("tcp pkt, srcip:%s, dstip:%s, sport:%u, dport:%u, seqid=%u\n", 
            srcip_str, dstip_str,
            tcp->th_sport, tcp->th_dport, tcp->th_seq);
        payload = (u_char *)(packet_buffer + ethernet_header_len + size_ip + size_tcp);
        */
    } else if (ip->ip_p == 0x11) {
        //UDP header
        printf("udp pkt\n");
    } else {
        //other protocols
        printf("other protocol pkt\n");
    }
}

int setup() {
   char *dev;			/* The device to sniff on */
   char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
   struct bpf_program fp;		/* The compiled filter */
   //char filter_exp[] = "port 23";	/* The filter expression */
   //char filter_exp[] = "tcp";	/* The filter expression */
   //only keep tcp packets, and dst (included in generated pcap files for senders)
   char filter_exp[] = "tcp and ether dst 7c:7a:91:86:b3:e8";	/* The filter expression */
   //char filter_exp[] = "";
   bpf_u_int32 mask;		/* Our netmask */
   bpf_u_int32 net;		/* Our IP */
   struct pcap_pkthdr header;	/* The header that pcap gives us */

   /* Define the device */
   dev = pcap_lookupdev(errbuf);
   printf("NIC:%s\n", dev);
   if (dev == NULL) {
       fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
       return -1;
   }
   /* Find the properties for the device */
   if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
       fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
       net = 0;
       mask = 0;
   }
   /* Open the session in promiscuous mode */
   handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
   if (handle == NULL) {
       fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
       return(2);
   }
   /* Compile and apply the filter */
   if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
       fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
       return -1;
   }
   if (pcap_setfilter(handle, &fp) == -1) {
       fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
       return -1;
   }

    /*create hashtable*/
    flow_seqid_hashmap = ht_create(HASH_MAP_SIZE);

    return 0;
}

void tearDown() {
    /* And close the session */
    pcap_close(handle);

    ht_destory(flow_seqid_hashmap, HASH_MAP_SIZE);
}

int main(int argc, char *argv[])
{
    /*open_fifos*/
    if(open_fifos() != 0) {
        return -1;
    }

    if (setup() < 0) {
        printf("setup failed\n");
        return -1;
    }

    pcap_loop(handle, 10000000, got_packet, NULL);
    tearDown();

    //TODO: should close the FIFO, not sure whether they would be closed automatically after the program exits
    //BTW, don't know how to do it with pcap_loop running.

    return(0);
}
