#include "vlan_tag.h"

void tag_packet_as_sampled(u_char* packet_buf, int datalink) {
    int l2_len = 0;
    uint16_t ether_type;
    vlan_hdr_t* vlan_hdr;

    if (datalink == DLT_JUNIPER_ETHER) {
        if ((packet_buf[3] & 0x80) == 0x80) {
            l2_len = ntohs(*((uint16_t*)&packet_buf[4]));
            l2_len += 6;
        } else
            l2_len = 4; /* no header extensions */
    }

    ether_type = ntohs(((eth_hdr_t*)(packet_buf + l2_len))->ether_type);
    if (ether_type == ETHERTYPE_VLAN) {
        vlan_hdr = (vlan_hdr_t *)(packet_buf + l2_len);
        vlan_hdr->vlan_priority_c_vid = TAG_VLAN_VAL;
    }
}

bool check_packet_tagged_as_sampled(u_char* packet_buf, int datalink) {
    int l2_len = 0;
    uint16_t ether_type;
    vlan_hdr_t* vlan_hdr;

    if (datalink == DLT_JUNIPER_ETHER) {
        if ((packet_buf[3] & 0x80) == 0x80) {
            l2_len = ntohs(*((uint16_t*)&packet_buf[4]));
            l2_len += 6;
        } else
            l2_len = 4; /* no header extensions */
    }

    ether_type = ntohs(((eth_hdr_t*)(packet_buf + l2_len))->ether_type);
    if (ether_type == ETHERTYPE_VLAN) {
        vlan_hdr = (vlan_hdr_t *)(packet_buf + l2_len);
        return vlan_hdr->vlan_priority_c_vid & TAG_VLAN_VAL;
    }
    return 0;
}
