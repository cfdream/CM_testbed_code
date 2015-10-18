/* 
 * multi-thread hashtable 
 * key: <srcip> flow, or flow_src_t, or kfs-key flow_src
 * Value: uint
 * get()
 * set()
 * ht_kfs_fixSize_next(): can only be used by one thread
 * del()
 *
 * */

#ifndef __CM_HASHTABLE_KFLOWSRC_VINT_FIXSIZE_H__
#define __CM_HASHTABLE_KFLOWSRC_VINT_FIXSIZE_H__

#define KEY_INT_TYPE uint32_t

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "flow.h"
#include "cm_experiment_setting.h"

struct entry_kfs_fixSize_s {
	flow_src_t key;
    bool is_target_flow;
	KEY_INT_TYPE value;
};

typedef struct entry_kfs_fixSize_s entry_kfs_fixSize_t;

struct hashtable_kfs_fixSize_s {
	int size;
    int collision_times;
	entry_kfs_fixSize_t **table;

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_MUTEX_SIZE];

    //for next()
    int next_current_bin;
};

typedef struct hashtable_kfs_fixSize_s hashtable_kfs_fixSize_t;


/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_kfs_fixSize_t *ht_kfs_fixSize_create(int size);

void ht_kfs_fixSize_destory( hashtable_kfs_fixSize_t *hashtable );

void ht_kfs_fixSize_refresh( hashtable_kfs_fixSize_t *hashtable );

/* Hash a string for a particular hash table. */
int ht_kfs_fixSize_hash( hashtable_kfs_fixSize_t *hashtable, flow_src_t *key );

/* Create a key-value pair. */
entry_kfs_fixSize_t *ht_kfs_fixSize_newpair( flow_src_t *key, KEY_INT_TYPE value, bool is_target_flow );

/**
* @brief Retrieve a key-value pair from a hash table.
*
* @param hashtable
* @param key
*
* @return -1: key not exist in the hashtable, >=0 : value of the key
*/
int ht_kfs_fixSize_get( hashtable_kfs_fixSize_t *hashtable, flow_src_t* key, entry_kfs_fixSize_t* ret_entry);

/**
* @brief get whether the flow is sampled or not
*
* @param hashtable
* @param key
*
* @return 
*/
bool ht_kfs_fixSize_is_sampled(hashtable_kfs_fixSize_t *hashtable, flow_src_t* key);

/* Insert a key-value pair into a hash table. */
//replaced by the followed two functions
void ht_kfs_fixSize_set(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, entry_kfs_fixSize_t* ret_entry);

void ht_kfs_fixSize_add_value(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, KEY_INT_TYPE delta_value);

void ht_kfs_fixSize_add_value_and_update_target_flow_info(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, KEY_INT_TYPE delta_value, bool is_target_flow);

void ht_kfs_fixSize_set_target_flow(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, bool is_target_flow);

/* del a key-value pair from a hash table. */
void ht_kfs_fixSize_del( hashtable_kfs_fixSize_t *hashtable, flow_src_t *key);

/* iterate through the hashmap, the next() function can only be used by one thread 
 *
 * @return 0-ret_entry is the next entry, -1: no more entries
 */
int ht_kfs_fixSize_next(hashtable_kfs_fixSize_t *hashtable, entry_kfs_fixSize_t* entry);

hashtable_kfs_fixSize_t* ht_kfs_fixSize_copy(hashtable_kfs_fixSize_t *source_hashtable);

//bool is_target_flow(hashtable_kfs_fixSize_t* target_flow_map, flow_src_t* key);

#endif
