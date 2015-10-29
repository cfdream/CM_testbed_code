import sys
import re
import statistics

class CONSTANTS():
    NUM_SENDER = 12
    NUM_SWITCH = 12

class host_target_flow_info_c():
    def __init__(self, srcip, dstip, volume, loss_volume, loss_rate):
        self.srcip = int(srcip)
        self.dstip = int(dstip)
        self.volume = volume
        self.loss_volume = volume
        self.loss_rate = loss_rate

class switch_flow_info_c():
    def __init__(self, srcip, dstip, real_volume, captured_volume):
        self.srcip = int(srcip)
        self.dstip = int(dstip)
        self.real_volume = real_volume
        self.captured_volume = captured_volume

class one_task_result_c():
    def __init__(self, target_flow_num, fn_num, fn, accuracy):
        self.target_flow_num = target_flow_num
        self.fn_num = fn_num
        self.fn = fn
        self.accuracy = accuracy
        #self.condition_map_size = 0

class one_setting_result_c():
    def __init__(self):
        self.avg_targetflow_num = 0
        self.avg_sample_map_size = 0
        self.raw_host_sample_switch_hold_accuracy = 0
        self.avg_fn_num = 0
        self.avg_fn = 0
        self.min_fn = 0
        self.max_fn = 0
        self.stdv_fn = 0
        self.avg_accuracy = 0
        self.min_accuracy = 0
        self.max_accuracy = 0
        self.stdv_accuracy = 0

class one_setting_result_calculator_c():
    #CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY = "CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY"
    def __init__(self):
        #setting
        self.host_switch_sample = 0
        self.replace = 0
        self.memory_type = 0
        self.freq = 0
        self.switches_sample_map_size = [None] * (CONSTANTS.NUM_SWITCH+1)
        #self.switches_condition_map_size = [None] * (CONSTANTS.NUM_SWITCH+1)
    
    def srcip_dstip_key(self, srcip, dstip):
        return "{0}_{1}" .format(srcip, dstip)

    def get_one_setting_result(self, one_setting_path, task_manager):
        one_setting_result = one_setting_result_c()

        print(one_setting_path)
        #1. read all target flows for each round for all hosts
        #format: key-hostid, value-rounds_target_flow_map {key: round_sec, value:{one_round_target_flow_map}}
        hosts_rounds_target_flows_map = {}
        self.read_hosts_rounds_target_flows(one_setting_path, hosts_rounds_target_flows_map, one_setting_result)
        
        #2. read flow infor for per switch per each round
        #pair: key-switch_id, value-{{sec, {flow info}},{sec, {flow info}},{sec, {flow info}}}
        switches_rounds_flow_info = {}
        self.read_switches_rounds_flow_info(one_setting_path, switches_rounds_flow_info, hosts_rounds_target_flows_map)
      
        task_result_map = {}
        self.calculate_metrics_for_tasks(task_manager, hosts_rounds_target_flows_map, switches_rounds_flow_info, task_result_map)
        
        self.calculate_one_setting_result(task_result_map, one_setting_result)
        return one_setting_result

    def read_hosts_rounds_target_flows(self, one_setting_path, hosts_rounds_target_flows_map, one_setting_result):
        host_path = "{0}/sender" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) milliseconds=====")
        target_flow_pattern = re.compile("^(\d+)\t(\d+)\t(\d+)\t(\d+.\d+)\t([-]*\d+)\t([-]*\d+)\t([-]*\d+)")
        raw_host_sample_switch_hold_accuracy = 0
        raw_host_sample_switch_hold_accuracy_cnt = 0
        not_sample_flow_num = 0

        for host_idx in range(1, CONSTANTS.NUM_SENDER+1):
            host_fname = "{0}/h{1}_intervals_target_flows.txt" .format(host_path, host_idx)
            print("start read {0}" .format(host_fname))

            #init rounds_target_flow_map for host_idx
            hosts_rounds_target_flows_map[host_idx] = {}
            rounds_target_flows_map = hosts_rounds_target_flows_map[host_idx]

            # 1. read the file 
            in_file = open(host_fname, 'r')
            lines = in_file.readlines()
            in_file.close()
            # 2. get per round target flows
            cur_round_sec = 0
            for line in lines:
                match = round_start_pattern.match(line)
                if match != None:
                    #new round start
                    cur_round_sec = int(int(match.group(1)) / 1000)
                    if cur_round_sec == '1439973225':
                        break
                    if cur_round_sec not in rounds_target_flows_map:
                        one_round_target_flow_map = {}
                        rounds_target_flows_map[cur_round_sec] = one_round_target_flow_map
                else:
                    match = target_flow_pattern.match(line)
                    if match != None:
                        #one target flow
                        srcip = match.group(1)
                        dstip = match.group(2)
                        volume = match.group(3)
                        loss_rate = match.group(4)
                        loss_volume = match.group(5)
                        not_sampled_volume = match.group(6)
                        host_target_flow = host_target_flow_info_c(srcip, dstip, volume, loss_volume, loss_rate)

                        one_round_target_flow_map = rounds_target_flows_map[cur_round_sec]
                        one_round_target_flow_map[self.srcip_dstip_key(srcip, dstip)] = host_target_flow
                        
                        raw_host_sample_switch_hold_accuracy_cnt += 1
                        raw_host_sample_switch_hold_accuracy += (1 - 1.0 * int(not_sampled_volume) / int(volume))
                        if not_sampled_volume == volume:
                            not_sample_flow_num += 1
            #print statistics infor for each round of the host
            for round_sec, one_round_target_flow_map in rounds_target_flows_map.items():
                print("host:{0}, round_sec:{1}, target_flow_num:{2}" .format(host_idx, round_sec, len(one_round_target_flow_map)))
            print("end read {0}" .format(host_fname))
        
        #output maximum accuracy, min fn that can be got at the host side
        if raw_host_sample_switch_hold_accuracy_cnt > 0:
            raw_host_sample_switch_hold_accuracy /= raw_host_sample_switch_hold_accuracy_cnt
        one_setting_result.raw_host_sample_switch_hold_accuracy = raw_host_sample_switch_hold_accuracy
        print("max_accuracy_at_host:{0}, min_fn_ratio_at_host:{1}" \
            .format(one_setting_result.raw_host_sample_switch_hold_accuracy, 
                1.0*not_sample_flow_num/raw_host_sample_switch_hold_accuracy_cnt))

    def read_switches_rounds_flow_info(self, one_setting_path, switches_rounds_flow_info, hosts_rounds_target_flows_map):
        switch_path = "{0}/switch" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) milliseconds=====")
        #new format
        flow_info_pattern_new = re.compile("^(\d+)\t(\d+)\t([-]*\d+)\t([-]*\d+)\t(\d+)")
        #old format
        flow_info_pattern_old = re.compile("^(\d+)\t(\d+)\t([-]*\d+)\t([-]*\d+)")
        sample_map_size_pattern = re.compile("^sample_hashmap_size:(\d+)")
        condition_map_size_pattern = re.compile("^condition_hashmap_size:(\d+)")
        condition_map_last_round_collision_num = re.compile("^condition_hashmap last rotate collision times:(\d+)")
        for switch_idx in range(1, CONSTANTS.NUM_SWITCH+1):
            switch_fname = "{0}/s{1}.result" .format(switch_path, switch_idx)
            print("start read {0}" .format(switch_fname))
            # 0. one new switch
            one_switch_rounds_info = {}
            switches_rounds_flow_info[switch_idx] = one_switch_rounds_info
            # 1. read the file 
            in_file = open(switch_fname, 'r')
            lines = in_file.readlines()
            in_file.close()
            #2. get per round flow info for the switch
            through_target_flow_num = 0
            cur_round_sec = 0
            cur_round_flow_num = 0
            signed_target_num = 0
            for line in lines:
                #get memory sizes
                match = sample_map_size_pattern.match(line)
                if match != None:
                    self.switches_sample_map_size[switch_idx] = int(match.group(1))
                #match = condition_map_size_pattern.match(line)
                #if match != None:
                #    self.switches_condition_map_size[switch_idx] = int(match.group(1))
                #match = condition_map_last_round_collision_num.match(line)
                #if match != None:
                #    one_switch_rounds_info[cur_round_sec][one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY] \
                #        = int(match.group(1))
                match = round_start_pattern.match(line)
                if match != None:
                    #new round start
                    cur_round_sec = int(int(match.group(1)) / 1000)
                    one_switch_one_round_info = {}
                    one_switch_rounds_info[cur_round_sec] = one_switch_one_round_info
                    cur_round_flow_num=0
                    through_target_flow_num = 0
                else:
                    cur_round_flow_num += 1

                    srcip = 0
                    dstip = 0
                    real_volume = 0
                    captured_volume = 0
                    matched = False
                    match_new = flow_info_pattern_new.match(line)
                    if match_new != None:
                        #one target flow
                        srcip = match_new.group(1)
                        dstip = match_new.group(2)
                        real_volume = int(match_new.group(3))
                        captured_volume = int(match_new.group(4))
                        matched = True
                    else:
                        match_old = flow_info_pattern_old.match(line)
                        if match_old != None:
                            srcip = match_old.group(1)
                            dstip = match_new.group(2)
                            real_volume = int(match_new.group(3))
                            captured_volume = int(match_new.group(4))
                            matched = True
                    if matched:
                        if cur_round_sec not in one_switch_rounds_info:
                            print("FATAL: switch_idx:{0}, cur_round_sec:{1} not exist in one_switch_rounds_info" \
                                .format(switch_idx, cur_round_sec))
                            continue
                        flow_info = switch_flow_info_c(srcip, dstip, real_volume, captured_volume)
                        one_switch_one_round_info = one_switch_rounds_info[cur_round_sec]
                        flow_key = self.srcip_dstip_key(srcip, dstip)
                        one_switch_one_round_info[flow_key] = flow_info
                        #check whether the flow is a target flow
                        is_target_flow = False
                        for senderid, rounds_target_flows_map in hosts_rounds_target_flows_map.items():
                            for round_sec, target_flow_map in rounds_target_flows_map.items():
                                if round_sec != cur_round_sec:
                                    continue
                                if flow_key in target_flow_map:
                                    is_target_flow = True
                                    break
                            if is_target_flow:
                                break
                        if is_target_flow:
                            through_target_flow_num += 1
            #print the last round statistics information
            if cur_round_sec != 0:
                print("sec:{0}, switch:{1}, flow num:{2}, through_target_flow_num:{3}" .format(cur_round_sec, switch_idx, len(one_switch_one_round_info), through_target_flow_num))
            print("end read {0}" .format(switch_fname))

        print("num switches:{0}" .format(len(switches_rounds_flow_info)))
        if len(switches_rounds_flow_info) > 0:
            for switch_idx, one_switch_rounds_info in switches_rounds_flow_info.items():
                for sec, one_switch_one_round_info in sorted(one_switch_rounds_info.items(), key=lambda pair: pair[0]):
                    #print("sec:{0}, switch:{1}, flow num:{2}, signed_target_num:{3}" .format(sec, switch_idx, len(one_switch_one_round_info), signed_target_num))
                    pass

    def flow_belongs_to_task(self, one_task, srcip, dstip):
        SENDERS_START_IP_PREFIX = int('0x0a000000', 16)
        ONE_SENDER_IP_NUM = int('0x01000000', 16)
        IP_MASK = int('0xff000000', 16)
        task_sender_ip_prefix = SENDERS_START_IP_PREFIX + (one_task.sender_hostid - 1) * ONE_SENDER_IP_NUM
        task_rece_ip_prefix = SENDERS_START_IP_PREFIX + (one_task.rece_hostid - 1) * ONE_SENDER_IP_NUM
        srcip_prefix = srcip & IP_MASK
        dstip_prefix = dstip & IP_MASK

        #print("sender_prefix:%x, rece_prefix:%x, srcip_prefix:%x, dstip_prefix:%x", task_sender_ip_prefix, task_rece_ip_prefix, srcip_prefix, dstip_prefix)
        if srcip_prefix == task_sender_ip_prefix and dstip_prefix == task_rece_ip_prefix:
            return True
        return False

    def calculate_metrics_for_tasks(self, task_manager, hosts_rounds_target_flows_map, switches_rounds_flow_info, task_result_map):
        for taskid, one_task in task_manager.task_map.items():
            print("task:{0}" .format(taskid))
            #1. get sender_hostid of the task
            sender_hostid = one_task.sender_hostid
           
            #for false negative
            all_target_flow_num = 0
            false_negative_target_flow_num = 0
            #for accuracy
            true_positive_target_flow_num = 0
            true_positive_target_flow_accuracy_sum = 0

            rounds_target_flows_map = hosts_rounds_target_flows_map[sender_hostid]
            first_round = True
            for round_sec, target_flow_map in sorted(rounds_target_flows_map.items(), key=lambda pair: pair[0]):
                if first_round:
                    #ignore the first round (for sync), we only care the second round
                    first_round = False
                    continue
                for flow_key, host_target_flow in target_flow_map.items():
                    #2. for one target flow from this sender, check whether it is a flow for the task
                    belong_to_task = self.flow_belongs_to_task(one_task, host_target_flow.srcip, host_target_flow.dstip)
                    if not belong_to_task:
                        continue
                    #print("task:{0}, srcip:{1}, dstip:{2}" .format(taskid, host_target_flow.srcip, host_target_flow.dstip))

                    #3. for one target flow of this task, check all switches
                    #3.1 if any switch observing the target flow has not captured the target flow, it is a FN
                    #3.2 for true positive, calculate average accuracy
                    flow_total_volume = 0
                    flow_total_captured_volume = 0
                    captured_by_all_observed_switches = True
                    for switchid, switch_rounds_flow_map in switches_rounds_flow_info.items():
                        #3.1.1 If the task has no selector on the switch, skip
                        if switchid not in one_task.selectors_switch_id_map:
                            continue
                        round_flow_map_of_switch = switch_rounds_flow_map[round_sec]
                        if flow_key not in round_flow_map_of_switch:
                            print("flow:{0} from sender {1} not through switch {2}" .format(flow_key, sender_hostid, switchid))
                            continue
                        switch_flow_info = round_flow_map_of_switch[flow_key]
                        #captured or not 
                        if switch_flow_info.real_volume > 0 and switch_flow_info.captured_volume <= 0:
                            captured_by_all_observed_switches = False
                            
                        flow_total_volume += switch_flow_info.real_volume
                        flow_total_captured_volume += switch_flow_info.captured_volume 
                        
                    if flow_total_volume == 0:
                        print("flow:{0} from sender {1} through no switches" .format(flow_key, sender_hostid, switchid))
                        continue

                    all_target_flow_num += 1
                    if not captured_by_all_observed_switches:
                        #flase negative
                        false_negative_target_flow_num += 1
                        continue

                    true_positive_target_flow_num += 1
                    true_positive_target_flow_accuracy_sum += (1.0 * flow_total_captured_volume / flow_total_volume)
            #avg accuracy of all true positive target flows
            avg_accuracy = 0
            if true_positive_target_flow_num > 0:
                avg_accuracy = true_positive_target_flow_accuracy_sum / true_positive_target_flow_num

            #false negative ratio
            false_negative_ratio = 0
            if all_target_flow_num > 0:
                false_negative_ratio = 1.0 * false_negative_target_flow_num / all_target_flow_num
            #result of one task
            print("task:{0}, target_flow_num:{1}, flows_avg_accuracy:{2}, fn:{3}" .format(taskid, all_target_flow_num, avg_accuracy, false_negative_ratio))
            one_task_result = one_task_result_c(all_target_flow_num, false_negative_target_flow_num, false_negative_ratio, avg_accuracy)
            task_result_map[taskid] = one_task_result
            
    def calculate_one_setting_result(self, task_result_map, one_setting_result):
        target_flow_num_list = []
        fn_num_list = []
        fn_list = []
        accuracy_list = []
        for taskid, task_result in task_result_map.items():
            target_flow_num_list.append(task_result.target_flow_num)
            fn_num_list.append(task_result.fn_num)
            fn_list.append(task_result.fn)
            accuracy_list.append(task_result.accuracy)
        #get avg stdv
        one_setting_result.avg_sample_map_size = statistics.mean(self.switches_sample_map_size[1:])
        one_setting_result.avg_targetflow_num = statistics.mean(target_flow_num_list)
        one_setting_result.avg_fn_num = statistics.mean(fn_num_list)
        one_setting_result.avg_fn = statistics.mean(fn_list)
        one_setting_result.avg_accuracy = statistics.mean(accuracy_list)
        one_setting_result.min_fn = min(fn_list)
        one_setting_result.max_fn = max(fn_list)
        one_setting_result.min_accuracy = min(accuracy_list)
        one_setting_result.max_accuracy = max(accuracy_list)
        if len(task_result_map) > 1:
            one_setting_result.stdv_fn = statistics.stdev(fn_list)
            one_setting_result.stdv_accuracy = statistics.stdev(accuracy_list)
