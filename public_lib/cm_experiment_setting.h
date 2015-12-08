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
#define CM_CONDITION_MILLISECONDS_POSTPOINE_FOR_SWITCH 10   //10 milliseconds, refer to time_library.h:get_next_sec_interval_start()

//packet dropping model
//#define CM_SWITCH_DROP_RATE 0.00

//#define CM_OPEN_REPLACE_MECHANISM 1

#define CM_SENDER_TARGET_FLOW_FNAME_PREFIX "/tmp/sender/"
#define CM_SENDER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_RECEIVER_TARGET_FLOW_FNAME_PREFIX "/tmp/switch/"
#define CM_RECEIVER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_EXPERIMENT_SETTING_FNAME "/home/xuemei/workspace/CM_testbed_code/public_lib/cm_experiment_setting.txt"

enum switch_mem_type_e {
    UNIFORM,
    DIVERSE
};

enum sender_mem_type_e {
    INFINITE,
    FIXED
};

enum host_switch_sample_e {
    HOST_SAMPLE,
    SWITCH_SAMPLE
};

//this is to decide whether the condition signal sent by injecting new packets or tagging existing packet
enum inject_tag_pkt_e {
    INJECT_PKT_AS_CONDITION,
    TAG_PKT_AS_CONDITION
};

typedef struct sample_hold_setting_s {
    int target_flow_volume_in_sampling;
    uint64_t switches_interval_volume[NUM_SWITCHES]; //for setting hold hashtable size
    uint64_t max_switch_interval_volume;
    // as we want to compare UNIFORM vs. DIVERSE, 
    // we need to make total mem used in UNIFORM model is the same as DIVERSE mode, 
    // when the switch_memory_times is the same
    float uniform_mem_ratio_to_diverse_mem;
    float default_byte_sampling_rate;
} sample_hold_setting_t;

typedef struct target_flow_setting_s {
    int volume_threshold;
    float loss_rate_threshold;
    int loss_volume_threshold;
}target_flow_setting_t;

typedef struct sender_setting_s {
    enum sender_mem_type_e loss_map_mem_type;
    float fix_loss_map_bucket_size;
    float default_loss_byte_sampling_rate;
}sender_setting_t;

typedef struct cm_experiment_setting_s {
    int interval_msec_len;
    int condition_msec_freq;
    bool replacement;
    enum switch_mem_type_e switch_mem_type;
    float switch_memory_times;   //the times of memory compared to default sample and hold
    float switch_drop_rate;
    enum host_switch_sample_e host_or_switch_sample;
    enum inject_tag_pkt_e inject_or_tag_packet;

    sample_hold_setting_t sample_hold_setting;
    target_flow_setting_t target_flow_setting;
    sender_setting_t sender_setting;
} cm_experiment_setting_t;

extern cm_experiment_setting_t cm_experiment_setting;

int init_cm_experiment_setting(void);
int read_cm_experiment_setting_from_file(void);
int init_other_experiment_setting(void);
int check_value_correct(void);

#endif
