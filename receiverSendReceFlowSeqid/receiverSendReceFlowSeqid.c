/*
 * Receiver sends received <5_tuple_info, seqid> to sender
 */

#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "../public_lib/senderFIFOsManager.h"
#include "../tcpreplay/src/tcpr.h"
#include "../public_lib/receiver_2_sender_proto.h"

pcap_t *handle;			/* Session handle */

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
    recv_2_send_proto_t packet;

	u_int size_ip;
	u_int size_tcp;
    u_char protocol;
    u_int seqid;
    int ethernet_header_len = get_ethernet_header_len(packet_buffer);
    if(ethernet_header_len < 0) {
        printf("no ethernet header in the packet\n");
        return;
    }


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
        packet.rece_seqid = ntohl(tcp->th_seq);
        writeConditionToFIFO(
            get_sender_fifo_handler(packet.srcip), 
            &packet);

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

        printf("RECE: flow[%s-%s-%u-%u-%u], receive_seqid[%u]\n", 
            src_str, dst_str, packet.src_port, packet.dst_port, 
            packet.protocol, packet.rece_seqid);
        /*DEBUG END: print packet info*/

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

    return 0;
}

void tearDown() {
    /* And close the session */
    pcap_close(handle);
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
