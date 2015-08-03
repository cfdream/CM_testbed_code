/* 
 * multi-thread hashtable 
 * key: flow_src_t, or kfs-key flow_src
 * Value: float
 * get()
 * set()
 * ht_kfs_vf_next(): can only be used by one thread
 * del()
 *
 * */

#include "mt_hashtable_kFlowSrc_vFloat.h"

/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_kfs_vf_t *ht_kfs_vf_create(void) {

	hashtable_kfs_vf_t *hashtable = NULL;
	int i;

	/* Allocate the table itself. */
	if( ( hashtable = malloc( sizeof( hashtable_kfs_vf_t ) ) ) == NULL ) {
		return NULL;
	}

	for( i = 0; i < HASH_MAP_SIZE; i++ ) {
		hashtable->table[i] = NULL;
	}

    /* initialize mutexs */
    for (i = 0; i < HASH_MAP_SIZE; ++i) {
        pthread_mutex_init(&hashtable->mutexs[i], NULL);
    }

    /* initilize for ht_kfs_vf_next() */
    hashtable->next_current_bin = -1;
    hashtable->next_last_visit_entry = NULL;

	hashtable->size = HASH_MAP_SIZE;

	return hashtable;	
}

void ht_kfs_vf_destory( hashtable_kfs_vf_t *hashtable ) {
    int i;
    entry_kfs_vf_t* p_node;
    entry_kfs_vf_t* next;

    if (NULL == hashtable) {
        return;
    }
    //free table
    for (i = 0; i < hashtable->size; i++) {
        p_node = hashtable->table[i];
        while (p_node) {
            free(p_node->key);
            next = p_node->next;
            free(p_node);
            p_node = next;
        }
    }

    /* free mutexs */
    for (i = 0; i < HASH_MAP_SIZE; ++i) {
        pthread_mutex_destroy(&hashtable->mutexs[i]);
    }

    free(hashtable);
}

/* Hash a string for a particular hash table. */
int ht_kfs_vf_hash( hashtable_kfs_vf_t *hashtable, flow_src_t *key ) {
	/* generate a 64-bit integer from srcip and dstip */
	unsigned long long int hashval = key->srcip;

	return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_kfs_vf_t *ht_kfs_vf_newpair( flow_src_t *key, KEY_FLOAT_TYPE value ) {
	entry_kfs_vf_t *newpair;

	if( ( newpair = malloc( sizeof( entry_kfs_vf_t ) ) ) == NULL ) {
		return NULL;
	}

    //copy the key and value
    newpair->key = deep_copy_flow(key);
    newpair->value = value;

	newpair->next = NULL;

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
KEY_FLOAT_TYPE ht_kfs_vf_get( hashtable_kfs_vf_t *hashtable, flow_src_t* key ) {
	int bin = 0;
	entry_kfs_vf_t *pair;

    if (NULL == hashtable) {
        return -1;
    }

	bin = ht_kfs_vf_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[ bin ];
	while( pair != NULL && pair->key != NULL && flow_src_compare( key, pair->key ) > 0 ) {
		pair = pair->next;
	}

	/* Did we actually find anything? */
	if( pair == NULL || pair->key == NULL || flow_src_compare( key, pair->key ) != 0 ) {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin]);
		return -1;

	} else {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin]);
		return pair->value;
	}
}

/* Insert a key-value pair into a hash table. */
void ht_kfs_vf_set( hashtable_kfs_vf_t *hashtable, flow_src_t *key, KEY_FLOAT_TYPE value ) {
	int bin = 0;
	entry_kfs_vf_t *newpair = NULL;
	entry_kfs_vf_t *next = NULL;
	entry_kfs_vf_t *last = NULL;

    if (NULL == hashtable) {
        return;
    }

	bin = ht_kfs_vf_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	next = hashtable->table[ bin ];

	while( next != NULL && next->key != NULL && flow_src_compare( key, next->key ) > 0 ) {
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's replace that string. */
	if( next != NULL && next->key != NULL && flow_src_compare( key, next->key ) == 0 ) {
		next->value = value;

	/* Nope, could't find it.  Time to grow a pair. */
	} else {
		newpair = ht_kfs_vf_newpair( key, value );

		/* We're at the start of the linked list in this bin. */
		if( next == hashtable->table[ bin ] ) {
			newpair->next = next;
			hashtable->table[ bin ] = newpair;
	
		/* We're at the end of the linked list in this bin. */
		} else if ( next == NULL ) {
			last->next = newpair;
	
		/* We're in the middle of the list. */
		} else  {
			newpair->next = next;
			last->next = newpair;
		}
	}
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin]);
}

/* del a key-value pair from a hash table. */
void ht_kfs_vf_del( hashtable_kfs_vf_t *hashtable, flow_src_t *key) {
	int bin = 0;
	entry_kfs_vf_t *next = NULL;
	entry_kfs_vf_t *last = NULL;

    if (NULL == hashtable) {
        return;
    }

	bin = ht_kfs_vf_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	next = hashtable->table[ bin ];

	while( next != NULL && next->key != NULL && flow_src_compare( key, next->key ) > 0 ) {
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's del that entry. */
	if( next != NULL && next->key != NULL && flow_src_compare( key, next->key ) == 0 ) {
		if (next == hashtable->table[bin]) {
            hashtable->table[bin] = next->next;
        } else {
            last->next = next->next;
        }
        free(next->key);
        free(next);
	}

    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin]);
}

/**
* @brief iterator through the hashmap, the next function can only be used by one thread
*
* @param hashtable
* @param ret_entry iteratored entry will be copied to this pointer
*
* @return 0-ret_entry is the next entry, -1:no more entries
*/
int ht_kfs_vf_next(hashtable_kfs_vf_t *hashtable, entry_kfs_vf_t* ret_entry) {
    if (hashtable->next_current_bin >= hashtable->size) {
        return -1;
    }
    
    //assert(ret_entry != NULL);
    entry_kfs_vf_t* iterator = NULL;
    if (hashtable->next_current_bin >= 0 && hashtable->next_current_bin < hashtable->size) {
        pthread_mutex_lock(&hashtable->mutexs[hashtable->next_current_bin]);
        if (hashtable->next_last_visit_entry != NULL && hashtable->next_last_visit_entry->next != NULL) {
            iterator = hashtable->next_last_visit_entry->next; 
            ret_entry->key = deep_copy_flow(iterator->key);
            ret_entry->value = iterator->value;
            hashtable->next_last_visit_entry = iterator; 
            pthread_mutex_unlock(&hashtable->mutexs[hashtable->next_current_bin]);
            return 0;
        }
        pthread_mutex_unlock(&hashtable->mutexs[hashtable->next_current_bin]);
    }

    //iterator to the bin not empty
    while (1) {
        //go to next bin
        hashtable->next_current_bin += 1;
        if (hashtable->next_current_bin >= hashtable->size) {
            //reset for next next() iteration
            hashtable->next_current_bin = -1;
            hashtable->next_last_visit_entry = NULL;
            return -1;
        }
        
        //check the bin
        pthread_mutex_lock(&hashtable->mutexs[hashtable->next_current_bin]);
        iterator = hashtable->table[hashtable->next_current_bin];
        if (iterator != NULL) {
            ret_entry->key = deep_copy_flow(iterator->key);
            ret_entry->value = iterator->value;
            hashtable->next_last_visit_entry = iterator;
            pthread_mutex_unlock(&hashtable->mutexs[hashtable->next_current_bin]);
            break;
        }
        pthread_mutex_unlock(&hashtable->mutexs[hashtable->next_current_bin]);
    }
    return 0;
}
