/**
* assumption: between one pair of hosts, there is only one task
* this is for one host, task_manager_t will read tasks that have selector on this host
*/
#ifndef __TASK_MANAGER_H__
#define __TASK_MANAGER_H__

#include <stdlib.h>
#include <stdint.h>
#include "../../public_lib/packet.h"
#include "../../public_lib/flow.h"

#define MAX_TASK_NUM 200

/**
* @brief task definiiton between one sender and one receiver
*/
typedef struct task_s {
    int taskid;
    /**
    * @brief selector will locate at sender
    */
    int sender_host;
    int receiver_host;

    /**
    * @brief monitors
    */
    int num_monitors;
    int monitor_switch_ids[12];
    
    /**
    * @brief 
    */
    uint32_t srcip_prefix;
    uint32_t dstip_prefix;
} task_t;

typedef struct task_manager_s {
    int num_tasks;
    task_t tasks[MAX_TASK_NUM];
} task_manager_t;

int init_tasks_info(task_manager_t* p_task_manager);
int read_tasks_from_file(task_manager_t* p_task_manager, int hostid);
void init_task_srcip_dstip_prefixes(task_manager_t* p_task_manager);

task_t* get_task_of_traffic(task_manager_t* p_task_manager, flow_src_t* p_flow);

void print_tasks_infor(task_manager_t* p_task_manager, int hostid);

#endif
