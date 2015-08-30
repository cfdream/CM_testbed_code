#ifndef __SEND_CONDITIOND_THREAD_H__
#define __SEND_CONDITIOND_THREAD_H__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "tcpr.h"
#include "../../public_lib/mt_hashtable_kFlowSrc_vInt.h"
#include "../../public_lib/condition.h"
#include "data_warehouse.h"

extern tcpreplay_t* g_tcpreplay_ctx;

int send_udp_condition_pkt(condition_t* p_condition, bool is_target_flow);

void* send_condition_to_network(void* param_ptr);

#endif
