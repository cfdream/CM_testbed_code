#ifndef __HOST_SAMPLOR_H__
#define __HOST_SAMPLOR_H__

#include <assert.h>
#include <stdbool.h>
#include "data_warehouse.h"
#include "sample_model.h"
#include "../../public_lib/flow.h"

#define RAND_MOD_NUMBER 1000000

int flow_src_already_sampled(flow_src_t* p_flow_src);

int sample_packet(packet_t* p_packet);

#endif
