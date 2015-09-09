/*
 * multi-thread hashtable. set by one thread, get by another thread. only two threads operate on the hashmap:w
 * key: 5-tuple flow, or flow_t
 * Value: linkedlist, sent pkt infor for the flow
 *
 * Each enntry records the one flow's packets (<seqid, volume of the packet>) list
 * ht_vl_set(flow, seqid, volume): add one packet for the 
 * ht_vl_get_rece_lost_volume(flow, seqid): get the <received volume, lost volume> of one flow. 
 */

#ifndef __MT_HASHTABLE_KFLOW_VLINKLIST_H__
#define __MT_HASHTABLE_KFLOW_VLINKLIST_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "flow.h"
#include "hashtable.h"

typedef struct receV_lostV_s {
    uint32_t received_volume;
    uint32_t lost_volume;
    uint32_t lost_pkt_num;
} receV_lostV_t;

typedef struct pkt_volume_s {
    uint32_t seqid; //seqid of the packet for the flow
    uint32_t volume;   //volume of the packet for the flow
    struct pkt_volume_s* next;
}pkt_volume_t;

struct entry_vl_s {
	flow_s key;
	pkt_volume_t *oldest_pkt;
	pkt_volume_t *newest_pkt;
	struct entry_vl_s *next;
};

typedef struct entry_vl_s entry_vl_t;

struct hashtable_vl_s {
	int size;
	struct entry_vl_s *table[HASH_MAP_SIZE];

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_SIZE];
};

typedef struct hashtable_vl_s hashtable_vl_t;

pkt_volume_t* new_pkt_volume(uint32_t seqid, uint32_t volume);

/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_vl_t *ht_vl_create(void);

void ht_vl_destory( hashtable_vl_t *hashtable );

/* Hash a string for a particular hash table. */
int ht_vl_hash( hashtable_vl_t *hashtable, flow_s *key );

/* Create a key-value pair. */
entry_vl_t *ht_vl_newpair( flow_s *key, pkt_volume_t* pkt_volume);

/**
* @brief packets between [oldest_pkt.seqid, seqid) for the flow are treated as lost, while seqid is treated as received
*
* @param hashtable
* @param key
* @param seqid received at the sender side
*
* @return receV_lostV
*/
receV_lostV_t ht_vl_get_rece_lost_volume( hashtable_vl_t *hashtable, flow_s* key, uint32_t seqid );

/* Insert a key-value pair into a hash table. */
void ht_vl_set( hashtable_vl_t *hashtable, flow_s *key, uint32_t seqid, uint32_t volume);

#endif
