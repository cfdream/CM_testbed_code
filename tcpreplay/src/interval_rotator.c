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
    fprintf(fp, "%s\n", "srcip");
    fflush(fp);
    return fp;
}

void write_target_flows_to_file(uint64_t current_sec, FILE* fp_target_flow) {
    //assert(fp_target_flow != NULL);
    if (fp_target_flow == NULL) {
        printf("fp_target_flow == NULL\n");
        return;
    }
    fprintf(fp_target_flow, "time-%lu seconds\n", current_sec);
    fflush(fp_target_flow);
    hashtable_kfs_vi_t* target_flow_map_pre_interval = data_warehouse_get_unactive_target_flow_map();
    entry_kfs_vi_t ret_entry;
    while (ht_kfs_vi_next(target_flow_map_pre_interval, &ret_entry) == 0) {
        //get one target flow, output to file
        flow_s* p_flow = ret_entry.key;
        fprintf(fp_target_flow, "%u\n", p_flow->srcip);
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

    while (true) {
        /* all hosts/senders start/end at the nearby timestamp for intervals */
        /* postpone till switching to next time interval */
        uint64_t current_sec = get_next_interval_start(cm_experiment_setting.interval_sec_len);

		//lock the data_warehouse.data_warehouse_mutex
		//in order to avoid IntervalRotator thread destory the data
        pthread_mutex_lock(&data_warehouse.data_warehouse_mutex);

        printf("=====start rotate_interval, current_sec:%lu=====\n", current_sec);

        //1. rotate the data warehouse buffer
        data_ware_rotate_buffer();

        //2. store the target flow identities of the past interval into file
        write_target_flows_to_file(current_sec, fp_target_flow);

        //3. reset the idel buffer of data warehouse
        data_warehouse_reset_noactive_buf();

        pthread_mutex_unlock(&data_warehouse.data_warehouse_mutex);

		struct timespec spec;
        clock_gettime(CLOCK_REALTIME, &spec);
        uint64_t sec = (intmax_t)((time_t)spec.tv_sec);
        printf("=====end rotate_interval, current_sec:%lu=====\n", sec);
    }

    //close file
    fclose(fp_target_flow);

    return NULL;
}
