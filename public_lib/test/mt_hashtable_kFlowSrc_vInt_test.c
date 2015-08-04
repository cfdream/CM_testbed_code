#include "../mt_hashtable_kFlowSrc_vInt.h"

void *test_set_func(void* param_ptr) {
    flow_s flow;
	hashtable_kfs_vi_t *hashtable = (hashtable_kfs_vi_t*) param_ptr;
    int i = 0;
    //for (i = 1; i < 10; ++i) {
    while(1) {
        int i = 1;
        for (i = 1; i < 65536*3; i++) {
            flow.srcip = i;
            flow.dstip = i;
            ht_kfs_vi_set(hashtable, &flow, i);
        }
        printf("set suc\n");
    }
}

void *test_del_func(void* param_ptr) {
    flow_s flow;
	hashtable_kfs_vi_t *hashtable = (hashtable_kfs_vi_t*) param_ptr;
    while(1) {
        int i = 1;
        for (i = 1; i < 65536*3; i++) {
            flow.srcip = i;
            flow.dstip = i;
            ht_kfs_vi_del(hashtable, &flow);
        }
        printf("del suc\n");
    }
}

void *test_get_func(void* param_ptr) {
    entry_kfs_vi_t entry;
    flow_s flow;
    flow.srcip = 1;
    flow.dstip = 1;
	hashtable_kfs_vi_t *hashtable = (hashtable_kfs_vi_t*) param_ptr;
    while(1) {
        int value = ht_kfs_vi_get(hashtable, &flow);
        printf("get- flow-%d, value:%d\n", flow.srcip, value);
        sleep(1);
    }
}

void *test_next_func(void* param_ptr) {
    entry_kfs_vi_t entry;
	hashtable_kfs_vi_t *hashtable = (hashtable_kfs_vi_t*) param_ptr;
    while(1) {
        int cnt = 0;
        while (ht_kfs_vi_next(hashtable, &entry) == 0) {
            ++cnt;
        }
        printf("next_cnt:%d\n", cnt);
    }
}

int main( int argc, char **argv ) {

	hashtable_kfs_vi_t *hashtable = ht_kfs_vi_create();

    /*
    pthread_t set_thread1;
    pthread_t set_thread2;
    pthread_t set_thread3;
    pthread_t del_thread1;
    pthread_t del_thread2;
    pthread_t get_thread1;
    pthread_t get_thread2;
    pthread_t next_thread1;

    if (pthread_create(&set_thread1, NULL, test_set_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&set_thread2, NULL, test_set_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&set_thread3, NULL, test_set_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&del_thread1, NULL, test_del_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&del_thread2, NULL, test_del_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&get_thread1, NULL, test_get_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&get_thread2, NULL, test_get_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    if (pthread_create(&next_thread1, NULL, test_next_func, hashtable)) {
        printf("pthread_create fail\n");
        return 1;
    }
    
    pthread_join(set_thread1, NULL);
    pthread_join(next_thread1, NULL);
*/
	//set
    flow_s flow;
    flow.srcip = 1;
    flow.dstip = 1;
    ht_kfs_vi_set(hashtable, &flow, 1);
    printf("srcip:1, v:%d\n", ht_kfs_vi_get(hashtable, &flow));
    flow.srcip = 1 + HASH_MAP_SIZE;
    flow.dstip = 1 + HASH_MAP_SIZE;
    ht_kfs_vi_set(hashtable, &flow, 1 + HASH_MAP_SIZE);
    printf("srcip:1+HASH_MAP_SIZE, v:%d\n", ht_kfs_vi_get(hashtable, &flow));
    flow.srcip = 2;
    flow.dstip = 2;
    ht_kfs_vi_set(hashtable, &flow, 2);
    printf("srcip:2, v:%d\n", ht_kfs_vi_get(hashtable, &flow));
    flow.srcip = 3;
    flow.dstip = 3;
    ht_kfs_vi_set(hashtable, &flow, 3);
    printf("srcip:3, v:%d\n", ht_kfs_vi_get(hashtable, &flow));
    flow.dstip = 4;
    printf("srcip:3, v:%d\n", ht_kfs_vi_get(hashtable, &flow));
    flow.srcip = 3;
    flow.dstip = 4;
    ht_kfs_vi_set(hashtable, &flow, 3);
    printf("srcip:3, v:%d\n", ht_kfs_vi_get(hashtable, &flow));

	//next + del
    entry_kfs_vi_t entry;
	if (ht_kfs_vi_next(hashtable, &entry) == 0) {
		printf("next_srcip:%u, next_v:%d\n", entry.key->srcip, entry.value);
		//del cur, next node
		flow.srcip = 1;
		ht_kfs_vi_del(hashtable, &flow);
		flow.srcip = 1+HASH_MAP_SIZE;
		ht_kfs_vi_del(hashtable, &flow);
		if (ht_kfs_vi_next(hashtable, &entry) == 0) {
			printf("next_srcip:%u, next_v:%d\n", entry.key->srcip, entry.value);
		}
	}

    int i = 1;
    for (i = 1; i < 65536*3; i++) {
        flow.srcip = i;
        flow.dstip = i;
        ht_kfs_vi_set(hashtable, &flow, i);
    }

    int cnt =0;
    while (ht_kfs_vi_next(hashtable, &entry) == 0) {
        if (entry.value == 196607) {
            printf("196607 found\n");
        }
        cnt++;
    }
    printf("1st round next:%d\n", cnt);
    cnt = 0;
    while (ht_kfs_vi_next(hashtable, &entry) == 0) {
        cnt++;
    }
    printf("2st round next:%d\n", cnt);
    
    printf("del flow 1\n");
    flow.srcip = 1;
    flow.dstip = 2;
    ht_kfs_vi_del(hashtable, &flow);

    cnt = 0;
    while (ht_kfs_vi_next(hashtable, &entry) == 0) {
        cnt++;
    }
    printf("round next after del flow 1:%d\n", cnt);

    for (i = 1; i < 65536*3; i++) {
        flow.srcip = i;
        flow.dstip = i+1;
        ht_kfs_vi_del(hashtable, &flow);
    }
    cnt = 0;
    while (ht_kfs_vi_next(hashtable, &entry) == 0) {
        cnt++;
    }
    printf("round next after del all:%d\n", cnt);

    flow.srcip = 1;
    flow.dstip = 2;
    printf("get flow key-%d, value-%d\n", flow.srcip, ht_kfs_vi_get(hashtable, &flow));
    flow.srcip = 65536*3-1;
    flow.dstip = 65536*3+1;
    printf("get flow key-%d, value-%d\n", flow.srcip, ht_kfs_vi_get(hashtable, &flow));
	return 0;
}
