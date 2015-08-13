#ifndef __CM_EXPERIMENT_SETTING_H__
#define __CM_EXPERIMENT_SETTING_H__

#include <stdbool.h>
#include <stdint.h>

#define NUM_SENDERS 12
#define NUM_SWITCHES 12

//#define CM_TIME_INTERVAL 75
//#define ONE_INTERVAL_TOTAL_VOLUME 102679970 //volume in 0.5 sec

//condition
//#define CM_CONDITION_TIME_INTERVAL 3 //cannot be 1
#define CM_CONDITION_TIME_INTERVAL_POSTPOINE_FOR_SWITCH 1

//packet dropping model
//#define CM_SWITCH_DROP_RATE 0.00

//#define CM_OPEN_REPLACE_MECHANISM 1

#define CM_SENDER_TARGET_FLOW_FNAME_PREFIX "/tmp/sender/"
#define CM_SENDER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_RECEIVER_TARGET_FLOW_FNAME_PREFIX "/tmp/switch/"
#define CM_RECEIVER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_EXPERIMENT_SETTING_FNAME "cm_experiment_setting.txt"

enum switch_mem_type_e {
    UNIFORM,
    DIVERSE
};

typedef struct cm_experiment_setting_s {
    int interval_sec_len;
    int condition_sec_freq;
    bool replacement;
    enum switch_mem_type_e switch_mem_type;
    float switch_drop_rate;
    uint64_t switches_interval_volume[NUM_SWITCHES];
} cm_experiment_setting_t;

int read_cm_experiment_setting_from_file(void);

#endif
