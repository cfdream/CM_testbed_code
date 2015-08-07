/*
 * Receiver sends received <5_tuple_info, seqid> to sender
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "../public_lib/senderFIFOsManager.h"
#include "../tcpreplay/src/tcpr.h"
#include "../public_lib/receiver_2_sender_proto.h"
#include "../public_lib/debug_config.h"
#include "../public_lib/ip_prefix_setting.h"

pcap_t *handle;            /* Session handle */
uint32_t net_identity;
uint32_t g_received_pkts = 0;
uint32_t g_received_condition_pkts = 0;

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

        /*filter packets that sent out from this host*/
        if (!(GET_NET_IDENTITY(packet.srcip) ^ net_identity)) {
            return;
        }
        //printf("pkt net identity:%u, xor_net_identity:%u\n", GET_NET_IDENTITY(packet.srcip), GET_NET_IDENTITY(packet.srcip)^net_identity);

        /*write to the FIFO of related sender*/
        writeConditionToFIFO(
            get_sender_fifo_handler(packet.srcip), 
            &packet);

        /*
        if (ENABLE_DEBUG && packet.srcip == DEBUG_SRCIP && packet.dstip == DEBUG_DSTIP &&
            packet.src_port == DEBUG_SPORT && packet.dst_port == DEBUG_DPORT) {
            int fifo_idx = GET_SENDER_IDX(packet.srcip);
            char src_str[100];
            char dst_str[100];
            ip_to_str(packet.srcip, src_str, 100);
            ip_to_str(packet.dstip, dst_str, 100);

            printf("receiver: flow[%s-%s-%u-%u-%u-sendToSender%d]\n", 
                src_str, dst_str, 
                packet.src_port, packet.dst_port, packet.rece_seqid, fifo_idx);
        }
        */
        ++g_received_pkts;
        if (!(g_received_pkts % NUM_PKTS_TO_DEBUG)) {
            printf("rece pkts:%u\n", g_received_pkts);
        }
    } else if (ip->ip_p == 0x11) {
        ++g_received_condition_pkts;
        if (!(g_received_condition_pkts % NUM_CONDITION_PKTS_TO_DEBUG)) {
            printf("rece condition pkts:%u\n", g_received_condition_pkts);
        }
        /*
        if (ENABLE_DEBUG) {
            //UDP header
            char src_str[100];
            memset(src_str, 0, sizeof(src_str));
            char* temp = inet_ntoa(ip->ip_src);
            memcpy(src_str, temp, strlen(temp));
            printf("udp pkt, srcip-%s\n", src_str);
        }
        */
    } else {
        //other protocols
        printf("other protocol pkt\n");
    }
}

int setup() {
   char *dev;            /* The device to sniff on */
   char errbuf[PCAP_ERRBUF_SIZE];    /* Error string */
   struct bpf_program fp;        /* The compiled filter */
   //char filter_exp[] = "port 23";    /* The filter expression */
   //char filter_exp[] = "tcp";    /* The filter expression */
   //only keep tcp packets, and dst (included in generated pcap files for senders)
   //char filter_exp[] = "tcp and ether dst 7c:7a:91:86:b3:e8";    /* The filter expression */
   char filter_exp[] = "ether dst 7c:7a:91:86:b3:e8";    /* The filter expression */
   //char filter_exp[] = "";
   bpf_u_int32 mask;        /* Our netmask */
   bpf_u_int32 net;        /* Our IP */
   struct pcap_pkthdr header;    /* The header that pcap gives us */

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
    if (argc != 2) {
        printf("format: ./receiverSendReceFlowSeqid host_ip_prefix\n");
        return -1;
    }
    /*get network identify for the host: 10.0.0.0/8=>10*/
    char* ip_prefix_str = argv[1];
    net_identity = atoi(ip_prefix_str);
    printf("ip_prefix_str :%s, net_identity:%u\n", ip_prefix_str, net_identity);

    /*open_fifos*/
    if(open_fifos() != 0) {
        return -1;
    }

    if (setup(ip_prefix_str) < 0) {
        printf("setup failed\n");
        return -1;
    }


    pcap_loop(handle, 10000000, got_packet, NULL);
    tearDown();

    //TODO: should close the FIFO, not sure whether they would be closed automatically after the program exits
    //BTW, don't know how to do it with pcap_loop running.

    return(0);
}
