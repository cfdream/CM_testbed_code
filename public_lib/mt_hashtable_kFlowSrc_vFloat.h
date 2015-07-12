/* 
 * multi-thread hashtable 
 * key: <srcip> flow, or flow_src_t, or kfs-key flow_src
 * Value: uint
 * get()
 * set()
 * ht_kfs_vf_next(): can only be used by one thread
 * del()
 *
 * */

#ifndef __CM_HASHTABLE_KFLOWSRC_VFLOAT_H__
#define __CM_HASHTABLE_KFLOWSRC_VFLOAT_H__

#define KEY_FLOAT_TYPE float

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "flow.h"
#include "hashtable.h"

struct entry_kfs_vf_s {
	flow_src_t *key;
	KEY_FLOAT_TYPE value;
	struct entry_kfs_vf_s *next;
};

typedef struct entry_kfs_vf_s entry_kfs_vf_t;

struct hashtable_kfs_vf_s {
	int size;
	struct entry_kfs_vf_s *table[HASH_MAP_SIZE];

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_SIZE];

    /* for ht_kfs_vf_next() */
    int next_current_bin;
    struct entry_kfs_vf_s* next_last_visit_entry;
};

typedef struct hashtable_kfs_vf_s hashtable_kfs_vf_t;


/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_kfs_vf_t *ht_kfs_vf_create();

void ht_kfs_vf_destory( hashtable_kfs_vf_t *hashtable );

/* Hash a string for a particular hash table. */
int ht_kfs_vf_hash( hashtable_kfs_vf_t *hashtable, flow_src_t *key );

/* Create a key-value pair. */
entry_kfs_vf_t *ht_kfs_vf_newpair( flow_src_t *key, KEY_FLOAT_TYPE value );

/**
* @brief Retrieve a key-value pair from a hash table.
*
* @param hashtable
* @param key
*
* @return -1: key not exist in the hashtable, >=0 : value of the key
*/
KEY_FLOAT_TYPE ht_kfs_vf_get( hashtable_kfs_vf_t *hashtable, flow_src_t* key );

/* Insert a key-value pair into a hash table. */
void ht_kfs_vf_set( hashtable_kfs_vf_t *hashtable, flow_src_t *key, KEY_FLOAT_TYPE value );

/* del a key-value pair from a hash table. */
void ht_kfs_vf_del( hashtable_kfs_vf_t *hashtable, flow_src_t *key);

/**
* @brief iterator through the hashmap, the next function can only be used by one thread
*
* @param hashtable
* @param ret_entry iteratored entry will be copied to this pointer
*
* @return 0-ret_entry is the next entry, -1:no more entries
*/
int ht_kfs_vf_next(hashtable_kfs_vf_t *hashtable, entry_kfs_vf_t* ret_entry);

#endif
