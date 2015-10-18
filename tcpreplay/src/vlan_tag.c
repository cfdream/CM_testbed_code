#include "vlan_tag.h"

void tag_packet_as_sampled(u_char* packet_buf, int datalink) {
    int l2_len = 0;
    uint16_t ether_type;
    struct tcpr_802_1q_hdr* vlan_hdr;

    if (datalink == DLT_JUNIPER_ETHER) {
        if ((packet_buf[3] & 0x80) == 0x80) {
            l2_len = ntohs(*((uint16_t*)&packet_buf[4]));
            l2_len += 6;
        } else
            l2_len = 4; /* no header extensions */
    }

    ether_type = ntohs(((eth_hdr_t*)(packet_buf + l2_len))->ether_type);
    if (ether_type == ETHERTYPE_VLAN) {
        vlan_hdr = (struct tcpr_802_1q_hdr *)(packet_buf + l2_len);
        vlan_hdr->vlan_priority_c_vid |= TAG_VLAN_PACKET_SAMPLED_VAL;
    }
}

bool check_packet_tagged_as_sampled(u_char* packet_buf, int datalink) {
    int l2_len = 0;
    uint16_t ether_type;
    struct tcpr_802_1q_hdr* vlan_hdr;

    if (datalink == DLT_JUNIPER_ETHER) {
        if ((packet_buf[3] & 0x80) == 0x80) {
            l2_len = ntohs(*((uint16_t*)&packet_buf[4]));
            l2_len += 6;
        } else
            l2_len = 4; /* no header extensions */
    }

    ether_type = ntohs(((eth_hdr_t*)(packet_buf + l2_len))->ether_type);
    if (ether_type == ETHERTYPE_VLAN) {
        vlan_hdr = (struct tcpr_802_1q_hdr *)(packet_buf + l2_len);
        return vlan_hdr->vlan_priority_c_vid & TAG_VLAN_PACKET_SAMPLED_VAL;
    }
    return 0;
}


/**
* @brief tag the vlan section of packet_buf to mark it as a target flow
*
* @param packet_buf
* @param datalink
*/
void tag_packet_as_target_flow_in_normal_packet(u_char* packet_buf, int datalink) {
    int l2_len = 0;
    uint16_t ether_type;
    struct tcpr_802_1q_hdr* vlan_hdr;

    if (datalink == DLT_JUNIPER_ETHER) {
        if ((packet_buf[3] & 0x80) == 0x80) {
            l2_len = ntohs(*((uint16_t*)&packet_buf[4]));
            l2_len += 6;
        } else
            l2_len = 4; /* no header extensions */
    }

    ether_type = ntohs(((eth_hdr_t*)(packet_buf + l2_len))->ether_type);
    if (ether_type == ETHERTYPE_VLAN) {
        vlan_hdr = (struct tcpr_802_1q_hdr *)(packet_buf + l2_len);
        vlan_hdr->vlan_priority_c_vid |= TAG_VLAN_TARGET_FLOW_VAL;
    }
}


void tag_packet_as_target_flow(struct tcpr_802_1q_hdr* p_ethernet_header_vlan) {
    p_ethernet_header_vlan->vlan_priority_c_vid = TAG_VLAN_TARGET_FLOW_VAL;
}

void tag_packet_as_not_target_flow(struct tcpr_802_1q_hdr* p_ethernet_header_vlan) {
    p_ethernet_header_vlan->vlan_priority_c_vid = 0;
}
