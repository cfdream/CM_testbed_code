/* 
 * multi-thread hashtable 
 * key: flow
 * Value: uint
 * get()
 * set()
 * ht_next(): can only be used by one thread
 * del()
 *
 * */

#ifndef __CM_HASHTABLE_KFLOW_VINT_H__
#define __CM_HASHTABLE_KFLOW_VINT_H__

#define KEY_TYPE uint32_t

#define _XOPEN_SOURCE 500 /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */
#define HASH_MAP_SIZE 65535

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "../flow.h"

struct entry_s {
	flow_s *key;
	KEY_TYPE value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
	int size;
	struct entry_s *table[HASH_MAP_SIZE];

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_SIZE];

    /* for ht_next() */
    int next_current_bin;
    struct entry_s* next_last_visit_entry;
};

typedef struct hashtable_s hashtable_t;


/* 
* @brief Create a new hashtable.
*
* @param size
*
* @return 
*/
hashtable_t *ht_create() {

	hashtable_t *hashtable = NULL;
	int i;

	/* Allocate the table itself. */
	if( ( hashtable = malloc( sizeof( hashtable_t ) ) ) == NULL ) {
		return NULL;
	}

	for( i = 0; i < HASH_MAP_SIZE; i++ ) {
		hashtable->table[i] = NULL;
	}

    /* initialize mutexs */
    for (i = 0; i < HASH_MAP_SIZE; ++i) {
        pthread_mutex_init(&hashtable->mutexs[i], NULL);
    }

    /* initilize for ht_next() */
    hashtable->next_current_bin = -1;
    hashtable->next_last_visit_entry = NULL;

	hashtable->size = HASH_MAP_SIZE;

	return hashtable;	
}

void ht_destory( hashtable_t *hashtable ) {
    int i;
    entry_t* p_node;
    entry_t* next;

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
int ht_hash( hashtable_t *hashtable, flow_s *key ) {
	/* generate a 64-bit integer from srcip and dstip */
	unsigned long long int hashval = key->srcip;
    hashval = ((hashval << 32) | key->dstip) ^ key->src_port ^ key->dst_port;

	return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_t *ht_newpair( flow_s *key, KEY_TYPE value ) {
	entry_t *newpair;

	if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
		return NULL;
	}

    //copy the key and value
    newpair->key = deep_copy_flow(key);
    newpair->value = value;

	newpair->next = NULL;

	return newpair;
}

/* Retrieve a key-value pair from a hash table. */
int ht_get( hashtable_t *hashtable, flow_s* key ) {
	int bin = 0;
	entry_t *pair;

    if (NULL == hashtable) {
        return -1;
    }

	bin = ht_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[ bin ];
	while( pair != NULL && pair->key != NULL && flow_compare( key, pair->key ) > 0 ) {
		pair = pair->next;
	}

	/* Did we actually find anything? */
	if( pair == NULL || pair->key == NULL || flow_compare( key, pair->key ) != 0 ) {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin]);
		return -1;

	} else {
        /* release mutex */
        pthread_mutex_unlock(&hashtable->mutexs[bin]);
		return pair->value;
	}
    
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin]);
}

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, flow_s *key, KEY_TYPE value ) {
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;

    if (NULL == hashtable) {
        return;
    }

	bin = ht_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	next = hashtable->table[ bin ];

	while( next != NULL && next->key != NULL && flow_compare( key, next->key ) > 0 ) {
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's replace that string. */
	if( next != NULL && next->key != NULL && flow_compare( key, next->key ) == 0 ) {
		next->value = value;

	/* Nope, could't find it.  Time to grow a pair. */
	} else {
		newpair = ht_newpair( key, value );

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
void ht_del( hashtable_t *hashtable, flow_s *key) {
	int bin = 0;
	entry_t *newpair = NULL;
	entry_t *next = NULL;
	entry_t *last = NULL;

    if (NULL == hashtable) {
        return;
    }

	bin = ht_hash( hashtable, key );

    /* request mutex */
    pthread_mutex_lock(&hashtable->mutexs[bin]);

	next = hashtable->table[ bin ];

	while( next != NULL && next->key != NULL && flow_compare( key, next->key ) > 0 ) {
		last = next;
		next = next->next;
	}

	/* There's already a pair.  Let's del that entry. */
	if( next != NULL && next->key != NULL && flow_compare( key, next->key ) == 0 ) {
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
int ht_next(hashtable_t *hashtable, entry_t* ret_entry) {
    if (hashtable->next_current_bin >= hashtable->size) {
        return -1;
    }
    
    //assert(ret_entry != NULL);
    entry_t* iterator = NULL;
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

#endif
