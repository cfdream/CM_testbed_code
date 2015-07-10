#include "../../public_lib/time_library.h"
#include "interval_rotator.h"

FILE* init_target_flow_file() {
    FILE* fp;
    fp = fopen(CM_TARGET_FLOW_FNAME, "a+");
    if (fp == NULL) {
        return NULL;
    }
    fprintf(fp, "%s\t%s\t%s\t%s\t%s\n", "srcip", "dstip", "src_port", "dst_port", "protocol");
    return fp;
}

void write_target_flows_to_file(uint64_t current_sec, FILE* fp_target_flow) {
    assert(fp_target_flow != NULL);
    fprintf(fp_target_flow, "time-%lu seconds\n", current_sec);
    hashtable_kfs_t* target_flow_map_pre_interval = data_warehouse_get_unactive_target_flow_map();
    entry_kfs_t ret_entry;
    while (ht_kfs_next(target_flow_map_pre_interval, &ret_entry) == 0) {
        //get one target flow, output to file
        flow_s* p_flow = ret_entry.key;
        fprintf(fp_target_flow, "%u\t%u\t%u\t%u\t%u\n", p_flow->srcip, p_flow->dstip, p_flow->src_port, p_flow->dst_port, p_flow->protocol);
    }
}

void* rotate_interval(void* param_ptr) {
    //sleep two second
    sleep(2);

    //
    FILE* fp_target_flow = init_target_flow_file();
    if (fp_target_flow == NULL) {
        printf("FAIL: initilize file-%s\n", CM_TARGET_FLOW_FNAME);
        return;
    }

    while (true) {
        /* all hosts/senders start/end at the nearby timestamp for intervals */
        /* postpone till switching to next time interval */
        uint64_t current_sec = get_next_interval_start(CM_TIME_INTERVAL);

        //1. rotate the data warehouse buffer
        data_ware_rotate_buffer();

        //2. store the target flow identities of the past interval into file
        write_target_flows_to_file(current_sec, fp_target_flow);

        //3. reset the idel buffer of data warehouse
        data_warehouse_reset_noactive_buf();
    }

    //close file
    fclose(fp_target_flow);

    return NULL;
}
