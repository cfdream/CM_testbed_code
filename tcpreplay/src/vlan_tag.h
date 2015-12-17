#ifndef __VLAN_TAG_H__
#define __VLAN_TAG_H__

#include <stdint.h>
#include "common/dlt_names.h"
#include "defines.h"
#include "defines.h"
#include "../../public_lib/packet.h"

//for normal packet
void tag_packet_as_sampled(u_char* packet_buf, int datalink);
bool check_packet_tagged_as_sampled(u_char* packet_buf, int datalink);
//void tag_packet_as_target_flow_in_normal_packet(u_char* packet_buf, int datalink);
// selected_level: 0(default value), 1, 2, 3
void tag_selected_flow_level_in_normal_packet(u_char* packet_buf, int datalink, int selected_level);
void tag_packet_for_switches(u_char* packet_buf, int datalink, int switchides[], int num_switch);

//for condition packet
void tag_packet_selected_level(struct tcpr_802_1q_hdr* p_ethernet_header_vlan, int selected_level);
void tag_packet_as_unselected(struct tcpr_802_1q_hdr* p_ethernet_header_vlan);
#endif
