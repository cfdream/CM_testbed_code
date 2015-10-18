#ifndef __VLAN_TAG_H__
#define __VLAN_TAG_H__

#include <stdint.h>
#include "common/dlt_names.h"
#include "defines.h"
#include "defines.h"
#include "../../public_lib/packet.h"

void tag_packet_as_sampled(u_char* packet_buf, int datalink);

bool check_packet_tagged_as_sampled(u_char* packet_buf, int datalink);

void tag_packet_as_target_flow_in_normal_packet(u_char* packet_buf, int datalink);

void tag_packet_as_target_flow(struct tcpr_802_1q_hdr* p_ethernet_header_vlan);
void tag_packet_as_not_target_flow(struct tcpr_802_1q_hdr* p_ethernet_header_vlan);
#endif
