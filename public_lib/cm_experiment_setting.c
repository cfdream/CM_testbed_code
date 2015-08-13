#include "cm_experiment_setting.h"
#include "general_functions.h"
#include "sample_setting.h"
#include <stdio.h>
#include <stdlib.h>

cm_experiment_setting_t cm_experiment_setting;

int init_cm_experiment_setting(void) {
    if (read_cm_experiment_setting_from_file() != 0) {
        return -1;
    }
    if (init_other_experiment_setting()) {
        return -1;
    }
    return 0;
}

int read_cm_experiment_setting_from_file(void) {
    FILE* fp;
    //open file
    fp = fopen(CM_EXPERIMENT_SETTING_FNAME, "r");
    if (fp == NULL) {
        printf("%s not exist\n", CM_EXPERIMENT_SETTING_FNAME);
        return -1; 
    }
    
    char* line_buffer = NULL;
    size_t len = 0;
    int ith_switch = 0;
    while (getline(&line_buffer, &len, fp) != -1) {
        //get one line
        if (line_buffer[0] == '#') {
            continue;
        }

        int i;
        char** tokens = str_split(line_buffer, ':');

        if (tokens) {
            //interval_sec_len
            if (strcmp(*(tokens+0), "interval_sec_len") == 0 && *(tokens+1)) {
                cm_experiment_setting.interval_sec_len = strtol(*(tokens+1), NULL, 10);
            }
            //condition_sec_freq
            if (strcmp(*(tokens+0), "condition_sec_freq") == 0 && *(tokens+1)) {
                cm_experiment_setting.condition_sec_freq = strtol(*(tokens+1), NULL, 10);
            }
            //replacement
            if (strcmp(*(tokens+0), "replacement") == 0 && *(tokens+1)) {
                cm_experiment_setting.replacement = (bool)strtol(*(tokens+1), NULL, 10);
            }
            //switch_mem_type
            if (strcmp(*(tokens+0), "switch_mem_type") == 0 && *(tokens+1)) {
                cm_experiment_setting.switch_mem_type = (enum switch_mem_type_e)strtol(*(tokens+1), NULL, 10);
            }
            //switch_drop_rate
            if (strcmp(*(tokens+0), "switch_drop_rate") == 0 && *(tokens+1)) {
                cm_experiment_setting.switch_drop_rate = strtof(*(tokens+1), NULL);
            }
            //host_or_switch_sample
            if (strcmp(*(tokens+0), "host_or_switch_sample") == 0 && *(tokens+1)) {
                cm_experiment_setting.host_or_switch_sample = (enum host_switch_sample_e)strtol(*(tokens+1), NULL, 10);
            }
            //target_flow_volume
            if (strcmp(*(tokens+0), "target_flow_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.target_flow_setting.volume_threshold = strtol(*(tokens+1), NULL, 10);
            }
            //target_flow_loss_rate
            if (strcmp(*(tokens+0), "target_flow_loss_rate") == 0 && *(tokens+1)) {
                cm_experiment_setting.target_flow_setting.loss_rate_threshold = strtof(*(tokens+1), NULL);
            }

            //switch1_interval_volume
            if (strcmp(*(tokens+0), "switch1_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[0] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch2_interval_volume
            if (strcmp(*(tokens+0), "switch2_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[1] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch3_interval_volume
            if (strcmp(*(tokens+0), "switch3_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[2] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch4_interval_volume
            if (strcmp(*(tokens+0), "switch4_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[3] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch5_interval_volume
            if (strcmp(*(tokens+0), "switch5_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[4] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch6_interval_volume
            if (strcmp(*(tokens+0), "switch6_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[5] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch7_interval_volume
            if (strcmp(*(tokens+0), "switch7_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[6] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch8_interval_volume
            if (strcmp(*(tokens+0), "switch8_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[7] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch9_interval_volume
            if (strcmp(*(tokens+0), "switch9_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[8] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch10_interval_volume
            if (strcmp(*(tokens+0), "switch10_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[9] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch11_interval_volume
            if (strcmp(*(tokens+0), "switch11_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[10] = strtoll(*(tokens+1), NULL, 10);
            }
            //switch12_interval_volume
            if (strcmp(*(tokens+0), "switch12_interval_volume") == 0 && *(tokens+1)) {
                cm_experiment_setting.sample_hold_setting.switches_interval_volume[11] = strtoll(*(tokens+1), NULL, 10);
            }
        }

        //free(line_buffer);
    }

    //close file
    fclose(fp);
    return 0;
}

int init_other_experiment_setting(void) {
    cm_experiment_setting.sample_hold_setting.default_byte_sampling_rate 
        = 1.0 / cm_experiment_setting.target_flow_setting.volume_threshold * OVER_SAMPLING_RATIO;
    return 0;
}
