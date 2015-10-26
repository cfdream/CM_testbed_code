#include "../../public_lib/get_mininet_host_name.h"
#include "../../public_lib/general_functions.h"
#include "task_manager.h"

#define TASK_INFO_FNAME "../public_lib/task_info.txt"

int init_tasks_info(task_manager_t* p_task_manager) {
    char hostname[100];
    if (get_mininet_host_name(hostname, 100) != 0) {
        printf("FAIL:get_mininet_host_name\n");
        return -1;
    }
    int hostid = atoi(hostname+1);
    if (read_tasks_from_file(p_task_manager, hostid) != 0) {
        return -1;
    }
    init_task_srcip_dstip_prefixes(p_task_manager);

    print_tasks_infor(p_task_manager);

    return 0;
}

int read_one_task_from_one_line(char* line_buffer, size_t len, task_t* p_task) {
    //taskid sender_hostid receiver_hostid s1_id, s2_id, ...
    char** tokens = str_split(line_buffer, ':');

    memset(p_task, 0, sizeof(task_t));

    int i = 0;
    while (tokens) {
        if (i == 0) {
            p_task->taskid = strtol(*tokens, NULL, 10);
        } else if (i == 1) {
            p_task->sender_host = strtol(*tokens, NULL, 10);
        } else if (i == 2) {
            p_task->receiver_host = strtol(*tokens, NULL, 10);
        } else if (i >= 3) {
            p_task->monitor_switch_ids[p_task->num_monitors] = strtol(*tokens, NULL, 10);
            ++p_task->num_monitors;
        }
        ++i;
    }
    
    if (p_task->num_monitors == 0) {
        printf("FAIL: read one task from line:%s", line_buffer);
        return -1;
    }
    return 0;
}

int read_tasks_from_file(task_manager_t* p_task_manager, int hostid) {
    memset(p_task_manager, 0, sizeof(task_manager_t));

    FILE* fp;
    //open file
    fp = fopen(TASK_INFO_FNAME, "r");
    if (fp == NULL) {
        printf("%s not exist\n", TASK_INFO_FNAME);
        return -1; 
    }
    
    char* line_buffer = NULL;
    size_t len = 0;
    while (getline(&line_buffer, &len, fp) != -1) {
        //get one line
        if (line_buffer[0] == '#') {
            continue;
        }
        task_t task;
        int ret = read_one_task_from_one_line(line_buffer, len, &task);
        if (ret == 0) {
            //for one specific host, we only care about the tasks that have selector on it.
            if(task.sender_host == hostid) {
                p_task_manager->tasks[p_task_manager->num_tasks] = task;
                ++p_task_manager->num_tasks;
            }
        } else {
            return -1;
        }
        free(line_buffer);
    }
    return 0;
}

void init_task_srcip_dstip_prefixes(task_manager_t* p_task_manager) {
    int i;
    for (i = 0; i < p_task_manager->num_tasks; i++) {
        p_task_manager->tasks[i].srcip_prefix = ITH_SENDER_START_IP(p_task_manager->tasks[i].sender_host-1);
        p_task_manager->tasks[i].dstip_prefix = ITH_SENDER_START_IP(p_task_manager->tasks[i].receiver_host-1);
    }
}

/**
* @brief get the task_t infor for specific packet
* assumption: between one pair of hosts, there is only one task
*
* @param p_flow
*
* @return 
*/
task_t* get_task_of_traffic(task_manager_t* p_task_manager, flow_src_t* p_flow) {
    #ifdef FLOW_SRC_DST
    uint32_t srcip_prefix = p_flow->srcip & IP_MASK;
    uint32_t dstip_prefix = p_flow->dstip & IP_MASK;
    int i = 0;
    for (i = 0; i < p_task_manager->num_tasks; i++) {
        if (srcip_prefix == p_task_manager->tasks[i].srcip_prefix 
            && dstip_prefix == p_task_manager->tasks[i].dstip_prefix) {
            //only need to compare dstip_prefix, as p_flow is the traffic sent from this host
            return &p_task_manager->tasks[i];
        }
    }
    #endif
    return NULL;
}

void print_tasks_infor(task_manager_t* p_task_manager) {
    int i = 0;
    for (i = 0; i < p_task_manager->num_tasks; i++) {
        task_t* p_task = &p_task_manager->tasks[i];
        printf("taskid:%d, sender:h%d, receiver:h%d, monitor[0]:s%d", 
            p_task->taskid,
            p_task->sender_host,
            p_task->receiver_host,
            p_task->monitor_switch_ids[0]);
    }
}
