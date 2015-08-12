#ifndef __CM_EXPERIMENT_SETTING_H__
#define __CM_EXPERIMENT_SETTING_H__

#define NUM_SENDERS 12
#define NUM_SWITCHES 12

#define CM_TIME_INTERVAL 75
#define ONE_INTERVAL_TOTAL_VOLUME 102679970 //volume in 0.5 sec

//condition
#define CM_CONDITION_TIME_INTERVAL 3 //cannot be 1
#define CM_CONDITION_TIME_INTERVAL_POSTPOINE_FOR_SWITCH 1

//packet dropping model
#define RAND_MOD_NUMBER 1000000
#define CM_SWITCH_DROP_RATE 0.01

#define CM_SENDER_TARGET_FLOW_FNAME_PREFIX "/tmp/sender/"
#define CM_SENDER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_RECEIVER_TARGET_FLOW_FNAME_PREFIX "/tmp/switch/"
#define CM_RECEIVER_TARGET_FLOW_FNAME_SUFFIX "_intervals_target_flows.txt"

#define CM_OPEN_REPLACE_MECHANISM 1

#endif
