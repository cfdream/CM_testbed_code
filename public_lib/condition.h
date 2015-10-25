#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <stdint.h>

typedef struct condition_s {
    #ifdef FLOW_SRC
    uint32_t srcip;
    #endif
    #ifdef FLOW_SRC_DST
    uint32_t srcip;
    uint32_t dstip;
    #endif
} condition_t;

#endif
