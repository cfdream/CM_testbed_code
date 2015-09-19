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
    def __init__(self, all_volume_one_interval, all_flow_num_one_interval, real_target_flow_num, fn, fp, accuracy, fn_num, fn_num_not_targetflow, fn_num_not_captured, condition_pkt_num):
        self.all_volume_one_interval = all_volume_one_interval
        self.all_flow_num_one_interval = all_flow_num_one_interval
        self.real_target_flow_num = real_target_flow_num
        self.fn = fn
        self.fp = fp
        self.accuracy = accuracy
        self.fn_num = fn_num
        self.fn_num_not_targetflow = fn_num_not_targetflow
        self.fn_num_not_captured  = fn_num_not_captured
        self.sample_map_size = 0
        #self.condition_map_size = 0
        self.condition_pkt_num = condition_pkt_num

class avg_switch_result_one_setting_c():
    def __init__(self):
        self.avg_fn = 0
        self.stdv_fn = 0
        self.avg_fp = 0
        self.stdv_fp = 0
        self.avg_accuracy = 0
        self.stdv_accuracy = 0
        self.avg_condition_pkt_num = 0
        self.stdv_condition_pkt_num = 0
        self.sample_map_size = 0
        #self.condition_map_size = 0
        self.avg_all_volume_one_interval = 0
        self.avg_all_flow_num_one_interval = 0
        self.avg_real_target_flow_num = 0
        self.avg_fn_num = 0
        self.avg_fn_num_not_targetflow = 0
        self.avg_fn_num_not_captured = 0
        self.avg_sample_map_size = 0
        self.raw_host_sample_switch_hold_accuracy = 0

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
        #self.avg_condition_map_size = 0
        #self.min_condition_map_size = 0
        #self.max_condition_map_size = 0
        self.raw_host_sample_switch_hold_accuracy = 0

class one_setting_result_calculator_c():
    ALL_SWITCHES=0
    #CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY = "CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY"
    def __init__(self):
        #setting
        self.host_switch_sample = 0
        self.replace = 0
        self.memory_type = 0
        self.freq = 0
        self.switches_sample_map_size = [0] * (CONSTANTS.NUM_SWITCH+1)
        self.rounds_allswitches_condition_pkt_num = {}
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
        self.read_rounds_per_switch_flow_info(one_setting_path, switches_rounds_flow_info)

        #3. calculate per switch per round result
        print("START calculate_rounds_per_switch_result()")
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        switches_rounds_result = {}     #switches_rounds_result_c
        self.calculate_rounds_per_switch_result(global_rounds_target_flows, switches_rounds_flow_info, global_rounds_not_sent_out_targetflows, switches_rounds_result)
        print("END calculate_rounds_per_switch_result()")

        #4. calculate one_setting_result for every switch
        print("START calculate_switches_one_setting_result()")
        #pair: key-switch_id, value-avg_switch_result_one_setting_c
        avg_switches_result_one_setting = {}    #avg_switch_result_one_setting_c
        self.calculate_switches_one_setting_result(switches_rounds_result, avg_switches_result_one_setting)
        print("END calculate_switches_one_setting_result()")
        avg_switches_result_one_setting[one_setting_result_calculator_c.ALL_SWITCHES].avg_sample_map_size  = \
            statistics.mean(self.switches_sample_map_size[1:])
        return one_setting_result, avg_switches_result_one_setting[one_setting_result_calculator_c.ALL_SWITCHES]

        #5. calculate_one_setting_result
        #print("START calculate_one_setting_result()")
        #self.calculate_one_setting_result(avg_switches_result_one_setting, one_setting_result)
        #print("END calculate_one_setting_result()")

        #return one_setting_result

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
                #print("one round sec:{0}, target flow num:{1}" .format(sec, len(one_round_result)))

    def read_rounds_per_switch_flow_info(self, one_setting_path, switches_rounds_flow_info):
        switch_path = "{0}/switch" .format(one_setting_path)
        round_start_pattern = re.compile("=====time-(\d+) milliseconds=====")
        #new format
        flow_info_pattern_new = re.compile("^(\d+)\t(\d+)\t([-]*\d+)\t([-]*\d+)\t(\d+)")
        #old format
        flow_info_pattern_old = re.compile("^(\d+)\t(\d+)\t([-]*\d+)\t([-]*\d+)")
        sample_map_size_pattern = re.compile("^sample_hashmap_size:(\d+)")
        condition_map_size_pattern = re.compile("^condition_hashmap_size:(\d+)")
        condition_map_last_round_collision_num = re.compile("^condition_hashmap last rotate collision times:(\d+)")
        condition_pkt_received_pattern = re.compile('^condition_pkt_num_rece:(\d+)')

        one_switch_rounds_info = {}
        switches_rounds_flow_info[one_setting_result_calculator_c.ALL_SWITCHES] = one_switch_rounds_info
        for switch_idx in range(1, CONSTANTS.NUM_SWITCH+1):
            switch_fname = "{0}/s{1}.result" .format(switch_path, switch_idx)
            print("start read {0}" .format(switch_fname))
            # 0. one new switch
            # 1. read the file 
            in_file = open(switch_fname, 'r')
            lines = in_file.readlines()
            in_file.close()
            #2. get per round flow info for the switch
            cur_round_sec = 0
            cur_round_flow_num = 0
            line_num = 0
            signed_target_num = 0
            condition_pkt_num = 0
            sample_map_size = 0
            for line in lines:
                #get memory sizes
                match = sample_map_size_pattern.match(line)
                if match != None:
                    self.switches_sample_map_size[switch_idx] = int(match.group(1))
                match = condition_pkt_received_pattern.match(line)
                if match != None:
                    condition_pkt_num = int(match.group(1))
                    if cur_round_sec not in self.rounds_allswitches_condition_pkt_num:
                        self.rounds_allswitches_condition_pkt_num[cur_round_sec] = 0
                    self.rounds_allswitches_condition_pkt_num[cur_round_sec] += condition_pkt_num 
                #match = condition_map_size_pattern.match(line)
                #if match != None:
                #    self.switches_condition_map_size[one_setting_result_calculator_c.ALL_SWITCHES] = int(match.group(1))
                #match = condition_map_last_round_collision_num.match(line)
                #if match != None:
                #    one_switch_rounds_info[cur_round_sec][one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY] \
                #        = int(match.group(1))
                match = round_start_pattern.match(line)
                if match != None:
                    #print previous round info
                    #print("sec:{0}, cur_round_flow_num:{1}, line_num:{2}" .format(cur_round_sec, cur_round_flow_num, line_num))
                    #new round start
                    cur_round_sec = int(int(match.group(1)) / 1000)
                    if cur_round_sec not in one_switch_rounds_info:
                        one_switch_one_round_info = {}
                        one_switch_rounds_info[cur_round_sec] = one_switch_one_round_info
                    #print("switch:{0}, cur_round_sec:{1}" .format(one_setting_result_calculator_c.ALL_SWITCHES, cur_round_sec))
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
                        if cur_round_sec not in one_switch_rounds_info:
                            print("FATAL: switch_idx:{0}, cur_round_sec:{1} not exist in one_switch_rounds_info" \
                                .format(switch_idx, cur_round_sec))
                            continue
                        one_switch_one_round_info = one_switch_rounds_info[cur_round_sec]
                        flow_info = None
                        if srcip in one_switch_one_round_info:
                            flow_info = one_switch_one_round_info[srcip]
                            flow_info.real_volume += real_volume
                            if flow_info.signed_target:
                                flow_info.captured_volume += captured_volume
                            flow_info.signed_target |= signed_target
                            flow_info.target_switch_received |= target_switch_received
                        else:
                            flow_info = switch_flow_info_c(real_volume, captured_volume, signed_target, target_switch_received)
                        one_switch_one_round_info[srcip] = flow_info
                        if signed_target == 1:
                            signed_target_num += 1
            print("end read {0}" .format(switch_fname))

        print("num switches:{0}" .format(len(switches_rounds_flow_info)))
        if len(switches_rounds_flow_info) > 0:
            for switch_idx, one_switch_rounds_info in switches_rounds_flow_info.items():
                for sec, one_switch_one_round_info in sorted(one_switch_rounds_info.items(), key=lambda pair: pair[0]):
                    #print("sec:{0}, switch:{1}, flow num:{2}" .format(sec, switch_idx, len(one_switch_one_round_info)))
                    pass
                    

    def calculate_rounds_per_switch_result(self, global_rounds_target_flows, switches_rounds_flow_info, global_rounds_not_sent_out_targetflows, switches_rounds_result):
        #switches_rounds_result
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        #switches_rounds_flow_info = {}
        #pair: key-switch_id, value-{{sec, {flow info}},{sec, {flow info}},{sec, {flow info}}}
        for switch_id, switch_rounds_flow_map in switches_rounds_flow_info.items():
            #1. for each switch
            one_switch_rounds_map = {}
            switches_rounds_result[switch_id] = one_switch_rounds_map
            for sec, switch_flow_info_map in sorted(switch_rounds_flow_map.items(), key=lambda pair: pair[0]):
                round_allswitches_condition_pktnum = self.rounds_allswitches_condition_pkt_num[sec]
                #2. for each round
                #global_rounds_target_flows = {}
                #format: {{sec, {flow}},{sec, {flow}},{sec, {flow}}}
                if sec not in global_rounds_target_flows: 
                    #one_setting experiment has not started yet.
                    continue
                global_target_flow_map = global_rounds_target_flows[sec]
                global_not_sent_out_targetflow_map = global_rounds_not_sent_out_targetflows[sec]
                
                #2.1. FNR
                #switch_flow_info_map = {}
                #pair: key-srcip, value-switch_flow_info_c
                all_flow_num_one_interval  = len(switch_flow_info_map)
                all_volume_one_interval = 0
                real_target_flow_num = 0
                false_negative_num = 0
                fn_num_not_targetflow = 0
                fn_num_not_captured = 0
                fn_num_not_sent_out_at_sender = 0
                fn_num_received_and_hashmap_collision = 0
                fn_num_sent_not_receive = 0
                #fn_num_condition_map_last_round_collision = \
                #    switch_flow_info_map[one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY]
                #print("all_flow_num_one_interval:{0}" .format(all_flow_num_one_interval))
                for srcip, flow_info in switch_flow_info_map.items():
                    #if srcip == one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY:
                    #    continue
                    all_volume_one_interval += flow_info.real_volume
                    if srcip in global_target_flow_map:
                        #the target flow goes through the switch
                        real_target_flow_num += 1
                        if flow_info.signed_target <= 0:
                            fn_num_not_targetflow += 1
                            if srcip in global_not_sent_out_targetflow_map:
                                fn_num_not_sent_out_at_sender += 1
                            elif flow_info.target_switch_received == 1:
                                fn_num_received_and_hashmap_collision += 1
                            else:
                                fn_num_sent_not_receive += 1
                        if flow_info.captured_volume <= 0:
                            fn_num_not_captured += 1
                        if flow_info.captured_volume <= 0 or flow_info.signed_target <= 0:
                            #not captured flow
                            false_negative_num += 1
                #fn_num_sent_out_not_receive = \
                #    fn_num_sent_not_receive - \
                #    fn_num_condition_map_last_round_collision
                false_negative_ratio=0.0
                if real_target_flow_num > 0:
                    false_negative_ratio = 1.0 * false_negative_num / real_target_flow_num 
                            
                #2.2. FPR
                not_target_flow_num = all_flow_num_one_interval - real_target_flow_num
                false_positive_num = 0
                for srcip, flow_info in switch_flow_info_map.items():
                    #if srcip == one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY:
                    #    continue
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
                    #if srcip == one_setting_result_calculator_c.CONDITION_MAP_LAST_ROTATE_COLISSION_TIMES_KEY:
                    #    continue
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
 
                if all_to_report_target_flow_num >= 10:
                    # 2.4 one round result of the switch
                    # If for one switch, the reported target flow number in one round < 10, means the output is not complete due to time cut,
                    # In this case, the result of this round is not meaningfule
                    one_result = switch_one_round_result_c(all_volume_one_interval, all_flow_num_one_interval, real_target_flow_num, \
                            false_negative_ratio, false_positive_ratio, accuracy, \
                            false_negative_num, fn_num_not_targetflow, fn_num_not_captured, \
                            round_allswitches_condition_pktnum)
                    switches_rounds_result[switch_id][sec] = one_result
                
                    #2.5 print debug log
                    print("switch_id:{switch_id}, sec:{secc}, traffic:{all_volume}Mbyte, #flow:{all_flow_num}, #target_flow:{target_flow_num}, #bucket:{sample_map_size}, accuracy:{acc}, fn:{fn}, fp:{fp}, #fn:{fn_num}, #fn_not_targetflow:{fn_num_not_targetflow}(not_sent_accurate-{not_sent}, not_receive:{not_rece}, rece_but_collision:{rece_collision}), #fn_not_captured:{fn_num_not_captured}" \
                            .format(switch_id = switch_id, \
                                secc=sec, \
                                all_volume = int(all_volume_one_interval/1000000), \
                                all_flow_num = all_flow_num_one_interval, \
                                sample_map_size = self.switches_sample_map_size[switch_id], \
                                fn=round(false_negative_ratio, 4), \
                                fp=round(false_positive_ratio, 4), \
                                acc=round(accuracy, 4), \
                                target_flow_num = real_target_flow_num, \
                                fn_num = false_negative_num,\
                                fn_num_not_targetflow = fn_num_not_targetflow, \
                                fn_num_not_captured = fn_num_not_captured, \
                                not_sent = fn_num_not_sent_out_at_sender, \
                                not_rece = fn_num_sent_not_receive, \
                                rece_collision = fn_num_received_and_hashmap_collision\
                            ))
                else:
                    print("WARNING: switch_id:{switch_id} has all_to_report_target_flow_num < 10" .format(switch_id=switch_id))

    def calculate_switches_one_setting_result(self, switches_rounds_result, avg_switches_result_one_setting):
        #switches_rounds_result
        #pair: key-switch_id, value-{{sec, switch_one_round_result_c},{sec, switch_one_round_result_c}}
        switches_avg_real_targetflow_num = []
        for switch_id, rounds_result_map in switches_rounds_result.items():
            avg_switch_one_setting = avg_switch_result_one_setting_c()
            avg_switches_result_one_setting[switch_id] = avg_switch_one_setting
            #avg_switch_one_setting.condition_map_size = self.switches_condition_map_size[switch_id]

            fn_list = []
            fp_list = []
            accuracy_list = []
            all_volume_list = []
            all_flow_num_list = []
            real_target_flow_num_list = []
            fn_num_list = []
            fn_num_not_targetflow_list = []
            fn_num_not_captured_list = []
            condition_pkt_num_list = []
            for sec, one_round_result in rounds_result_map.items():
                #print("fn-fp-accuracy:{0}-{1}-{2}" .format(one_round_result.fn, one_round_result.fp, one_round_result.accuracy))
                fn_list.append(one_round_result.fn)
                fp_list.append(one_round_result.fp)
                accuracy_list.append(one_round_result.accuracy)
                all_volume_list.append(one_round_result.all_volume_one_interval)
                all_flow_num_list.append(one_round_result.all_flow_num_one_interval)
                real_target_flow_num_list.append(one_round_result.real_target_flow_num)
                fn_num_list.append(one_round_result.fn_num)
                fn_num_not_targetflow_list.append(one_round_result.fn_num_not_targetflow)
                fn_num_not_captured_list.append(one_round_result.fn_num_not_captured)
                condition_pkt_num_list.append(one_round_result.condition_pkt_num)

            avg_switch_one_setting.avg_fn = statistics.mean(fn_list)
            avg_switch_one_setting.avg_fp = statistics.mean(fp_list)
            avg_switch_one_setting.avg_accuracy = statistics.mean(accuracy_list)
            avg_switch_one_setting.avg_real_target_flow_num = statistics.mean(real_target_flow_num_list)
            switches_avg_real_targetflow_num.append(round(avg_switch_one_setting.avg_real_target_flow_num, 1))
            avg_switch_one_setting.avg_fn_num = statistics.mean(fn_num_list)
            avg_switch_one_setting.avg_fn_num_not_targetflow = statistics.mean(fn_num_not_targetflow_list)
            avg_switch_one_setting.avg_fn_num_not_captured = statistics.mean(fn_num_not_captured_list)
            avg_switch_one_setting.avg_condition_pkt_num = statistics.mean(condition_pkt_num_list)
            if len(rounds_result_map) > 1:
                avg_switch_one_setting.stdv_fn =statistics.stdev(fn_list)
                avg_switch_one_setting.stdv_fp =statistics.stdev(fp_list)
                avg_switch_one_setting.stdv_accuracy =statistics.stdev(accuracy_list)
                avg_switch_one_setting.stdv_condition_pkt_num = statistics.stdev(condition_pkt_num_list)
            print("switch_id:{switch_id}, #interval:{num_interval}, traffic:{all_volume}Mbyte, #flow:{all_flow_num}, #target_flow:{target_flow_num}, #buckets:{avg_sample_map_size}, avg_fn:{fn}(stdv:{fn_stdv}), avg_accuracy:{accuracy}(stdv:{acc_stdv})" \
                .format( \
                    switch_id=switch_id, \
                    num_interval = len(rounds_result_map), \
                    all_volume = int(sum(all_volume_list)/1000000), \
                    all_flow_num = sum(all_flow_num_list), \
                    target_flow_num = round(avg_switch_one_setting.avg_real_target_flow_num, 1), \
                    avg_sample_map_size = avg_switch_one_setting.avg_sample_map_size, \
                    fn=round(avg_switch_one_setting.avg_fn, 4), \
                    fn_stdv = round(avg_switch_one_setting.stdv_fn, 4), \
                    accuracy=round(avg_switch_one_setting.avg_accuracy, 4), \
                    acc_stdv = round(avg_switch_one_setting.stdv_accuracy, 4) \
                ))

    def calculate_one_setting_result(self, avg_switches_result_one_setting, one_setting_result):
        #avg_switches_result_one_setting = {}
        #format:key-switch_id, value-avg_switch_result_one_setting_c
        fn_list = []
        fp_list = []
        accuracy_list = []
        sample_map_size_list = []
        #condition_map_size_list = []

        real_target_flow_num_list = []
        fn_num_list = []
        fn_num_not_targetflow_list = []
        fn_num_not_captured_list = []

        for switch_id, avg_switch_one_setting in avg_switches_result_one_setting.items():
            fn_list.append(avg_switch_one_setting.avg_fn)
            fp_list.append(avg_switch_one_setting.avg_fp)
            accuracy_list.append(avg_switch_one_setting.avg_accuracy)
            sample_map_size_list.append(avg_switch_one_setting.sample_map_size)
            #condition_map_size_list.append(avg_switch_one_setting.condition_map_size)
            real_target_flow_num_list.append(avg_switch_one_setting.avg_real_target_flow_num)
            fn_num_list.append(avg_switch_one_setting.avg_fn_num)
            fn_num_not_targetflow_list.append(avg_switch_one_setting.avg_fn_num_not_targetflow)
            fn_num_not_captured_list.append(avg_switch_one_setting.avg_fn_num_not_captured)

        one_setting_result.avg_fn = statistics.mean(fn_list)
        one_setting_result.min_fn = min(fn_list)
        one_setting_result.max_fn = max(fn_list)
        one_setting_result.avg_real_target_flow_num = statistics.mean(real_target_flow_num_list)
        one_setting_result.avg_fn_num = statistics.mean(fn_num_list)
        one_setting_result.avg_fn_num_not_targetflow = statistics.mean(fn_num_not_targetflow_list)
        one_setting_result.avg_fn_num_not_captured = statistics.mean(fn_num_not_captured_list)
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
        #one_setting_result.avg_condition_map_size = statistics.mean(condition_map_size_list)
        #one_setting_result.min_condition_map_size = min(condition_map_size_list)
        #one_setting_result.max_condition_map_size = max(condition_map_size_list)

