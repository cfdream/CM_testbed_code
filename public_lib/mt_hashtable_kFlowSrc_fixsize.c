/* 
 * multi-thread hashtable 
 * key: flow_src_t, or kfs-key flow_src
 * Value: uint, one entry per bin
 * get()
 * set()
 * ht_kfs_fixSize_next(): can only be used by one thread
 * del()
 *
 * */

#include "debug_output.h"
#include "hashtable.h"
#include "mt_hashtable_kFlowSrc_fixsize.h"

extern cm_experiment_setting_t cm_experiment_setting;

/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_kfs_fixSize_t *ht_kfs_fixSize_create(int size) {

    hashtable_kfs_fixSize_t *hashtable = NULL;
    int i;

    /* Allocate the table itself. */
    if( ( hashtable = malloc( sizeof( hashtable_kfs_fixSize_t ) ) ) == NULL ) {
        return NULL;
    }
    hashtable->size = size;
    hashtable->collision_times = 0;
    hashtable->table = (entry_kfs_fixSize_t **)malloc(sizeof(entry_kfs_fixSize_t*) * hashtable->size);

    for( i = 0; i < hashtable->size; i++ ) {
        hashtable->table[i] = NULL;
    }

    /* initialize mutexs */
    for (i = 0; i < HASH_MAP_MUTEX_SIZE; ++i) {
        pthread_mutex_init(&hashtable->mutexs[i], NULL);
    }

    /* initilized for ht_kfs_fixSize_next() */
    hashtable->next_current_bin = -1;

    return hashtable;    
}

void ht_kfs_fixSize_destory( hashtable_kfs_fixSize_t *hashtable ) {
    int i;
    entry_kfs_fixSize_t* p_node;

    if (NULL == hashtable) {
        return;
    }
    //free table
    for (i = 0; i < hashtable->size; i++) {
        p_node = hashtable->table[i];
        if (p_node) {
            free(p_node);
        }
    }
    free(hashtable->table);

    /* free mutexs */
    for (i = 0; i < HASH_MAP_MUTEX_SIZE; ++i) {
        pthread_mutex_destroy(&hashtable->mutexs[i]);
    }

    free(hashtable);
}

//Different from ht_kfs_destory()
//This will just clear the data in the hashtable, will not destory the hashmap
void ht_kfs_fixSize_refresh( hashtable_kfs_fixSize_t *hashtable ) {
    int i;
    entry_kfs_fixSize_t* p_node;

    if (NULL == hashtable) {
        DEBUG("hashtable == NULL");
        return;
    }
    //free table
    for (i = 0; i < hashtable->size; i++) {
        // request lock
        pthread_mutex_lock(&hashtable->mutexs[i%HASH_MAP_MUTEX_SIZE]);

        //delete all nodes in this bin
        p_node = hashtable->table[i];
        if (p_node) {
            free(p_node);
        }
        //set the bin to empty
        hashtable->table[i] = NULL; 
        //release lock
        pthread_mutex_unlock(&hashtable->mutexs[i%HASH_MAP_MUTEX_SIZE]);
    }

    hashtable->collision_times = 0;
    /* refresh for ht_kfs_fixSize_next() */
    hashtable->next_current_bin = -1;
}

/* Create a key-value pair. */
entry_kfs_fixSize_t *ht_kfs_fixSize_newpair( flow_src_t *key, KEY_INT_TYPE value, int selected_level) {
    entry_kfs_fixSize_t *newpair;

    if( ( newpair = malloc( sizeof( entry_kfs_fixSize_t ) ) ) == NULL ) {
        return NULL;
    }

    //copy the key and value
    newpair->key = *key;
    newpair->value = value;
    newpair->selected_level = selected_level;

    return newpair;
}

/**
* @brief Retrieve a key-value pair from a hash table.
*
* @param hashtable
* @param key
*
* @return -1: key not exist in the hashtable, ==0 : value of the key
*/
int ht_kfs_fixSize_get( hashtable_kfs_fixSize_t *hashtable, flow_src_t* key, entry_kfs_fixSize_t* ret_entry) {
    int bin = 0;
    entry_kfs_fixSize_t *pair;

    if (NULL == hashtable) {
        return -1;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    /* Step through the bin, looking for our value. */
    pair = hashtable->table[ bin ];
    /* Did we actually find anything? */
    if( pair == NULL || flow_src_compare( key, &pair->key ) != 0 ) {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
        return -1;
    } else {
        /* release mutex */
        KEY_INT_TYPE value = pair->value;
        ret_entry->value = pair->value;
        ret_entry->selected_level = pair->selected_level;
        pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
        return 0;
    }
}

bool ht_kfs_fixSize_is_sampled(hashtable_kfs_fixSize_t *hashtable, flow_src_t* key) {
    //In ovs, the is_sampled info is stored in flow_sample_map as well. When a flow is just marked as target by the senders, it has not been sampled yet.
    //Thus, ht_kfs_fixSize_get == 0 cannot be treated as the flow already sampled
    //This will influence sample and hold at switch. If condition flows are treated as sampled, sample and hold at switch will get much better performance, which is wrong.
    entry_kfs_fixSize_t ret_entry;
    if (ht_kfs_fixSize_get(hashtable, key, &ret_entry) == 0
        && ret_entry.value > 0) {
        return true;
    }
    return false;
}

/* Insert a key-value pair into a hash table. */
void ht_kfs_fixSize_add_value(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, KEY_INT_TYPE delta_value) {
    int bin = 0;
    entry_kfs_fixSize_t *newpair = NULL;
    entry_kfs_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    if( next != NULL ) {
        /* There's already a pair. */
        if (flow_src_compare( key, &next->key ) == 0 ) {
            //the flow exist
            next->value += delta_value;
        } else {
            //nothing needs to be done in this case
            ++hashtable->collision_times;
        }
    /* The bin is empty */
    } else {
        //a new sampled packet, set the selected_level as 0
        newpair = ht_kfs_fixSize_newpair( key, delta_value, 0);
        hashtable->table[ bin ] = newpair;
    }
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

void ht_kfs_fixSize_add_value_and_update_selected_level_info(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, KEY_INT_TYPE delta_value, int selected_level) {
    int bin = 0;
    entry_kfs_fixSize_t *newpair = NULL;
    entry_kfs_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    if( next != NULL ) {
        /* There's already a pair. */
        if (flow_src_compare( key, &next->key ) == 0 ) {
            //the flow exist
            next->value += delta_value;
            next->selected_level = selected_level;
        } else {
            ++hashtable->collision_times;
            //another flow already exist
            //conflict happens
            if (cm_experiment_setting.replacement) {
                // replacement
                if (next->selected_level >= selected_level) {
                    // the existing flow's selected_level >= incoming flow's selected_level
                    /* keep the existing flow */
                } else {
                    // the existing flow's selected_level < incoming flow's selected_level
                    /* discard the existing flow, keep the incoming flow */
                    //1. free the existing pair
                    free(next);
                    //2. set the new pair
                    //a new sampled packet, set selected_level
                    newpair = ht_kfs_fixSize_newpair( key, delta_value, selected_level);
                    hashtable->table[ bin ] = newpair;
                }
            } else {
                //otherwise, keep existing flow
            }
        }
    /* The bin is empty */
    } else {
        //a new sampled packet, set selected_level
        newpair = ht_kfs_fixSize_newpair( key, delta_value, selected_level);
        hashtable->table[ bin ] = newpair;
    }
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

/* Insert a key-value pair into a hash table. */
void ht_kfs_fixSize_set_selected_level(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, int selected_level) {
    int bin = 0;
    entry_kfs_fixSize_t *newpair = NULL;
    entry_kfs_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    if( next != NULL ) {
        /* There's already a pair. */
        if (flow_src_compare( key, &next->key ) == 0 ) {
            //the flow exist
            next->selected_level = selected_level;
        } else {
            if (selected_level == 0) {
                //this only happens when the previous + delta information (the flow is a target flow) did not arrive, or collisoin happens
                /* release mutex */
                pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
                return;
            }
            //another flow already exists, conflict happens
            ++hashtable->collision_times;
            if (cm_experiment_setting.replacement) {
                // replacement 
                if (next->selected_level > selected_level ||
                   (next->selected_level == selected_level && next->value > 0)
                ) {
                    // the existing flow has higher selected_level or same selected_level but value > 0
                    /* keep the existing flow */
                } else {
                    //higher or same selected_level, keep incoming signal; otherwise, keep existing signal
                    if (selected_level >= next->selected_level) {
                        //replace with the new condition information
                        //1. free the existing pair
                        free(next);
                        //2. set the new pair
                        //a new condition packet, value is set to 0
                        newpair = ht_kfs_fixSize_newpair( key, 0, selected_level);
                        hashtable->table[ bin ] = newpair;
                    }
                }
            } else {
                //not replacement mode, ignore the condition signal
            }
        }
    /* The bin is empty */
    } else {
        //replacement && a new condition packet, value is set to 0
        //not replacement mode, ignore the condition signal
        if (cm_experiment_setting.replacement && selected_level > 0) {
            newpair = ht_kfs_fixSize_newpair( key, 0, selected_level);
            hashtable->table[ bin ] = newpair;
        } else {
            //this only happens when the previous + delta information (the flow is a target flow) did not arrive 
            //(not coz of collisoin, as no existing flow in the bucket)
        }
    }
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

/* del a key-value pair from a hash table. */
void ht_kfs_fixSize_del( hashtable_kfs_fixSize_t *hashtable, flow_src_t *key) {
    int bin = 0;
    entry_kfs_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    /* There's already a pair.  Let's del that entry. */
    if( next != NULL && flow_src_compare( key, &next->key ) == 0 ) {
        free(next);
        hashtable->table[ bin ] = NULL;
    }

    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

int ht_kfs_fixSize_next(hashtable_kfs_fixSize_t *hashtable, entry_kfs_fixSize_t* ret_entry) {
    if (NULL == hashtable) {
        DEBUG("ht_kfs_next(), hashtable==NULL");
        return -1;
    }
    if (hashtable->next_current_bin >= hashtable->size) {
        return -1;
    }
    
    //iterator to the bin not empty
    while (1) {
        //go to next bin
        hashtable->next_current_bin += 1;
        if (hashtable->next_current_bin >= hashtable->size) {
            //reset for next next() iteration
            hashtable->next_current_bin = -1;
            return -1;
        }
        
        //check the bin
        pthread_mutex_lock(&hashtable->mutexs[(hashtable->next_current_bin)%HASH_MAP_MUTEX_SIZE]);
        entry_kfs_fixSize_t* iterator = hashtable->table[hashtable->next_current_bin];
        if (iterator != NULL) {
            ret_entry->key = iterator->key;
            ret_entry->value = iterator->value;
            pthread_mutex_unlock(&hashtable->mutexs[(hashtable->next_current_bin)%HASH_MAP_MUTEX_SIZE]);
            break;
        }
        pthread_mutex_unlock(&hashtable->mutexs[(hashtable->next_current_bin)%HASH_MAP_MUTEX_SIZE]);
    }
    return 0;
}

/*
// Insert a key-value pair into a hash table.
void ht_kfs_fixSize_set(hashtable_kfs_fixSize_t *hashtable, flow_src_t *key, entry_kfs_fixSize_t* ret_entry) {
    int bin = 0;
    entry_kfs_fixSize_t *newpair = NULL;
    entry_kfs_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = flow_src_hash_bin(key, hashtable->size);

    // request mutex
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    if( next != NULL ) {
        // There's already a pair.
        if (flow_src_compare( key, &next->key ) == 0 ) {
            //the flow exist
            next->value = ret_entry->value;
            next->is_target_flow = ret_entry->is_target_flow;
        } else {
            ++hashtable->collision_times;
            //another flow already exist
            //conflict happens
            if (cm_experiment_setting.replacement
                && next->is_target_flow) {
                //replay && the existing flow is_target_flow, 
                // keep the existing flow
            } else {
                //if the new flow is a target flow, replace; otherwise, keep existing flow
                if (ret_entry->is_target_flow) {
                    //replace with the new flow
                    //1. free the existing pair
                    free(next);
                    //2. set the new pair
                    newpair = ht_kfs_fixSize_newpair( key, ret_entry->value, ret_entry->is_target_flow);
                    hashtable->table[ bin ] = newpair;
                }
            }
        }
    // The bin is empty
    } else {
        newpair = ht_kfs_fixSize_newpair( key, ret_entry->value, ret_entry->is_target_flow);
        hashtable->table[ bin ] = newpair;
    }
    // release mutex
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

hashtable_kfs_fixSize_t* ht_kfs_fixSize_copy(hashtable_kfs_fixSize_t *source_hashtable) {
    hashtable_kfs_fixSize_t* ret_hashtable = ht_kfs_fixSize_create(source_hashtable->size);
    if (ret_hashtable == NULL) {
        return NULL;
    }


    entry_kfs_fixSize_t ret_entry;
    while(ht_kfs_fixSize_next(source_hashtable, &ret_entry) != -1) {
        ht_kfs_fixSize_set(ret_hashtable, &ret_entry.key, &ret_entry);
    }
    return ret_hashtable;
}

bool is_target_flow(hashtable_kfs_fixSize_t* target_flow_map, flow_src_t* key) {
    if (target_flow_map == NULL) {
        return false;
    }
    if (ht_kfs_fixSize_get(target_flow_map, key) < 0) {
        return false;
    } else {
        return true;
    }
}
*/
