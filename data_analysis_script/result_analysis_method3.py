#!/usr/bin/python

import time
import re
import os
import sys
from one_setting_result_calculator_method3 import one_setting_result_calculator_c

class results_analyzer_c():
    def get_all_setting_result(self, result_path):
        for inject_or_tag_packet in [0]:
            for host_switch_sample in [0]:
                for replace in [1]:
                    for memory_type in [0]:
                        #for memory_times in [1, 2, 4, 8, 16, 32]:
                        for memory_times in [1]:
                            for freq in [500, 1000, 2000, 4000, 8000, 16000]:
                                #one setting result path
                                one_setting_path = '{0}/sample_{1}_replace_{2}_mem_{3}_mem_times_{4}_freq_{5}_tag_{6}' .format(result_path, host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
                                if not os.path.lexists(one_setting_path):
                                    continue
                                #get the results of one setting
                                one_setting_result_calculator = one_setting_result_calculator_c()
                                one_setting_result, all_switches_result_one_setting = one_setting_result_calculator.get_one_setting_result(one_setting_path)
                                #output the results of one setting
                                self.new_output_one_setting_result(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet, all_switches_result_one_setting, one_setting_result)

    def new_output_one_setting_result(self, host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet, all_switches_result_one_setting, one_setting_result):
        with open('result_analysis_method3_result.num', 'a') as out_file:
            #setting info
            out_file.write("{host_switch_sample}\t{replace}\t{memory_type}\t{memory_times}\t{freq}\t{inject_or_tag_packet}\t{avg_fn}\t{stdv_fn}\t{avg_fp}\t{stdv_fp}\t{avg_accuracy}\t{stdv_accuracy}\t{avg_sample_map_size}\t{raw_host_sample_switch_hold_accuracy}\t{avg_real_target_flow_num}\t{avg_fn_num}\t{avg_condition_pkt_num}\n" \
            .format(host_switch_sample=host_switch_sample, \
                replace=replace, memory_type=memory_type, memory_times=memory_times, freq=freq, \
                inject_or_tag_packet=inject_or_tag_packet, \
                avg_fn = all_switches_result_one_setting.avg_fn, \
                stdv_fn = all_switches_result_one_setting.stdv_fn, \
                avg_fp = all_switches_result_one_setting.avg_fp, \
                stdv_fp = all_switches_result_one_setting.stdv_fp, \
                avg_accuracy = all_switches_result_one_setting.avg_accuracy, \
                stdv_accuracy = all_switches_result_one_setting.stdv_accuracy, \
                avg_sample_map_size = all_switches_result_one_setting.avg_sample_map_size, \
                raw_host_sample_switch_hold_accuracy = one_setting_result.raw_host_sample_switch_hold_accuracy, \
                avg_real_target_flow_num = all_switches_result_one_setting.avg_real_target_flow_num, \
                avg_fn_num = all_switches_result_one_setting.avg_fn_num, \
                avg_condition_pkt_num = all_switches_result_one_setting.avg_condition_pkt_num
            )) 
        with open('result_analysis_method3_result.txt', 'a') as out_file:
            #setting info
            out_file.write("host_switch_sample={host_switch_sample}\treplace={replace}\tmemory_type={memory_type}\tmemory_times={memory_times}\tfreq={freq}\tinject_or_tag_packet={inject_or_tag_packet}\tavg_fn={avg_fn}\tstdv_fn={stdv_fn}\tavg_fp={avg_fp}\tstdv_fp={stdv_fp}\tavg_accuracy={avg_accuracy}\tstdv_accuracy={stdv_accuracy}\tavg_sample_map_size={avg_sample_map_size}\traw_host_sample_switch_hold_accuracy={raw_host_sample_switch_hold_accuracy}\tavg_real_target_flow_num={avg_real_target_flow_num}\tavg_fn_num={avg_fn_num}\tavg_condition_pkt_num={avg_condition_pkt_num}\n" \
            .format(host_switch_sample=host_switch_sample, \
                replace=replace, memory_type=memory_type, memory_times=memory_times, freq=freq, \
                inject_or_tag_packet=inject_or_tag_packet, \
                avg_fn = all_switches_result_one_setting.avg_fn, \
                stdv_fn = all_switches_result_one_setting.stdv_fn, \
                avg_fp = all_switches_result_one_setting.avg_fp, \
                stdv_fp = all_switches_result_one_setting.stdv_fp, \
                avg_accuracy = all_switches_result_one_setting.avg_accuracy, \
                stdv_accuracy = all_switches_result_one_setting.stdv_accuracy, \
                avg_sample_map_size = all_switches_result_one_setting.avg_sample_map_size, \
                raw_host_sample_switch_hold_accuracy = one_setting_result.raw_host_sample_switch_hold_accuracy, \
                avg_real_target_flow_num = all_switches_result_one_setting.avg_real_target_flow_num, \
                avg_fn_num = all_switches_result_one_setting.avg_fn_num, \
                avg_condition_pkt_num = all_switches_result_one_setting.avg_condition_pkt_num
            )) 

if __name__ == '__main__':
    if len(sys.argv) !=2:
        print("usage: python3 result_analysis.py result_path")
        exit(0)

    result_path = sys.argv[1]
    print(result_path)

    results_analyzer = results_analyzer_c()
    results_analyzer.get_all_setting_result(result_path)
