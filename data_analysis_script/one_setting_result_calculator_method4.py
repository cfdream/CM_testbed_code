import sys
import re
import statistics

class CONSTANTS():
    NUM_SENDER = 12
    NUM_SWITCH = 12

class switch_flow_info_c():
    def __init__(self, real_volume, captured_volume, signed_target, target_switch_received):
        self.real_volume = real_volume
        self.captured_volume = captured_volume
        self.signed_target = signed_target
        self.target_switch_received = target_switch_received

class switch_one_round_result_c():
    def __init__(self, all_volume_one_interval, all_flow_num_one_interval, real_target_flow_num, fn, fp, accuracy, fn_num, fn_num_not_targetflow, fn_num_not_captured, fn_num_all_switches_not_signed_target):
        self.all_volume_one_interval = all_volume_one_interval
        self.all_flow_num_one_interval = all_flow_num_one_interval
        self.real_target_flow_num = real_target_flow_num
        self.fn = fn
        self.fp = fp
        self.accuracy = accuracy
        self.fn_num = fn_num
        self.fn_num_not_targetflow = fn_num_not_targetflow
        self.fn_num_not_captured  = fn_num_not_captured
        self.fn_num_all_switches_not_signed_target = fn_num_all_switches_not_signed_target
        self.sample_map_size = 0
        #self.condition_map_size = 0

class avg_switch_result_one_setting_c():
    def __init__(self):
        self.avg_fn = 0
        self.stdv_fn = 0
        self.avg_fp = 0
        self.stdv_fp = 0
        self.avg_accuracy = 0
        self.stdv_accuracy = 0
        self.sample_map_size = 0
        #self.condition_map_size = 0
        self.avg_all_volume_one_interval = 0
        self.avg_all_flow_num_one_interval = 0
        self.avg_real_target_flow_num = 0
        self.avg_fn_num = 0
        self.avg_fn_num_not_targetflow = 0
        self.avg_fn_num_not_captured = 0
        self.avg_fn_num_all_switches_not_signed_target = 0

class one_setting_result_c():
    def __init__(self):
        self.avg_fn = 0
        self.min_fn = 0
        self.max_fn = 0
        self.stdv_fn = 0
        self.avg_targetflow_num = 0
        self.avg_fn_num = 0
        self.avg_fn_num_not_targetflow = 0
        self.avg_fn_num_not_captured = 0
        self.avg_fn_num_all_switches_not_signed_target = 0
        self.avg_fp = 0
        self.min_fp = 0
        self.max_fp = 0
        self.stdv_fp = 0
        self.avg_accuracy = 0
        self.min_accuracy = 0
        self.min_accuracy = 0
        self.stdv_accuracy = 0
        self.avg_sample_map_size = 0
        self.min_sample_map_size = 0
        self.max_sample_map_size = 0
        self.avg_condition_map_size = 0
        self.min_condition_map_size = 0
        self.max_condition_map_size = 0
        self.raw_host_sample_switch_hold_accuracy = 0

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
    
    def get_one_setting_result(self, one_setting_path):
        one_setting_result = one_setting_result_c()

        print(one_setting_path)
        #1. read all target flows for each round
        #format: {{sec, {flow}},{sec, {flow}},{sec, {flow}}}
        global_rounds_target_flows = {}
        global_rounds_not_sent_out_targetflows = {}
        self.read_global_rounds_target_flows(one_setting_path, global_rounds_target_flows, global_rounds_not_sent_out_targetflows, one_setting_result)
        
        #2. read flow infor for per switch per each round
        #pair: key-switch_id, value-{{sec, {flow info}},{sec, {flow info}},{sec, {flow info}}}
        switches_rounds_flow_info = {}
        self.read_rounds_per_switch_flow_info(one_setting_path, switches_rounds_flow_info, global_rounds_target_flows)
        
        self.calculate_result_method4(global_rounds_target_flows, switches_rounds_flow_info, global_rounds_not_sent_out_targetflows, one_setting_result)
        return one_setting_result

    def read_global_rounds_target_flows(self, one_setting_path, global_rounds_target_flows, global_rounds_not_sent_out_targetflows, one_setting_result):
        sender_path = "{0}/sender" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) milliseconds=====")
        target_flow_pattern = re.compile("^(\d+)\t(\d+)\t(\d+.\d+)\t(\d+)\t([-]*\d+)\t([-]*\d+)")
        raw_host_sample_switch_hold_accuracy = 0
        raw_host_sample_switch_hold_accuracy_cnt = 0
        not_sample_flow_num = 0

        for sender_idx in range(1, CONSTANTS.NUM_SENDER+1):
            sender_fname = "{0}/h{1}_intervals_target_flows.txt" .format(sender_path, sender_idx)
            print("start read {0}" .format(sender_fname))
            # 1. read the file 
            in_file = open(sender_fname, 'r')
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
                    if cur_round_sec not in global_rounds_target_flows:
                        one_round_result = {}
                        global_rounds_target_flows[cur_round_sec] = one_round_result
                        not_sent_map= {}
                        global_rounds_not_sent_out_targetflows[cur_round_sec] = not_sent_map
                else:
                    match = target_flow_pattern.match(line)
                    if match != None:
                        #one target flow
                        srcip = match.group(1)
                        volume = match.group(2)
                        loss_rate = match.group(3)
                        loss_volume = match.group(4)
                        not_sampled_volume = match.group(5)
                        sent_out = int(match.group(6))

                        one_round_result = global_rounds_target_flows[cur_round_sec]
                        one_round_result[srcip] = 1
                        if sent_out < 0:
                            global_rounds_not_sent_out_targetflows[cur_round_sec][srcip] = 1

                        raw_host_sample_switch_hold_accuracy_cnt += 1
                        raw_host_sample_switch_hold_accuracy += (1 - 1.0 * int(not_sampled_volume) / int(volume))
                        if not_sampled_volume == volume:
                            not_sample_flow_num += 1
                            print(not_sample_flow_num)
            print("end read {0}" .format(sender_fname))
        if raw_host_sample_switch_hold_accuracy_cnt > 0:
            raw_host_sample_switch_hold_accuracy /= raw_host_sample_switch_hold_accuracy_cnt
        one_setting_result.raw_host_sample_switch_hold_accuracy = raw_host_sample_switch_hold_accuracy
        print("num_rounds:{0}, max_accuracy_at_host:{1}, min_fn_ratio_at_host:{2}" \
            .format(len(global_rounds_target_flows), one_setting_result.raw_host_sample_switch_hold_accuracy, 
                1.0*not_sample_flow_num/raw_host_sample_switch_hold_accuracy_cnt))
        if len(global_rounds_target_flows) > 0:
            max_target_flow_num_round = 0
            for sec, one_round_result in sorted(global_rounds_target_flows.items(), key=lambda pair: pair[0]):
                if len(one_round_result) > max_target_flow_num_round:
                    max_target_flow_num_round = len(one_round_result)
            for sec, one_round_result in sorted(global_rounds_target_flows.items(), key=lambda pair: pair[0]):
                if len(one_round_result) < max_target_flow_num_round / 2:
                    #this round shouldb the last round
                    del global_rounds_target_flows[sec]
                    continue
                print("one round sec:{0}, target flow num:{1}" .format(sec, len(one_round_result)))

    def read_rounds_per_switch_flow_info(self, one_setting_path, switches_rounds_flow_info, global_rounds_target_flows):
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
            cur_round_sec = 0
            cur_round_flow_num = 0
            line_num = 0
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
                    #print previous round info
                    if cur_round_sec != 0 and cur_round_sec in global_rounds_target_flows:
                        one_switch_one_round_info = one_switch_rounds_info[cur_round_sec]
                        global_target_flow_map = global_rounds_target_flows[cur_round_sec]
                        through_target_flow_num = 0
                        for srcip, flow_info in one_switch_one_round_info.items():
                            if srcip in global_target_flow_map and flow_info.real_volume > 0:
                                #the target flow goes through the switch
                                through_target_flow_num += 1
                        print("sec:{0}, switch:{1}, flow num:{2}, through_target_flow_num:{3}" .format(cur_round_sec, switch_idx, len(one_switch_one_round_info), through_target_flow_num))
                    #new round start
                    cur_round_sec = int(int(match.group(1)) / 1000)
                    one_switch_one_round_info = {}
                    one_switch_rounds_info[cur_round_sec] = one_switch_one_round_info
                    #print("switch:{0}, cur_round_sec:{1}" .format(switch_idx, cur_round_sec))
                    #print("pre interval signed_target_num:{0}" .format(signed_target_num))
                    signed_target_num = 0
                    cur_round_flow_num=0
                    line_num = 0
                else:
                    #print(line)
                    line_num += 1
                    cur_round_flow_num += 1

                    srcip = 0
                    real_volume = 0
                    captured_volume = 0
                    signed_target = 0
                    target_switch_received = 0
                    matched = False
                    match_new = flow_info_pattern_new.match(line)
                    if match_new != None:
                        #one target flow
                        srcip = match_new.group(1)
                        real_volume = int(match_new.group(2))
                        captured_volume = int(match_new.group(3))
                        signed_target = int(match_new.group(4))
                        target_switch_received = int(match_new.group(5))
                        matched = True
                    else:
                        match_old = flow_info_pattern_old.match(line)
                        if match_old != None:
                            srcip = match_old.group(1)
                            real_volume = int(match_old.group(2))
                            captured_volume = int(match_old.group(3))
                            signed_target = int(match_old.group(4))
                            matched = True
                    if matched:
                        flow_info = switch_flow_info_c(real_volume, captured_volume, signed_target, target_switch_received)
                        if cur_round_sec not in one_switch_rounds_info:
                            print("FATAL: switch_idx:{0}, cur_round_sec:{1} not exist in one_switch_rounds_info" \
                                .format(switch_idx, cur_round_sec))
                            continue
                        one_switch_one_round_info = one_switch_rounds_info[cur_round_sec]
                        one_switch_one_round_info[srcip] = flow_info
                        if signed_target == 1 and captured_volume > 0:
                            signed_target_num += 1

            print("end read {0}" .format(switch_fname))

        print("num switches:{0}" .format(len(switches_rounds_flow_info)))
        if len(switches_rounds_flow_info) > 0:
            for switch_idx, one_switch_rounds_info in switches_rounds_flow_info.items():
                for sec, one_switch_one_round_info in sorted(one_switch_rounds_info.items(), key=lambda pair: pair[0]):
                    #print("sec:{0}, switch:{1}, flow num:{2}, signed_target_num:{3}" .format(sec, switch_idx, len(one_switch_one_round_info), signed_target_num))
                    pass

    def calculate_result_method4(self, global_rounds_target_flows, switches_rounds_flow_info, global_rounds_not_sent_out_targetflows, one_setting_result):
        fn_list = []
        accuracy_list = []
        for sec, global_target_flow_map in sorted(global_rounds_target_flows.items(), key=lambda pair:pair[0]):
            if len(global_target_flow_map) < 800:
                print("sec:{0}, len(global_target_flow_map) < 1000" .format(sec))
                continue
            all_target_flow_num = 0
            false_negative_num = 0

            all_accuracy = 0
            all_captured_target_flow_num = 0
            #for each round, calculate FN, accuracy separately.
            for srcip in global_target_flow_map:
                #check the switches the sricp travels through.
                #If all switches the srcip travels through have captured the volume of srcip = > not FN
                #otherwise, = > FN
                is_target_flow_through_switches = False
                is_false_negative = False
                srcip_volume_among_switches = 0
                srcip_captured_volume_among_switches = 0
                for switch_id, switch_rounds_flow_map in switches_rounds_flow_info.items():
                    if sec not in switch_rounds_flow_map:
                        print("round {0} not in switch {1}" .format(sec, switch_id))
                        continue
                    switch_sec_flow_map = switch_rounds_flow_map[sec]
                    if srcip not in switch_sec_flow_map:
                        continue
                    flow_info = switch_sec_flow_map[srcip]
                    if flow_info.real_volume <= 0:
                        #srcip not in this switch
                        continue
                    is_target_flow_through_switches = True
                    if flow_info.captured_volume <= 0:
                        is_false_negative = True
                        break
                    #captured by the switch
                    srcip_volume_among_switches += flow_info.real_volume
                    srcip_captured_volume_among_switches += flow_info.captured_volume
                if not is_target_flow_through_switches:
                    continue
                all_target_flow_num += 1
                if is_false_negative:
                    false_negative_num += 1
                    continue
                all_captured_target_flow_num += 1
                all_accuracy += 1.0 * srcip_captured_volume_among_switches / srcip_volume_among_switches

            #fn ratio and avg accuracy of one round
            false_negative_ratio = 1.0 * false_negative_num / all_target_flow_num
            captured_flows_avg_accuracy = all_accuracy / all_captured_target_flow_num
            fn_list.append(false_negative_ratio)
            accuracy_list.append(captured_flows_avg_accuracy)
            print("sec:{0}, all_target_flow_num:{1}, false_negative_ratio:{2}, captured_flows_avg_accuracy:{3}" .format(sec, all_target_flow_num, false_negative_ratio, captured_flows_avg_accuracy))
            
        #calculate avg value among rounds, and stdv
        one_setting_result.avg_fn = statistics.mean(fn_list)
        one_setting_result.avg_accuracy = statistics.mean(accuracy_list)
        if len(fn_list) > 1:
            one_setting_result.stdv_fn =statistics.stdev(fn_list)
            one_setting_result.stdv_accuracy =statistics.stdev(accuracy_list)

