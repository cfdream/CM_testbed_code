import sys
import re
import statistics

class CONSTANTS():
    NUM_SENDER = 12
    NUM_SWITCH = 12

class switch_flow_info_c():
    def __init__(self, real_volume, captured_volume, signed_target):
        self.real_volume = real_volume
        self.captured_volume = captured_volume
        self.signed_target = signed_target

class switch_one_round_result_c():
    def __init__(self, fn, fp, accuracy):
        self.fn = fn
        self.fp = fp
        self.accuracy = accuracy
        self.sample_map_size = 0
        self.condition_map_size = 0

class avg_switch_result_one_setting_c():
    def __init__(self):
        self.avg_fn = 0
        self.stdv_fn = 0
        self.avg_fp = 0
        self.stdv_fp = 0
        self.avg_accuracy = 0
        self.stdv_accuracy = 0
        self.sample_map_size = 0
        self.condition_map_size = 0

class one_setting_result_c():
    def __init__(self):
        self.avg_fn = 0
        self.min_fn = 0
        self.max_fn = 0
        self.stdv_fn = 0
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
        self.max_condition_map_size = 0
        self.max_condition_map_size = 0

class one_setting_result_calculator_c():
    def __init__(self):
        #setting
        self.host_switch_sample = 0
        self.replace = 0
        self.memory_type = 0
        self.freq = 0
        self.switches_sample_map_size = [None] * (CONSTANTS.NUM_SWITCH+1)
        self.switches_condition_map_size = [None] * (CONSTANTS.NUM_SWITCH+1)
    
    def get_one_setting_result(self, one_setting_path):
        print(one_setting_path)
        #1. read all target flows for each round
        #format: {{sec, {flow}},{sec, {flow}},{sec, {flow}}}
        global_rounds_target_flows = {}
        self.read_global_rounds_target_flows(one_setting_path, global_rounds_target_flows)
        
        #2. read flow infor for per switch per each round
        #pair: key-switch_id, value-{{sec, {flow info}},{sec, {flow info}},{sec, {flow info}}}
        switches_rounds_flow_info = {}
        self.read_rounds_per_switch_flow_info(one_setting_path, switches_rounds_flow_info)

        #3. calculate per switch per round result
        print("START calculate_rounds_per_switch_result()")
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        switches_rounds_result = {}     #switches_rounds_result_c
        self.calculate_rounds_per_switch_result(global_rounds_target_flows, switches_rounds_flow_info, switches_rounds_result)
        print("END calculate_rounds_per_switch_result()")

        #4. calculate one_settig_result
        print("START calculate_switches_one_setting_result()")
        #pair: key-switch_id, value-avg_switch_result_one_setting_c
        avg_switches_result_one_setting = {}    #avg_switch_result_one_setting_c
        self.calculate_switches_one_setting_result(switches_rounds_result, avg_switches_result_one_setting)
        print("END calculate_switches_one_setting_result()")

        #5. calculate_one_setting_result
        print("START calculate_one_setting_result()")
        one_setting_result = one_setting_result_c()
        self.calculate_one_setting_result(avg_switches_result_one_setting, one_setting_result)
        print("END calculate_one_setting_result()")

        return one_setting_result

    def read_global_rounds_target_flows(self, one_setting_path, global_rounds_target_flows):
        sender_path = "{0}/sender" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) seconds=====")
        target_flow_pattern = re.compile("^(\d+)\t(\d+)\t(\d+.\d+)\t(\d+)")
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
                    cur_round_sec = match.group(1)
                    if cur_round_sec not in global_rounds_target_flows:
                        one_round_result = {}
                        global_rounds_target_flows[cur_round_sec] = one_round_result
                else:
                    match = target_flow_pattern.match(line)
                    if match != None:
                        #one target flow
                        srcip = match.group(1)
                        volume = match.group(2)
                        loss_rate = match.group(3)
                        loss_volume = match.group(4)

                        one_round_result = global_rounds_target_flows[cur_round_sec]
                        one_round_result[srcip] = 1
        print("num_rounds:{0}" .format(len(global_rounds_target_flows)))
        if len(global_rounds_target_flows) > 0:
            for sec, one_round_result in global_rounds_target_flows.items():
                print("one round target flow num:{0}" .format(len(one_round_result)))
                break
        print("end read {0}" .format(sender_fname))

    def read_rounds_per_switch_flow_info(self, one_setting_path, switches_rounds_flow_info):
        switch_path = "{0}/switch" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) seconds=====")
        flow_info_pattern = re.compile("^(\d+)\t(\d+)\t(\d+)\t([-]*\d+)")
        sample_map_size_pattern = re.compile("^sample_hashmap_size:(\d+)")
        condition_map_size_pattern = re.compile("^condition_hashmap_size:(\d+)")
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
            signed_target_num = 0
            for line in lines:
                #get memory sizes
                match = sample_map_size_pattern.match(line)
                if match != None:
                    self.switches_sample_map_size[switch_idx] = int(match.group(1))
                match = condition_map_size_pattern.match(line)
                if match != None:
                    self.switches_condition_map_size[switch_idx] = int(match.group(1))

                match = round_start_pattern.match(line)
                if match != None:
                    #new round start
                    cur_round_sec = match.group(1)
                    one_switch_one_round_info = {}
                    one_switch_rounds_info[cur_round_sec] = one_switch_one_round_info
                    #print("switch:{0}, cur_round_sec:{1}" .format(switch_idx, cur_round_sec))
                    #print("pre interval signed_target_num:{0}" .format(signed_target_num))
                    signed_target_num = 0
                else:
                    match = flow_info_pattern.match(line)
                    if match != None:
                        #print(line)
                        #one target flow
                        srcip = match.group(1)
                        real_volume = int(match.group(2))
                        captured_volume = int(match.group(3))
                        signed_target = int(match.group(4))
                        flow_info = switch_flow_info_c(real_volume, captured_volume, signed_target)
                        if cur_round_sec not in one_switch_rounds_info:
                            print("FATAL: switch_idx:{0}, cur_round_sec:{1} not exist in one_switch_rounds_info" \
                                .format(switch_idx, cur_round_sec))
                            continue
                        one_switch_one_round_info = one_switch_rounds_info[cur_round_sec]
                        one_switch_one_round_info[srcip] = flow_info
                        if signed_target == 1:
                            signed_target_num += 1

            print("end read {0}" .format(switch_fname))
        print("num switches:{0}" .format(len(switches_rounds_flow_info)))
        if len(switches_rounds_flow_info) > 0:
            for switch_idx, one_switch_rounds_info in switches_rounds_flow_info.items():
                for sec, one_switch_one_round_info in one_switch_rounds_info.items():
                    print("one switch flow num:{0}" .format(len(one_switch_one_round_info)))
                    break

    def calculate_rounds_per_switch_result(self, global_rounds_target_flows, switches_rounds_flow_info, switches_rounds_result):
        #switches_rounds_result
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        #switches_rounds_flow_info = {}
        #pair: key-switch_id, value-{{sec, {flow info}},{sec, {flow info}},{sec, {flow info}}}
        for switch_id, switch_rounds_flow_map in switches_rounds_flow_info.items():
            #1. for each switch
            one_switch_rounds_map = {}
            switches_rounds_result[switch_id] = one_switch_rounds_map
            for sec, switch_flow_info_map in switch_rounds_flow_map.items():
                #2. for each round
                #global_rounds_target_flows = {}
                #format: {{sec, {flow}},{sec, {flow}},{sec, {flow}}}
                if sec not in global_rounds_target_flows: 
                    #one_setting experiment has not started yet.
                    continue
                global_target_flow_map = global_rounds_target_flows[sec]
                
                #2.1. FNR
                #switch_flow_info_map = {}
                #pair: key-srcip, value-switch_flow_info_c
                all_flow_num  = len(switch_flow_info_map)
                real_target_flow_num = 0
                false_negative_num = 0
                #print("all_flow_num:{0}" .format(all_flow_num))
                for srcip, flow_info in switch_flow_info_map.items():
                    if srcip in global_target_flow_map:
                        #the target flow goes through the switch
                        real_target_flow_num += 1
                        if flow_info.captured_volume < 0 or flow_info.signed_target < 0:
                            #not captured flow
                            false_negative_num += 1
                false_negative_ratio=0.0
                if real_target_flow_num > 0:
                    false_negative_ratio = 1.0 * false_negative_num / real_target_flow_num 
                #print("real_target_flow_num:{0}" .format(real_target_flow_num))
                #print("false_negative_num:{0}" .format(false_negative_num))
                #print("false_negative_ratio:{0}" .format(false_negative_ratio))
                            
                #2.2. FPR
                not_target_flow_num = all_flow_num - real_target_flow_num
                false_positive_num = 0
                for srcip, flow_info in switch_flow_info_map.items():
                    if (srcip not in global_target_flow_map) \
                        and (flow_info.captured_volume > 0) \
                        and (flow_info.signed_target > 0):
                        false_positive_num += 1
                false_positive_ratio = 0
                if not_target_flow_num > 0:
                    false_positive_ratio = 1.0 * false_positive_num / not_target_flow_num
                
                #2.3. accuracy
                all_to_report_target_flow_accuracy = 0
                all_to_report_target_flow_num = 0
                for srcip, flow_info in switch_flow_info_map.items():
                    if srcip in global_target_flow_map \
                        and (flow_info.captured_volume > 0) \
                        and (flow_info.signed_target > 0):
                        one_flow_accuracy = (1.0 * flow_info.captured_volume / flow_info.real_volume)
                        all_to_report_target_flow_accuracy += one_flow_accuracy
                        all_to_report_target_flow_num += 1
                        #print("srcip:{0}, real_v:{1}, captured_v:{2}, accuracy:{3}" \
                        #    .format(srcip, flow_info.real_volume, flow_info.captured_volume, one_flow_accuracy))
                accuracy = 0
                if all_to_report_target_flow_num > 0:
                    accuracy = all_to_report_target_flow_accuracy / all_to_report_target_flow_num
  
                #2.4 one round result of the switch
                one_result = switch_one_round_result_c(false_negative_ratio, false_positive_ratio, accuracy)
                switches_rounds_result[switch_id][sec] = one_result
                #print("fn-fp-accuracy:{0}-{1}-{2}" .format(one_result.fn, one_result.fp, one_result.accuracy))

    def calculate_switches_one_setting_result(self, switches_rounds_result, avg_switches_result_one_setting):
        #switches_rounds_result
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        for switch_id, rounds_result_map in switches_rounds_result.items():
            avg_switch_one_setting = avg_switch_result_one_setting_c()
            avg_switches_result_one_setting[switch_id] = avg_switch_one_setting
            avg_switch_one_setting.sample_map_size = self.switches_sample_map_size[switch_id]
            avg_switch_one_setting.condition_map_size = self.switches_condition_map_size[switch_id]

            fn_list = []
            fp_list = []
            accuracy_list = []
            for sec, one_round_result in rounds_result_map.items():
                #print("fn-fp-accuracy:{0}-{1}-{2}" .format(one_round_result.fn, one_round_result.fp, one_round_result.accuracy))
                fn_list.append(one_round_result.fn)
                fp_list.append(one_round_result.fp)
                accuracy_list.append(one_round_result.accuracy)
            avg_switch_one_setting.avg_fn = statistics.mean(fn_list)
            avg_switch_one_setting.avg_fp = statistics.mean(fp_list)
            avg_switch_one_setting.avg_accuracy = statistics.mean(accuracy_list)
            if len(rounds_result_map) > 1:
                avg_switch_one_setting.stdv_fn =statistics.stdev(fn_list)
                avg_switch_one_setting.stdv_fp =statistics.stdev(fp_list)
                avg_switch_one_setting.stdv_accuracy =statistics.stdev(accuracy_list)
    def calculate_one_setting_result(self, avg_switches_result_one_setting, one_setting_result):
        #avg_switches_result_one_setting = {}
        #format:key-switch_id, value-avg_switch_result_one_setting_c
        fn_list = []
        fp_list = []
        accuracy_list = []
        sample_map_size_list = []
        condition_map_size_list = []
        for switch_id, avg_switch_one_setting in avg_switches_result_one_setting.items():
            fn_list.append(avg_switch_one_setting.avg_fn)
            fp_list.append(avg_switch_one_setting.avg_fp)
            accuracy_list.append(avg_switch_one_setting.avg_accuracy)
            sample_map_size_list.append(avg_switch_one_setting.sample_map_size)
            condition_map_size_list.append(avg_switch_one_setting.condition_map_size)

        one_setting_result.avg_fn = statistics.mean(fn_list)
        one_setting_result.min_fn = min(fn_list)
        one_setting_result.max_fn = max(fn_list)
        one_setting_result.stdv_fn =statistics.stdev(fn_list)
        one_setting_result.avg_fp = statistics.mean(fp_list)
        one_setting_result.min_fp = min(fp_list)
        one_setting_result.max_fp = max(fp_list)
        one_setting_result.stdv_fp =statistics.stdev(fp_list)
        one_setting_result.avg_accuracy = statistics.mean(accuracy_list)
        one_setting_result.min_accuracy = min(accuracy_list)
        one_setting_result.max_accuracy = max(accuracy_list)
        one_setting_result.stdv_accuracy =statistics.stdev(accuracy_list)
        one_setting_result.avg_sample_map_size = statistics.mean(sample_map_size_list)
        one_setting_result.min_sample_map_size = min(sample_map_size_list)
        one_setting_result.max_sample_map_size = max(sample_map_size_list)
        one_setting_result.avg_condition_map_size = statistics.mean(condition_map_size_list)
        one_setting_result.min_condition_map_size = min(condition_map_size_list)
        one_setting_result.max_condition_map_size = max(condition_map_size_list)

