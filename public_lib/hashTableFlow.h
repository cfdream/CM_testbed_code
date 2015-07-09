/*single thread*/

#ifndef __HASHMAP_FLOW_H__
#define __HASHMAP_FLOW_H__

#define _XOPEN_SOURCE 500 /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */
#define HASH_MAP_SIZE 655350

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "flow.h"

struct entry_kf_s {
	flow_s *key;
	u_int value;  //seqid
	struct entry_kf_s *next;
};

typedef struct entry_kf_s entry_kf_t;

struct hashtable_kf_s {
	int size;
	struct entry_kf_s **table;	
};

typedef struct hashtable_kf_s hashtable_kf_t;


/* Create a new hashtable. */
hashtable_kf_t *ht_kf_create( int size );

void ht_kf_destory(hashtable_kf_t *hashtable, int size);

/* Hash a string for a particular hash table. */
int ht_kf_hash( hashtable_kf_t *hashtable, flow_s *key );

/* Create a key-value pair. */
entry_kf_t *ht_kf_newpair( flow_s *key, u_int value );

/* Insert a key-value pair into a hash table. */
void ht_kf_set( hashtable_kf_t *hashtable, flow_s *key, u_int value );

/* Retrieve a key-value pair from a hash table. */
int ht_kf_get( hashtable_kf_t *hashtable, flow_s* key );

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
#endif
