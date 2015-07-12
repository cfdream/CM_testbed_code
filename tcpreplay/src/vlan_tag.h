#ifndef __VLAN_TAG_H__
#define __VLAN_TAG_H__

#include <stdint.h>
#include "common/dlt_names.h"
#include "defines.h"
#include "defines.h"
#include "../../public_lib/packet.h"

void tag_packet_as_sampled(u_char* packet_buf, int datalink);

bool check_packet_tagged_as_sampled(u_char* packet_buf, int datalink);

#endif
