/*single thread*/

#include "hashTableFlow.h"

/* Create a new hashtable. */
hashtable_kf_t *ht_kf_create( int size ) {

	hashtable_kf_t *hashtable = NULL;
	int i;

	if( size < 1 ) return NULL;

	/* Allocate the table itself. */
	if( ( hashtable = malloc( sizeof( hashtable_kf_t ) ) ) == NULL ) {
		return NULL;
	}

	/* Allocate pointers to the head nodes. */
	if( ( hashtable->table = malloc( sizeof( entry_kf_t * ) * size ) ) == NULL ) {
		return NULL;
	}
	for( i = 0; i < size; i++ ) {
		hashtable->table[i] = NULL;
	}

	hashtable->size = size;

	return hashtable;	
}

void ht_kf_destory(hashtable_kf_t *hashtable, int size) {
    int i;
    entry_kf_t* p_node;
    entry_kf_t* next;

    if (NULL == hashtable) {
        return;
    }
    for (i = 0; i < size; i++) {
        p_node = hashtable->table[i];
        while (p_node) {
            free(p_node->key);
            next = p_node->next;
            free(p_node);
            p_node = next;
        }
    }
}

/* Hash a string for a particular hash table. */
int ht_kf_hash( hashtable_kf_t *hashtable, flow_s *key ) {
	/* generate a 64-bit integer from srcip and dstip */
	unsigned long long int hashval = key->srcip;
    hashval = ((hashval << 32) | key->dstip) ^ key->src_port ^ key->dst_port;

	return hashval % hashtable->size;
}

/* Create a key-value pair. */
entry_kf_t *ht_kf_newpair( flow_s *key, u_int value ) {
	entry_kf_t *newpair;

	if( ( newpair = malloc( sizeof( entry_kf_t ) ) ) == NULL ) {
		return NULL;
	}

    //copy the key and value
    newpair->key = deep_copy_flow(key);
    newpair->value = value;

	newpair->next = NULL;

	return newpair;
}

/* Insert a key-value pair into a hash table. */
void ht_kf_set( hashtable_kf_t *hashtable, flow_s *key, u_int value ) {
	int bin = 0;
	entry_kf_t *newpair = NULL;
	entry_kf_t *next = NULL;
	entry_kf_t *last = NULL;

    if (NULL == hashtable) {
        return;
    }

	bin = ht_kf_hash( hashtable, key );

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
		newpair = ht_kf_newpair( key, value );

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
}

/* Retrieve a key-value pair from a hash table. */
int ht_kf_get( hashtable_kf_t *hashtable, flow_s* key ) {
	int bin = 0;
	entry_kf_t *pair;

    if (NULL == hashtable) {
        return -1;
    }

	bin = ht_kf_hash( hashtable, key );

	/* Step through the bin, looking for our value. */
	pair = hashtable->table[ bin ];
	while( pair != NULL && pair->key != NULL && flow_compare( key, pair->key ) > 0 ) {
		pair = pair->next;
	}

	/* Did we actually find anything? */
	if( pair == NULL || pair->key == NULL || flow_compare( key, pair->key ) != 0 ) {
		return -1;

	} else {
		return pair->value;
	}
	
}

/*
int main( int argc, char **argv ) {

	hashtable_kf_t *hashtable = ht_kf_create( 65536 );

	ht_kf_set( hashtable, "key1", "inky" );
	ht_kf_set( hashtable, "key2", "pinky" );
	ht_kf_set( hashtable, "key3", "blinky" );
	ht_kf_set( hashtable, "key4", "floyd" );

	printf( "%s\n", ht_kf_get( hashtable, "key1" ) );
	printf( "%s\n", ht_kf_get( hashtable, "key2" ) );
	printf( "%s\n", ht_kf_get( hashtable, "key3" ) );
	printf( "%s\n", ht_kf_get( hashtable, "key4" ) );

	return 0;
}
*/
