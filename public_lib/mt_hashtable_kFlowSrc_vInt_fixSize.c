/* 
 * multi-thread hashtable 
 * key: flow_src_t, or kfs-key flow_src
 * Value: uint, one entry per bin
 * get()
 * set()
 * ht_kfs_vi_fixSize_next(): can only be used by one thread
 * del()
 *
 * */

#include "debug_output.h"
#include "hashtable.h"
#include "mt_hashtable_kFlowSrc_vInt_fixSize.h"

extern cm_experiment_setting_t cm_experiment_setting;

/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_kfs_vi_fixSize_t *ht_kfs_vi_fixSize_create(int size) {

    hashtable_kfs_vi_fixSize_t *hashtable = NULL;
    int i;

    /* Allocate the table itself. */
    if( ( hashtable = malloc( sizeof( hashtable_kfs_vi_fixSize_t ) ) ) == NULL ) {
        return NULL;
    }
    hashtable->size = size;
    hashtable->collision_times = 0;
    hashtable->table = (entry_kfs_vi_fixSize_t **)malloc(sizeof(entry_kfs_vi_fixSize_t*) * hashtable->size);

    for( i = 0; i < hashtable->size; i++ ) {
        hashtable->table[i] = NULL;
    }

    /* initialize mutexs */
    for (i = 0; i < HASH_MAP_MUTEX_SIZE; ++i) {
        pthread_mutex_init(&hashtable->mutexs[i], NULL);
    }

    /* initilized for ht_kfs_vi_fixSize_next() */
    hashtable->next_current_bin = -1;

    return hashtable;    
}

void ht_kfs_vi_fixSize_destory( hashtable_kfs_vi_fixSize_t *hashtable ) {
    int i;
    entry_kfs_vi_fixSize_t* p_node;

    if (NULL == hashtable) {
        return;
    }
    //free table
    for (i = 0; i < hashtable->size; i++) {
        p_node = hashtable->table[i];
        if (p_node) {
            free(p_node->key);
            free(p_node);
        }
    }

    /* free mutexs */
    for (i = 0; i < HASH_MAP_MUTEX_SIZE; ++i) {
        pthread_mutex_destroy(&hashtable->mutexs[i]);
    }

    free(hashtable);
}

//Different from ht_kfs_vi_destory()
//This will just clear the data in the hashtable, will not destory the hashmap
void ht_kfs_vi_fixSize_refresh( hashtable_kfs_vi_fixSize_t *hashtable ) {
    int i;
    entry_kfs_vi_fixSize_t* p_node;

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
            free(p_node->key);
            free(p_node);
        }
        //set the bin to empty
        hashtable->table[i] = NULL; 
        //release lock
        pthread_mutex_unlock(&hashtable->mutexs[i%HASH_MAP_MUTEX_SIZE]);
    }

    hashtable->collision_times = 0;
    /* refresh for ht_kfs_vi_fixSize_next() */
    hashtable->next_current_bin = -1;
}

/* Hash a string for a particular hash table. */
int ht_kfs_vi_fixSize_hash( hashtable_kfs_vi_fixSize_t *hashtable, flow_src_t *key ) {
    /* generate a 64-bit integer from srcip and dstip */
    unsigned long long int hashval = key->srcip;

    return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_kfs_vi_fixSize_t *ht_kfs_vi_fixSize_newpair( flow_src_t *key, KEY_INT_TYPE value ) {
    entry_kfs_vi_fixSize_t *newpair;

    if( ( newpair = malloc( sizeof( entry_kfs_vi_fixSize_t ) ) ) == NULL ) {
        return NULL;
    }

    //copy the key and value
    newpair->key = deep_copy_flow(key);
    newpair->value = value;

    return newpair;
}

/**
* @brief Retrieve a key-value pair from a hash table.
*
* @param hashtable
* @param key
*
* @return -1: key not exist in the hashtable, >=0 : value of the key
*/
int ht_kfs_vi_fixSize_get( hashtable_kfs_vi_fixSize_t *hashtable, flow_src_t* key ) {
    int bin = 0;
    entry_kfs_vi_fixSize_t *pair;

    if (NULL == hashtable) {
        return -1;
    }

    bin = ht_kfs_vi_fixSize_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    /* Step through the bin, looking for our value. */
    pair = hashtable->table[ bin ];
    /* Did we actually find anything? */
    if( pair == NULL || pair->key == NULL || flow_src_compare( key, pair->key ) != 0 ) {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
        return -1;
    } else {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
        return pair->value;
    }
}

/* Insert a key-value pair into a hash table. */
void ht_kfs_vi_fixSize_set(hashtable_kfs_vi_fixSize_t *hashtable, hashtable_kfs_vi_fixSize_t* target_flow_map, flow_src_t *key, KEY_INT_TYPE value) {
    int bin = 0;
    entry_kfs_vi_fixSize_t *newpair = NULL;
    entry_kfs_vi_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = ht_kfs_vi_fixSize_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    if( next != NULL && next->key != NULL ) {
        /* There's already a pair. */
        if (flow_src_compare( key, next->key ) == 0 ) {
            //the flow exist
            next->value = value;
        } else {
            ++hashtable->collision_times;
            //another flow already exist
            //conflict happens
            if (cm_experiment_setting.replacement
                && is_target_flow(target_flow_map, next->key)) {
                //replay && the existing flow is_target_flow, 
                /* keep the existing flow */
            } else {
                //replace with the new flow
                //1. free the existing pair
                free(next->key);
                free(next);
                //2. set the new pair
                newpair = ht_kfs_vi_fixSize_newpair( key, value );
                hashtable->table[ bin ] = newpair;
            }
        }
    /* The bin is empty */
    } else {
        newpair = ht_kfs_vi_fixSize_newpair( key, value );
        hashtable->table[ bin ] = newpair;
    }
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

/* del a key-value pair from a hash table. */
void ht_kfs_vi_fixSize_del( hashtable_kfs_vi_fixSize_t *hashtable, flow_src_t *key) {
    int bin = 0;
    entry_kfs_vi_fixSize_t *next = NULL;

    if (NULL == hashtable) {
        return;
    }

    bin = ht_kfs_vi_fixSize_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);

    next = hashtable->table[ bin ];

    /* There's already a pair.  Let's del that entry. */
    if( next != NULL && next->key != NULL && flow_src_compare( key, next->key ) == 0 ) {
        free(next->key);
        free(next);
        hashtable->table[ bin ] = NULL;
    }

    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin%HASH_MAP_MUTEX_SIZE]);
}

int ht_kfs_vi_fixSize_next(hashtable_kfs_vi_fixSize_t *hashtable, entry_kfs_vi_fixSize_t* ret_entry) {
    if (NULL == hashtable) {
        DEBUG("ht_kfs_vi_next(), hashtable==NULL");
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
        entry_kfs_vi_fixSize_t* iterator = hashtable->table[hashtable->next_current_bin];
        if (iterator != NULL) {
            ret_entry->key = deep_copy_flow(iterator->key);
            ret_entry->value = iterator->value;
            pthread_mutex_unlock(&hashtable->mutexs[(hashtable->next_current_bin)%HASH_MAP_MUTEX_SIZE]);
            break;
        }
        pthread_mutex_unlock(&hashtable->mutexs[(hashtable->next_current_bin)%HASH_MAP_MUTEX_SIZE]);
    }
    return 0;
}

hashtable_kfs_vi_fixSize_t* ht_kfs_vi_fixSize_copy(hashtable_kfs_vi_fixSize_t *source_hashtable) {
    hashtable_kfs_vi_fixSize_t* ret_hashtable = ht_kfs_vi_fixSize_create(source_hashtable->size);
    if (ret_hashtable == NULL) {
        return NULL;
    }

    entry_kfs_vi_fixSize_t ret_entry;
    while(ht_kfs_vi_fixSize_next(source_hashtable, &ret_entry) != -1) {
        ht_kfs_vi_fixSize_set(ret_hashtable, NULL, ret_entry.key, ret_entry.value);
        free(ret_entry.key);
    }
    return ret_hashtable;
}

bool is_target_flow(hashtable_kfs_vi_fixSize_t* target_flow_map, flow_src_t* key) {
    if (target_flow_map == NULL) {
        return false;
    }
    if (ht_kfs_vi_fixSize_get(target_flow_map, key) < 0) {
        return false;
    } else {
        return true;
    }
}
