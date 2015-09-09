/* 
 * multi-thread hashtable 
 * key: <srcip> flow, or flow_src_t, or kfs-key flow_src
 * Value: float
 * get()
 * set()
 * ht_vf_next(): can only be used by one thread
 * del()
 *
 * */

#ifndef __CM_HASHTABLE_KFLOW_VFLOAT_H__
#define __CM_HASHTABLE_KFLOW_VFLOAT_H__

#define KEY_FLOAT_TYPE float

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "flow.h"
#include "hashtable.h"

struct entry_vf_s {
	flow_s key;
	KEY_FLOAT_TYPE value;
	struct entry_vf_s *next;
};

typedef struct entry_vf_s entry_vf_t;

struct hashtable_vf_s {
	int size;
	struct entry_vf_s *table[HASH_MAP_SIZE];

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_SIZE];

    /* for ht_vf_next() */
    int next_current_bin;
    struct entry_vf_s* next_last_visit_entry;
};

typedef struct hashtable_vf_s hashtable_vf_t;


/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_vf_t *ht_vf_create(void);

void ht_vf_destory( hashtable_vf_t *hashtable );

/* Hash a string for a particular hash table. */
int ht_vf_hash( hashtable_vf_t *hashtable, flow_s *key );

/* Create a key-value pair. */
entry_vf_t *ht_vf_newpair( flow_s *key, KEY_FLOAT_TYPE value );

/**
* @brief Retrieve a key-value pair from a hash table.
*
* @param hashtable
* @param key
*
* @return -1: key not exist in the hashtable, >=0 : value of the key
*/
KEY_FLOAT_TYPE ht_vf_get( hashtable_vf_t *hashtable, flow_s* key );

/* Insert a key-value pair into a hash table. */
void ht_vf_set( hashtable_vf_t *hashtable, flow_s *key, KEY_FLOAT_TYPE value );

/* del a key-value pair from a hash table. */
void ht_vf_del( hashtable_vf_t *hashtable, flow_s *key);

/**
* @brief iterator through the hashmap, the next function can only be used by one thread
*
* @param hashtable
* @param ret_entry iteratored entry will be copied to this pointer
*
* @return 0-ret_entry is the next entry, -1:no more entries
*/
int ht_vf_next(hashtable_vf_t *hashtable, entry_vf_t* ret_entry);

#endif
