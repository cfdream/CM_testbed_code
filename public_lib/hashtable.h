#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#define _XOPEN_SOURCE 500 /* Enable certain library functions (strdup) on linux.  See feature_test_macros(7) */
#define HASH_MAP_SIZE 65535

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

struct entry_s {
	char *key;
	char *value;
	struct entry_s *next;
};

typedef struct entry_s entry_t;

struct hashtable_s {
	int size;
	struct entry_s **table;	
};

typedef struct hashtable_s hashtable_t;

/* Create a new hashtable. */
hashtable_t *ht_create( int size );

void ht_destory(hashtable_t *hashtable, int size);

/* Hash a string for a particular hash table. */
int ht_hash( hashtable_t *hashtable, char *key );

/* Create a key-value pair. */
entry_t *ht_newpair( char *key, char *value );

/* Insert a key-value pair into a hash table. */
void ht_set( hashtable_t *hashtable, char *key, char *value );

/* Retrieve a key-value pair from a hash table. */
char *ht_get( hashtable_t *hashtable, char *key );

/*
int main( int argc, char **argv ) {

	hashtable_t *hashtable = ht_create( 65536 );

	ht_set( hashtable, "key1", "inky" );
	ht_set( hashtable, "key2", "pinky" );
	ht_set( hashtable, "key3", "blinky" );
	ht_set( hashtable, "key4", "floyd" );

	printf( "%s\n", ht_get( hashtable, "key1" ) );
	printf( "%s\n", ht_get( hashtable, "key2" ) );
	printf( "%s\n", ht_get( hashtable, "key3" ) );
	printf( "%s\n", ht_get( hashtable, "key4" ) );

	return 0;
}
*/
#endif
