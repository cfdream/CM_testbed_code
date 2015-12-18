#include "../public_lib/sample_model.h"
#include "../public_lib/sample_packet.h"
#include "data_warehouse.h"
#include <pthread.h>

struct drand48_data g_rand_buffer;

/**
* @brief init the data warehouse, at the beginning, only the first buffer should be initiliazed
*
* @return 0-succ -1:fail
*/
int data_warehouse_init() {
    if (init_tasks_info(&data_warehouse.task_manager) != 0) {
        return -1;
    }

    pthread_mutex_init(&data_warehouse.packet_send_mutex, NULL);
    pthread_mutex_init(&data_warehouse.data_warehouse_mutex, NULL);

    // create flow_recePktList_map
    data_warehouse.flow_recePktList_map = ht_vl_create();
    if (data_warehouse.flow_recePktList_map == NULL) {
        return -1;
    }

    data_warehouse.last_sent_flow_selected_level_map = ht_kfs_vi_create();
    if (data_warehouse.last_sent_flow_selected_level_map == NULL) {
        return -1;
    }

    int a_idx = 0;
    for (; a_idx < BUFFER_NUM; ++a_idx) {
        data_warehouse.flow_volume_map[a_idx] = ht_kfs_vi_create();
        if (data_warehouse.flow_volume_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.fixed_flow_loss_volume_map[a_idx] = ht_kfs_fixSize_create(cm_experiment_setting.sender_setting.fix_loss_map_bucket_size);
        if (data_warehouse.fixed_flow_loss_volume_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.flow_loss_volume_map[a_idx] = ht_kfs_vi_create();
        if (data_warehouse.flow_loss_volume_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.flow_loss_rate_map[a_idx] = ht_kfs_vf_create();
        if (data_warehouse.flow_loss_rate_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.flow_selected_level_map[a_idx] = ht_kfs_vi_create();
        if (data_warehouse.flow_selected_level_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.flow_sample_map[a_idx] = ht_kfs_vi_create();
        if (data_warehouse.flow_sample_map[a_idx] == NULL) {
            return -1;
        }
        data_warehouse.flow_not_sampled_volume_map[a_idx] = ht_kfs_vi_create();
        if (data_warehouse.flow_not_sampled_volume_map[a_idx] == NULL) {
            return -1;
        }

        data_warehouse.pkt_num_sent[a_idx] = 0;
        data_warehouse.volume_sent[a_idx] = 0;
        data_warehouse.condition_pkt_num_sent[a_idx] = 0;
        data_warehouse.volume_lost[a_idx] = 0;
    }

    data_warehouse.active_idx = 0;

    int i = 0;
    /* initialize mutexs */
    for (i = 0; i < HASH_MAP_SIZE; ++i) {
        pthread_mutex_init(&data_warehouse.mutexs[i], NULL);
    }
    printf("data_warehouse_init succ\n");
    return 0;
}

void data_warehouse_destroy() {
    pthread_mutex_destroy(&data_warehouse.packet_send_mutex);
    pthread_mutex_destroy(&data_warehouse.data_warehouse_mutex);

    // destory flow_recePktList_map
    ht_vl_destory(data_warehouse.flow_recePktList_map);

    //destory last_sent_flow_selected_level_map
    ht_kfs_vi_destory(data_warehouse.last_sent_flow_selected_level_map);

    int a_idx = 0;
    for (; a_idx < BUFFER_NUM; ++a_idx) {
        ht_kfs_vi_destory(data_warehouse.flow_volume_map[a_idx]);
        ht_kfs_fixSize_destory(data_warehouse.fixed_flow_loss_volume_map[a_idx]);
        ht_kfs_vi_destory(data_warehouse.flow_loss_volume_map[a_idx]);
        ht_kfs_vf_destory(data_warehouse.flow_loss_rate_map[a_idx]);
        ht_kfs_vi_destory(data_warehouse.flow_selected_level_map[a_idx]);
        ht_kfs_vi_destory(data_warehouse.flow_sample_map[a_idx]);
        ht_kfs_vi_destory(data_warehouse.flow_not_sampled_volume_map[a_idx]);
    }

    int i = 0;
    /* destory mutexs */
    for (i = 0; i < HASH_MAP_SIZE; ++i) {
        pthread_mutex_destroy(&data_warehouse.mutexs[i]);
    }
}

void update_flow_loss_volume(flow_src_t* p_flow, int added_loss_volume) {
    uint32_t lock_bin = flow_src_hash_bin(p_flow, HASH_MAP_SIZE);
    /* request mutex */
    pthread_mutex_lock(&data_warehouse.mutexs[lock_bin]);

    hashtable_kfs_vi_t* flow_volume_map = data_warehouse_get_flow_volume_map();
    hashtable_kfs_fixSize_t* fixed_flow_loss_volume_map = data_warehouse_get_fixed_flow_loss_volume_map();
    hashtable_kfs_vi_t* flow_loss_volume_map = data_warehouse_get_flow_loss_volume_map();
    hashtable_kfs_vf_t* flow_loss_rate_map = data_warehouse_get_flow_loss_rate_map();
    data_warehouse.volume_lost[data_warehouse.active_idx] += added_loss_volume;

    if (added_loss_volume > 0) {
        //get total volume of flow
        int volume = ht_kfs_vi_get(flow_volume_map, p_flow);
        if (volume < 0) {
            printf("FATAL: flow_volume = 0 while loss_volume > 0 \n");
            pthread_mutex_unlock(&data_warehouse.mutexs[lock_bin]);
            return;
        }

        //update loss volume
        int loss_volume = ht_kfs_vi_get(flow_loss_volume_map, p_flow);
        if (loss_volume < 0) {
            loss_volume = 0;
        }
        loss_volume += added_loss_volume;
        ht_kfs_vi_set(flow_loss_volume_map, p_flow, loss_volume);

        if (cm_experiment_setting.sender_setting.loss_map_mem_type == FIXED) {
            //sample the flow first. If sampled, update the flow loss volume
            if (sample_packet_fixSize_map_sender(p_flow, loss_volume, &g_rand_buffer, fixed_flow_loss_volume_map)) {
                ht_kfs_fixSize_add_value(fixed_flow_loss_volume_map, p_flow, added_loss_volume);
            }
            //get loss_volume from fixed size hashmap; 
            //if not exist, loss_volume = 0
            loss_volume = 0;
            entry_kfs_fixSize_t ret_entry;
            if (ht_kfs_fixSize_get(fixed_flow_loss_volume_map, p_flow, &ret_entry) == 0) {
                loss_volume = ret_entry.value;
            }
        }

        //update loss rate
        float loss_rate = 1.0 * loss_volume / volume;
        ht_kfs_vf_set(flow_loss_rate_map, p_flow, loss_rate);

        //update flow selected level
        update_flow_selected_level_map(p_flow, volume, loss_volume, loss_rate);

        //printf("update_flow_loss_volume, flow[srcip:%u-volume:%d-lossrate:%f-%f-lossVolume:%d]\n", 
        //    p_flow->srcip, volume, loss_rate, ht_kfs_vf_get(flow_loss_rate_map, p_flow), 
        //    loss_volume);
    }
    /* release mutex */
    pthread_mutex_unlock(&data_warehouse.mutexs[lock_bin]);
}

void update_flow_normal_volume(flow_src_t* p_flow, int added_volume) {
    uint32_t lock_bin = flow_src_hash_bin(p_flow, HASH_MAP_SIZE);
    /* request mutex */
    pthread_mutex_lock(&data_warehouse.mutexs[lock_bin]);

    hashtable_kfs_vi_t* flow_volume_map = data_warehouse_get_flow_volume_map();
    hashtable_kfs_vi_t* flow_loss_volume_map = data_warehouse_get_flow_loss_volume_map();
    hashtable_kfs_fixSize_t* fixed_flow_loss_volume_map = data_warehouse_get_fixed_flow_loss_volume_map();
    hashtable_kfs_vf_t* flow_loss_rate_map = data_warehouse_get_flow_loss_rate_map();

    if (added_volume > 0) {
        //update total volume of flow
        int volume = ht_kfs_vi_get(flow_volume_map, p_flow);
        if (volume < 0) {
            volume = 0;
        }
        volume += added_volume;
        ht_kfs_vi_set(flow_volume_map, p_flow, volume);

        //get loss volume
        int loss_volume = ht_kfs_vi_get(flow_loss_volume_map, p_flow);
        if (loss_volume < 0) {
            loss_volume = 0;
        }
        if (cm_experiment_setting.sender_setting.loss_map_mem_type == FIXED) {
            //get loss_volume from fixed size hashmap; 
            //if not exist, loss_volume = 0
            loss_volume = 0;
            entry_kfs_fixSize_t ret_entry;
            if (ht_kfs_fixSize_get(fixed_flow_loss_volume_map, p_flow, &ret_entry) == 0) {
                loss_volume = ret_entry.value;
            }
        }

        //update loss rate
        float loss_rate = 1.0 * loss_volume / volume;
        ht_kfs_vf_set(flow_loss_rate_map, p_flow, loss_rate);

        //update selected level of the flow
        update_flow_selected_level_map(p_flow, volume, loss_volume, loss_rate);

        /*
        if (loss_rate > 0) {
            printf("update_flow_normal_volume, flow[srcip:%u-volume:%d-lossrate:%f-%f-lossVolume:%d]\n", 
                p_flow->srcip, volume, loss_rate, ht_kfs_vf_get(flow_loss_rate_map, p_flow),  loss_volume);
        }
        */
    }

    /* release mutex */
    pthread_mutex_unlock(&data_warehouse.mutexs[lock_bin]);
}

void update_flow_selected_level_map(flow_src_t* p_flow, int volume, int loss_volume, float loss_rate) {
    //update selected level of the flow
    hashtable_kfs_vi_t* flow_selected_level_map = data_warehouse_get_flow_selected_level_map();
    if (volume >= cm_experiment_setting.target_flow_setting.volume_threshold
        && loss_rate >= cm_experiment_setting.target_flow_setting.loss_rate_threshold)
    {
        if (loss_volume >= cm_experiment_setting.target_flow_setting.loss_volume_threshold) {
            //this is a target flow
            ht_kfs_vi_set(flow_selected_level_map, p_flow, TARGET_LEVEL);
        } else if (loss_volume >= cm_experiment_setting.target_flow_setting.selected_loss_volume_threshold) {
            //this is a selected flow
            ht_kfs_vi_set(flow_selected_level_map, p_flow, SELECTED_LEVEL);
        } else {
            //this is not a target flow nor a selected one
            ht_kfs_vi_del(flow_selected_level_map, p_flow);
        }
    } else {
        //this is not a target flow.
        //to simplity, just del the flow from flow_selected_level_map
        ht_kfs_vi_del(flow_selected_level_map, p_flow);
    }
}

void update_flow_not_sampled_volume(flow_src_t* p_flow) {
    hashtable_kfs_vi_t* flow_volume_map = data_warehouse_get_flow_volume_map();
    hashtable_kfs_vi_t* flow_not_sampled_volume_map = data_warehouse_get_flow_not_sampled_volume_map();

    //check whether flow_src is in flow_not_sampled_volume_map
    if (ht_kfs_vi_get(flow_not_sampled_volume_map, p_flow) < 0) {
        //not in flow_not_sampled_volume_map
        //add the not_sampled_volume into the map
        int volume = ht_kfs_vi_get(flow_volume_map, p_flow);
        if (volume < 0) {
            volume = 0;
        }
        ht_kfs_vi_set(flow_not_sampled_volume_map, p_flow, volume);
    }

}

/**
* @brief call when need to switch to another buffer
*/
void data_ware_rotate_buffer() {
    data_warehouse.active_idx = (data_warehouse.active_idx + 1) % BUFFER_NUM;
}

/**
* @brief This should be called after data_ware_rotate_buffer() is called, and after non-active buffer is useless
*
*
* @return 0-succ, -1-fail
*/
int data_warehouse_reset_noactive_buf() {
    //noactive buffer idx
    int na_idx = (data_warehouse.active_idx + 1) % BUFFER_NUM;
    //destory the hashmaps
    ht_kfs_vi_destory(data_warehouse.flow_volume_map[na_idx]);
    ht_kfs_fixSize_destory(data_warehouse.fixed_flow_loss_volume_map[na_idx]);
    ht_kfs_vi_destory(data_warehouse.flow_loss_volume_map[na_idx]);
    ht_kfs_vi_destory(data_warehouse.flow_selected_level_map[na_idx]);
    ht_kfs_vf_destory(data_warehouse.flow_loss_rate_map[na_idx]);
    ht_kfs_vi_destory(data_warehouse.flow_sample_map[na_idx]);
    ht_kfs_vi_destory(data_warehouse.flow_not_sampled_volume_map[na_idx]);

    //destory last_sent_flow_selected_level_map
    //This is to make sure that next interval, last_sent_flow_selected_level_map is clear
    ht_kfs_vi_destory(data_warehouse.last_sent_flow_selected_level_map);

    //recreate the hashmaps
    data_warehouse.flow_volume_map[na_idx] = ht_kfs_vi_create();
    if (data_warehouse.flow_volume_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.fixed_flow_loss_volume_map[na_idx] = ht_kfs_fixSize_create(cm_experiment_setting.sender_setting.fix_loss_map_bucket_size);
    if (data_warehouse.fixed_flow_loss_volume_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.flow_loss_volume_map[na_idx] = ht_kfs_vi_create();
    if (data_warehouse.flow_loss_volume_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.flow_loss_rate_map[na_idx] = ht_kfs_vf_create();
    if (data_warehouse.flow_loss_rate_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.flow_selected_level_map[na_idx] = ht_kfs_vi_create();
    if (data_warehouse.flow_selected_level_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.flow_sample_map[na_idx] = ht_kfs_vi_create();
    if (data_warehouse.flow_sample_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.flow_not_sampled_volume_map[na_idx] = ht_kfs_vi_create();
    if (data_warehouse.flow_not_sampled_volume_map[na_idx] == NULL) {
        return -1;
    }
    data_warehouse.last_sent_flow_selected_level_map = ht_kfs_vi_create();
    if (data_warehouse.last_sent_flow_selected_level_map == NULL) {
        return -1;
    }
    /* interval infor */
    data_warehouse.pkt_num_sent[na_idx] = 0;
    data_warehouse.volume_sent[na_idx] = 0;
    data_warehouse.condition_pkt_num_sent[na_idx] = 0;
    data_warehouse.volume_lost[na_idx] = 0;

    return 0;
}

hashtable_vl_t* data_warehouse_get_flow_recePktList_map() {
    return data_warehouse.flow_recePktList_map;
}

hashtable_kfs_vi_t* data_warehouse_get_flow_volume_map() {
    return data_warehouse.flow_volume_map[data_warehouse.active_idx];
}

hashtable_kfs_fixSize_t* data_warehouse_get_fixed_flow_loss_volume_map() {
    return data_warehouse.fixed_flow_loss_volume_map[data_warehouse.active_idx];
}

hashtable_kfs_vi_t* data_warehouse_get_flow_loss_volume_map() {
    return data_warehouse.flow_loss_volume_map[data_warehouse.active_idx];
}

hashtable_kfs_vf_t* data_warehouse_get_flow_loss_rate_map() {
    return data_warehouse.flow_loss_rate_map[data_warehouse.active_idx];
}

hashtable_kfs_vi_t* data_warehouse_get_flow_selected_level_map() {
    return data_warehouse.flow_selected_level_map[data_warehouse.active_idx];
}

hashtable_kfs_vi_t* data_warehouse_get_flow_sample_map() {
    return data_warehouse.flow_sample_map[data_warehouse.active_idx];
}

hashtable_kfs_vi_t* data_warehouse_get_flow_not_sampled_volume_map() {
    return data_warehouse.flow_not_sampled_volume_map[data_warehouse.active_idx];
}

hashtable_kfs_vi_t* data_warehouse_get_unactive_flow_selected_level_map() {
    return data_warehouse.flow_selected_level_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}

hashtable_kfs_vi_t* data_warehouse_get_unactive_flow_volume_map() {
    return data_warehouse.flow_volume_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}

hashtable_kfs_fixSize_t* data_warehouse_get_unactive_fixed_flow_loss_volume_map() {
    return data_warehouse.fixed_flow_loss_volume_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}

hashtable_kfs_vi_t* data_warehouse_get_unactive_flow_loss_volume_map() {
    return data_warehouse.flow_loss_volume_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}

hashtable_kfs_vf_t* data_warehouse_get_unactive_flow_loss_rate_map() {
    return data_warehouse.flow_loss_rate_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}

hashtable_kfs_vi_t* data_warehouse_get_unactive_flow_not_sampled_volume_map() {
    return data_warehouse.flow_not_sampled_volume_map[(data_warehouse.active_idx+1)%BUFFER_NUM];
}
