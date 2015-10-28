#!/usr/bin/python

import time
import re
import os
import sys

class Task():
    def __init__(self, taskid, sender_hostid, rece_hostid, selectors_switch_id_map):
        self.taskid = taskid
        self.sender_hostid = sender_hostid
        self.rece_hostid = rece_hostid
        self.selectors_switch_id_map = selectors_switch_id_map
    
class TaskManager():
    def __init__(self):
        #key: taskid, value: Task
        self.task_map = {}

    def readTasksFromFile(self, task_file_name):
        print("start read {0}\n" .format(task_file_name))
        # 1. read the file 
        in_file = open(task_file_name, 'r')
        lines = in_file.readlines()
        in_file.close()
        
        num_tasks = 0
        for line in lines:
            line = line[:-1]
            if line[0] == '#':
                continue
            num_tasks += 1
            items = line.split()
            taskid = items[0]
            sender_hostid = int(items[1])
            rece_hostid = int(items[2])
            selectors_switch_id_map = {}
            for i in range(3, len(items)):
                switchid = int(items[i])
                selectors_switch_id_map[switchid] = 1
            one_task = Task(taskid, sender_hostid, rece_hostid, selectors_switch_id_map)
            self.task_map[taskid] = one_task
        print("end read {0}. {1} tasks read\n" .format(task_file_name, num_tasks))
