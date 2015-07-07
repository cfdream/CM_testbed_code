#include "mt_hashtable_kFlow_vLinklist.h"

#define DEFAULT_PKT_VOLUME 1

void *test_set_func(void* param_ptr) {
    flow_s flow;
    memset(&flow, 0, sizeof(flow));
	hashtable_t *hashtable = (hashtable_t*) param_ptr;
    int i = 0;
    //for (i = 1; i < 10; ++i) {
    int j = 1;
    for (j = 1; j < 1000001*50; j++) {
        int i = 1;
        for (i = 1; i < 2; i++) {
            flow.srcip = i;
            ht_set(hashtable, &flow, j, DEFAULT_PKT_VOLUME); 
        }
    }

    printf("set suc\n");
}

void *test_get_func(void* param_ptr) {
    sleep(5);
	hashtable_t *hashtable = (hashtable_t*) param_ptr;
    flow_s flow;
    memset(&flow, 0, sizeof(flow));
    flow.srcip = 1;
    uint32_t seqid = 10;
    for (; seqid < 1000001*50; seqid+=10) {
        get_ans_t pkt_volume = ht_get_rece_lost_volume(hashtable, &flow, seqid);
        if (! (seqid%1000000)) 
            printf("get- flow-%d, lost_volume:%u, rece_volume:%u\n", flow.srcip, pkt_volume.lost_volume, pkt_volume.received_volume);
    }
    
    printf("oldest_pkt seqid:%u\n", hashtable->table[ht_hash(hashtable, &flow)]->oldest_pkt->seqid);
    printf("newest_pkt seqid:%u\n", hashtable->table[ht_hash(hashtable, &flow)]->newest_pkt->seqid);

    printf("get suc\n");
}

int main( int argc, char **argv ) {

	hashtable_t *hashtable = ht_create( 65536 );

    pthread_t set_thread1;
    pthread_t get_thread1;

    if (pthread_create(&set_thread1, NULL, test_set_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&get_thread1, NULL, test_get_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    
    pthread_join(set_thread1, NULL);
    pthread_join(get_thread1, NULL);
}
