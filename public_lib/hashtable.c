#include "hashtable.h"

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
