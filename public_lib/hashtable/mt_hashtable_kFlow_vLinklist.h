/*
 * multi-thread hashtable. set by one thread, get by another thread. only two threads operate on the hashmap:w
 *
 * Each enntry records the one flow's packets (<seqid, volume of the packet>) list
 * ht_set(flow, seqid, volume): add one packet for the 
 * ht_get_rece_lost_volume(flow, seqid): get the <received volume, lost volume> of one flow. 
 */

#ifndef __MT_HASHTABLE_KFLOW_VLINKLIST_H__
#define __MT_HASHTABLE_KFLOW_VLINKLIST_H__

#define HASH_MAP_SIZE 65535

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "../flow.h"

typedef struct get_ans_s {
    uint32_t received_volume;
    uint32_t lost_volume;
} get_ans_t;

typedef struct pkt_volume_s {
    uint32_t seqid; //seqid of the packet for the flow
    uint32_t volume;   //volume of the packet for the flow
    struct pkt_volume_s* next;
}pkt_volume_t;

struct entry_s {
	flow_s *key;
	pkt_volume_t *oldest_pkt;
	pkt_volume_t *newest_pkt;
	struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
	int size;
	struct entry_s *table[HASH_MAP_SIZE];

    /* for multi-thread accessing */
    pthread_mutex_t mutexs[HASH_MAP_SIZE];
};

typedef struct hashtable_s hashtable_t;

pkt_volume_t* new_pkt_volume(uint32_t seqid, uint32_t volume) {
    pkt_volume_t* pkt_volume;
    if ((pkt_volume = malloc(sizeof(pkt_volume_t))) == NULL) {
        return NULL;
    }
    pkt_volume->seqid = seqid;
    pkt_volume->volume = volume;
    pkt_volume->next = NULL;
    return pkt_volume;
}


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

	hashtable->size = HASH_MAP_SIZE;

    printf("SUCC: ht_create\n");

	return hashtable;	
}

void ht_destory( hashtable_t *hashtable ) {
    int i;
    entry_t* p_node;
    entry_t* next;
    pkt_volume_t* p_pkt;
    pkt_volume_t* p_next_pkt;

    if (NULL == hashtable) {
        return;
    }
    //free table
    for (i = 0; i < hashtable->size; i++) {
        p_node = hashtable->table[i];
        while (p_node) {
            free(p_node->key);

            //value: free packet list
            p_pkt = p_node->oldest_pkt;
            while (p_pkt) {
                p_next_pkt = p_pkt->next;
                free(p_pkt);
                p_pkt = p_next_pkt;
            }

            next = p_node->next;
            free(p_node);
            p_node = next;
        }
    }
    /* destroy mutexs */
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
entry_t *ht_newpair( flow_s *key, pkt_volume_t* pkt_volume) {
    assert(pkt_volume != NULL);
	entry_t *newpair;

	if( ( newpair = malloc( sizeof( entry_t ) ) ) == NULL ) {
		return NULL;
	}

    //copy the key and value
    newpair->key = deep_copy_flow(key);
    newpair->oldest_pkt = pkt_volume;
    newpair->newest_pkt = pkt_volume;

	newpair->next = NULL;

	return newpair;
}

/**
* @brief packets between [oldest_pkt.seqid, seqid) for the flow is treated as lost, while seqid is treated as received
*
* @param hashtable
* @param key
* @param seqid
*
* @return 
*/
get_ans_t ht_get_rece_lost_volume( hashtable_t *hashtable, flow_s* key, uint32_t seqid ) {
	int bin = 0;
	entry_t *pair;
    uint32_t lost_volume = 0;
    uint32_t received_volume = 0;
    pkt_volume_t* temp;
    get_ans_t ans;
    memset(&ans, 0, sizeof(pkt_volume_t));

    if (NULL == hashtable) {
        return ans;
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
        printf("FAIL: flow not in hashmap\n");
	} else {
        /* go through the pkt list, to get <received volume, lost volume> */
        pkt_volume_t* iterator = pair->oldest_pkt;
        while (iterator && iterator->seqid < seqid) {
            //packets between [oldest_pkt.seqid, seqid) not received
            ans.lost_volume += iterator->volume;
            temp = iterator;
            iterator = iterator->next;
            free(temp);
        }
        if (iterator && iterator->seqid == seqid) {
            //packet seqid received
            ans.received_volume += iterator->volume;
            //
            temp = iterator;
            iterator = iterator->next;
            free(temp);
        } else {
            printf("FATAL ERROR: packet received not set! debug it.\n");
        }

        //updated newest_pkt, oldest_pkt
        pair->oldest_pkt = iterator;
        if (!iterator) {
            pair->newest_pkt = NULL;
        }
	}
    
    /* release mutex */
    pthread_mutex_unlock(&hashtable->mutexs[bin]);

    return ans;
}

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, flow_s *key, uint32_t seqid, uint32_t volume) {
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

    pkt_volume_t* pkt_volume = new_pkt_volume(seqid, volume);
	/* There's already a pair.  add the packet to the list. */
	if( next != NULL && next->key != NULL && flow_compare( key, next->key ) == 0 ) {
		if (next->oldest_pkt == NULL) {
            next->oldest_pkt = pkt_volume;
            next->newest_pkt = pkt_volume;
        } else {
            //assert(next->newest_pkt != NULL);
            next->newest_pkt->next = pkt_volume;
            next->newest_pkt = pkt_volume;
        }

	/* Nope, could't find it.  Time to grow a pair. */
	} else {
		newpair = ht_newpair( key, pkt_volume);

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

#endif
