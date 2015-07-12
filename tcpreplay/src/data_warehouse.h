#ifndef __DATA_WAREHOUSE_H__
#define __DATA_WAREHOUSE_H__

#include "../../public_lib/mt_hashtable_kFlow_vLinklist.h"
#include "../../public_lib/mt_hashtable_kFlowSrc_vInt.h"
#include "../../public_lib/mt_hashtable_kFlowSrc_vFloat.h"

#define BUFFER_NUM 2

/**
* @brief 
*/
typedef struct data_warehouse_s {
    /*This is the hashmap recording sent pkts for each flow, only one buffer*/
    /* 5-tuple flow */
    hashtable_vl_t* flow_recePktList_map;

    /*Following are 2-buffer hashtables*/
    int active_idx;
    //4 hashtables for recording flow properties
    //here are <srcip> flow
    hashtable_kfs_vi_t* flow_volume_map[BUFFER_NUM];
    hashtable_kfs_vi_t* flow_loss_volume_map[BUFFER_NUM];
    hashtable_kfs_vf_t* flow_loss_rate_map[BUFFER_NUM];
    hashtable_kfs_vi_t* target_flow_map[BUFFER_NUM];

    /* 1 hashtable for sample and hold*/
    hashtable_kfs_vi_t* flow_sample_map[BUFFER_NUM];
}data_warehouse_t;

data_warehouse_t data_warehouse;

/**
* @brief init the data warehouse, at the beginning, only the first buffer should be initiliazed
*
* @return 0-succ -1:fail
*/
int data_warehouse_init();

/**
* @brief call when need to switch to another buffer
*/
void data_ware_rotate_buffer();

/**
* @brief This should be called after data_ware_rotate_buffer() is called, and after non-active buffer is useless
*
*
* @return 0-succ, -1-fail
*/
int data_warehouse_reset_noactive_buf();

hashtable_vl_t* data_warehouse_get_flow_recePktList_map();

hashtable_kfs_vi_t* data_warehouse_get_flow_volume_map();

hashtable_kfs_vi_t* data_warehouse_get_flow_loss_volume_map();

hashtable_kfs_vf_t* data_warehouse_get_flow_loss_rate_map();

hashtable_kfs_vi_t* data_warehouse_get_target_flow_map();

hashtable_kfs_vi_t* data_warehouse_get_flow_sample_map();

hashtable_kfs_vi_t* data_warehouse_get_unactive_target_flow_map();

#endif
