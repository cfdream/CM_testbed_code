#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <stdint.h>

typedef struct condition_s {
    uint32_t srcip;
    uint32_t lost_seqid;
}condition_t;

#endif
