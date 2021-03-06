#include <unistd.h>
#include "../../public_lib/time_library.h"
#include "interval_rotator.h"

extern cm_experiment_setting_t cm_experiment_setting;

FILE* init_target_flow_file() {
    FILE* fp;
    //1. get hostname
    char hostname[100];
    if (get_mininet_host_name(hostname, 100) != 0) {
        printf("FAIL:get_mininet_host_name\n");
        return NULL;
    }
    //2. generate target_flow_fname
    char fname[200];
    snprintf(fname, 200, "%s%s%s", CM_SENDER_TARGET_FLOW_FNAME_PREFIX,
        hostname, CM_SENDER_TARGET_FLOW_FNAME_SUFFIX );
    //3. open fname
    fp = fopen(fname, "w+");
    if (fp == NULL) {
        return NULL;
    }
    #ifdef FLOW_SRC
    fprintf(fp, "%s\n", "srcip");
    #endif
    #ifdef FLOW_SRC_DST
    fprintf(fp, "%s\n", "srcip\tdstip");
    #endif
    fflush(fp);
    return fp;
}

void write_last_sent_flow_selected_level_map_lost_target_flow(FILE* fp_target_flow) {
    hashtable_kfs_vi_t* flow_selected_level_map_pre_interval = data_warehouse_get_unactive_flow_selected_level_map();
    hashtable_kfs_vi_t* last_sent_flow_selected_level_map = data_warehouse.last_sent_flow_selected_level_map;

    int final_target_flow_num = 0;
    int final_lost_num_in_last_sent_flow_selected_level_map = 0;
    entry_kfs_vi_t ret_entry;
    while (ht_kfs_vi_next(flow_selected_level_map_pre_interval, &ret_entry) == 0) {
        //get one target flow, output to file
        flow_src_t* p_flow = &ret_entry.key;

        //just care about target flows
        if (ret_entry.value != TARGET_LEVEL) {
            continue;
        }

        //------------if the flow is not interested by any task on this host. No need to output it-------------
        task_t* p_task = get_task_of_traffic(&data_warehouse.task_manager, p_flow);
        if (p_task == NULL) {
            continue;
        }
        
        ++final_target_flow_num;
        if (ht_kfs_vi_get(last_sent_flow_selected_level_map, p_flow) != TARGET_LEVEL) {
                ++ final_lost_num_in_last_sent_flow_selected_level_map;
        }
    }
    fprintf(fp_target_flow, "target_flow_num:%d, lost_target_flow_num_last_sent_conditionmap:%d\n", 
          final_target_flow_num, final_lost_num_in_last_sent_flow_selected_level_map);
}

void write_interval_info_to_file(uint64_t current_msec, FILE* fp_target_flow) {
    if (fp_target_flow == NULL) {
        printf("fp_target_flow == NULL\n");
        return;
    }
    //manditory infor
    fprintf(fp_target_flow, "=====time-%lu milliseconds=====\n", current_msec);
    fprintf(fp_target_flow, "interval_msec_len:%d\n", cm_experiment_setting.interval_msec_len);
    fprintf(fp_target_flow, "condition_msec_freq:%d\n", cm_experiment_setting.condition_msec_freq);
    fprintf(fp_target_flow, "replacement:%d\n", cm_experiment_setting.replacement);
    fprintf(fp_target_flow, "switch_mem_type:%d\n", cm_experiment_setting.switch_mem_type);
    fprintf(fp_target_flow, "host_or_switch_sample:%d\n", cm_experiment_setting.host_or_switch_sample);
    fprintf(fp_target_flow, "switch_drop_rate:%f\n", cm_experiment_setting.switch_drop_rate);
    fprintf(fp_target_flow, "target_flow_volume_threshold:%d\n", cm_experiment_setting.target_flow_setting.volume_threshold);
    fprintf(fp_target_flow, "target_flow_loss_rate_threshold:%f\n", cm_experiment_setting.target_flow_setting.loss_rate_threshold);
    fprintf(fp_target_flow, "target_flow_loss_volume_threshold:%d\n", cm_experiment_setting.target_flow_setting.loss_volume_threshold);

    //optional infor
    int na_idx = (data_warehouse.active_idx+1)%BUFFER_NUM;
    fprintf(fp_target_flow, "pkt_num_sent:%lu\n", data_warehouse.pkt_num_sent[na_idx]);
    fprintf(fp_target_flow, "volume_sent:%lu\n", data_warehouse.volume_sent[na_idx]);
    fprintf(fp_target_flow, "volume_lost_detected:%lu\n", data_warehouse.volume_lost[na_idx]);
    if (data_warehouse.volume_sent[na_idx] > 0) {
        fprintf(fp_target_flow, "volume_lost_rate_detected:%f\n", 1.0*data_warehouse.volume_lost[na_idx] / data_warehouse.volume_sent[na_idx]);
    }
    fprintf(fp_target_flow, "condition_pkt_num_sent:%lu\n", data_warehouse.condition_pkt_num_sent[na_idx]);

    int loss_rate_ok_flow_num = 0;
    hashtable_kfs_vf_t* flow_loss_rate_map_pre_interval = data_warehouse_get_unactive_flow_loss_rate_map();
    entry_kfs_vf_t ret_vf_entry;
    while(ht_kfs_vf_next(flow_loss_rate_map_pre_interval, &ret_vf_entry) == 0) {
        if (ret_vf_entry.value > cm_experiment_setting.target_flow_setting.loss_rate_threshold) {
            ++loss_rate_ok_flow_num;
        }
    }
    fprintf(fp_target_flow, "loss_rate_ok_flow_num:%d\n", loss_rate_ok_flow_num);
    
    int volume_ok_flow_num = 0;
    int unique_flow_num_sent = 0;
    hashtable_kfs_vi_t* flow_volume_map_pre_interval = data_warehouse_get_unactive_flow_volume_map();
    entry_kfs_vi_t ret_entry;
    while(ht_kfs_vi_next(flow_volume_map_pre_interval, &ret_entry) == 0){
        if (ret_entry.value > cm_experiment_setting.target_flow_setting.volume_threshold) {
            ++volume_ok_flow_num;
        }
        ++unique_flow_num_sent;
    }
    fprintf(fp_target_flow, "volume_ok_flow_num:%d\n", volume_ok_flow_num);
    fprintf(fp_target_flow, "unique_flow_num_sent:%d\n", unique_flow_num_sent);
}

void write_target_flows_to_file(uint64_t current_msec, FILE* fp_target_flow) {
    //assert(fp_target_flow != NULL);
    if (fp_target_flow == NULL) {
        printf("fp_target_flow == NULL\n");
        return;
    }
    fflush(fp_target_flow);
    hashtable_kfs_vi_t* flow_selected_level_map_pre_interval = data_warehouse_get_unactive_flow_selected_level_map();
    hashtable_kfs_vi_t* flow_volume_map_pre_interval = data_warehouse_get_unactive_flow_volume_map();
    hashtable_kfs_vi_t* flow_loss_volume_map_pre_interval = data_warehouse_get_unactive_flow_loss_volume_map();
    hashtable_kfs_fixSize_t* fixed_flow_loss_volume_map_pre_interval = data_warehouse_get_unactive_fixed_flow_loss_volume_map();
    hashtable_kfs_vf_t* flow_loss_rate_map_pre_interval = data_warehouse_get_unactive_flow_loss_rate_map();
    hashtable_kfs_vi_t* flow_not_sampled_volume_map = data_warehouse_get_unactive_flow_not_sampled_volume_map();
    hashtable_kfs_vi_t* last_sent_flow_selected_level_map = data_warehouse.last_sent_flow_selected_level_map;

    entry_kfs_vi_t ret_entry;
    while (ht_kfs_vi_next(flow_selected_level_map_pre_interval, &ret_entry) == 0) {
        //just care about target flow
        if (ret_entry.value != TARGET_LEVEL) {
            continue;
        }

        //get one target flow, output to file
        flow_src_t* p_flow = &ret_entry.key;

        //------------if the flow is not interested by any task on this host. No need to output it-------------
        task_t* p_task = get_task_of_traffic(&data_warehouse.task_manager, p_flow);
        if (p_task == NULL) {
            continue;
        }
        
        int volume = ht_kfs_vi_get(flow_volume_map_pre_interval, p_flow);
        float loss_rate = ht_kfs_vf_get(flow_loss_rate_map_pre_interval, p_flow);
        int loss_volume = ht_kfs_vi_get(flow_loss_volume_map_pre_interval, p_flow);
        if (cm_experiment_setting.sender_setting.loss_map_mem_type == FIXED) {
            loss_volume = 0;
            entry_kfs_fixSize_t ret_entry;
            for (int i = 0; i < 100; i++)
                printf("tag5\n");
            if(ht_kfs_fixSize_get(fixed_flow_loss_volume_map_pre_interval, p_flow, &ret_entry) == 0) {
                loss_volume = ret_entry.value;
            }
            for (int i = 0; i < 100; i++)
                printf("tag6\n");
        }
        int not_sampled_volume = ht_kfs_vi_get(flow_not_sampled_volume_map, p_flow);
        int target_flow_sent_out = ht_kfs_vi_get(last_sent_flow_selected_level_map, p_flow);
        #ifdef FLOW_SRC
        fprintf(fp_target_flow, "%u\t%d\t%f\t%d\t%d\t%d\n", p_flow->srcip, volume, loss_rate, loss_volume, not_sampled_volume, target_flow_sent_out);
        #endif
        #ifdef FLOW_SRC_DST
        fprintf(fp_target_flow, "%u\t%u\t%d\t%f\t%d\t%d\t%d\n", p_flow->srcip, p_flow->dstip, volume, loss_rate, loss_volume, not_sampled_volume, target_flow_sent_out);
        #endif
        fflush(fp_target_flow);
    }
}

void write_all_flows_to_file(uint64_t current_msec, FILE* fp_target_flow) {
    //assert(fp_target_flow != NULL);
    if (fp_target_flow == NULL) {
        printf("fp_target_flow == NULL\n");
        return;
    }
    fflush(fp_target_flow);
    hashtable_kfs_vi_t* flow_volume_map_pre_interval = data_warehouse_get_unactive_flow_volume_map();
    hashtable_kfs_vi_t* flow_loss_volume_map_pre_interval = data_warehouse_get_unactive_flow_loss_volume_map();
    hashtable_kfs_vf_t* flow_loss_rate_map_pre_interval = data_warehouse_get_unactive_flow_loss_rate_map();
    hashtable_kfs_vi_t* flow_not_sampled_volume_map = data_warehouse_get_unactive_flow_not_sampled_volume_map();
    hashtable_kfs_vi_t* last_sent_flow_selected_level_map = data_warehouse.last_sent_flow_selected_level_map;
    hashtable_kfs_fixSize_t* fixed_flow_loss_volume_map_pre_interval = data_warehouse_get_unactive_fixed_flow_loss_volume_map();

    entry_kfs_vi_t ret_entry;
    while (ht_kfs_vi_next(flow_volume_map_pre_interval, &ret_entry) == 0) {
        //get one target flow, output to file
        flow_src_t* p_flow = &ret_entry.key;
        int volume = ht_kfs_vi_get(flow_volume_map_pre_interval, p_flow);
        float loss_rate = ht_kfs_vf_get(flow_loss_rate_map_pre_interval, p_flow);
        int loss_volume = ht_kfs_vi_get(flow_loss_volume_map_pre_interval, p_flow);
        if (cm_experiment_setting.sender_setting.loss_map_mem_type == FIXED) {
            loss_volume = 0;
            entry_kfs_fixSize_t ret_entry;
            for (int i = 0; i < 100; i++)
                printf("tag7\n");
            if(ht_kfs_fixSize_get(fixed_flow_loss_volume_map_pre_interval, p_flow, &ret_entry) == 0) {
                loss_volume = ret_entry.value;
            }
            for (int i = 0; i < 100; i++)
                printf("tag8\n");
        }
        int not_sampled_volume = ht_kfs_vi_get(flow_not_sampled_volume_map, p_flow);
        int target_flow_sent_out = ht_kfs_vi_get(last_sent_flow_selected_level_map, p_flow);
        #ifdef FLOW_SRC
        fprintf(fp_target_flow, "%u\t%d\t%f\t%d\t%d\t%d\n", p_flow->srcip, volume, loss_rate, loss_volume, not_sampled_volume, target_flow_sent_out);
        #endif
        #ifdef FLOW_SRC_DST
        fprintf(fp_target_flow, "%u\t%u\t%d\t%f\t%d\t%d\t%d\n", p_flow->srcip, p_flow->dstip, volume, loss_rate, loss_volume, not_sampled_volume, target_flow_sent_out);
        #endif
        fflush(fp_target_flow);
    }
}

void* rotate_interval(void* param_ptr) {
    //sleep two second
    sleep(2);

    //
    FILE* fp_target_flow = init_target_flow_file();
    if (fp_target_flow == NULL) {
        printf("FAIL: initilize target_flow_file\n");
        return NULL;
    }
    
    //wait to next whole 10 minutes <=> 600s <=> 600000 minisecond
    uint64_t current_msec = get_next_interval_start(600000);

    while (true) {
		//lock the data_warehouse.data_warehouse_mutex
		//in order to avoid IntervalRotator thread destory the data
        pthread_mutex_lock(&data_warehouse.data_warehouse_mutex);

        printf("=====start rotate_interval, current_msec:%lu=====\n", current_msec);

        //1. rotate the data warehouse buffer
        data_ware_rotate_buffer();

        //2. store the target flow identities of the past interval into file
        //2.1
        write_interval_info_to_file(current_msec, fp_target_flow);
        //2.2 
        write_last_sent_flow_selected_level_map_lost_target_flow(fp_target_flow);
        //2.3
        write_target_flows_to_file(current_msec, fp_target_flow);
        //write_all_flows_to_file(current_msec, fp_target_flow);

        //3. reset the idel buffer of data warehouse
        data_warehouse_reset_noactive_buf();

        pthread_mutex_unlock(&data_warehouse.data_warehouse_mutex);

		struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        uint64_t msec = (intmax_t)((time_t)spec.tv_sec*1000 + spec.tv_nsec/1000000);
        printf("=====end rotate_interval, current_msec:%lu=====\n", msec);
        
        //get time(msec) used to rotate
        clock_gettime(CLOCK_REALTIME, &spec);
        //1s = 10^3 msec, 1 ns = 10^-6 msec
        uint64_t after_rotate_msec = (intmax_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
        uint32_t rotate_duration_msec = after_rotate_msec - current_msec;
        //get time(msec) that should be slept
        uint32_t to_sleep_msec = cm_experiment_setting.interval_msec_len - rotate_duration_msec;
        
        //wait one interval length
        struct timespec to_sleep_time;
        to_sleep_time.tv_sec = to_sleep_msec / 1000;
        to_sleep_time.tv_nsec = (to_sleep_msec % 1000) * 1000000;
        nanosleep(&to_sleep_time, NULL);

        clock_gettime(CLOCK_REALTIME, &spec);
        //1s = 10^3 msec, 1 ns = 10^-6 msec
        current_msec = (intmax_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
    }

    //close file
    fclose(fp_target_flow);

    return NULL;
}
