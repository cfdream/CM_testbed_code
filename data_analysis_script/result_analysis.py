#!/usr/bin/python

import time
import re
import os
import sys
from one_setting_result_calculator import one_setting_result_calculator_c

class results_analyzer_c():
    def get_all_setting_result(self, result_path):
        for host_switch_sample in [0, 1]:
            for replace in [0, 1]:
                for memory_type in [0, 1]:
                    for memory_times in [1, 2, 4, 8, 10,16]:
                        for freq in [500, 2000, 8000, 32000]:
                            #one setting result path
                            one_setting_path = '{0}/sample_{1}_replace_{2}_mem_{3}_mem_times_{4}_freq_{5}' .format(result_path, host_switch_sample, replace, memory_type, memory_times, freq)
                            if not os.path.lexists(one_setting_path):
                                continue
                            #get the results of one setting
                            one_setting_result_calculator = one_setting_result_calculator_c()
                            one_setting_result = one_setting_result_calculator.get_one_setting_result(one_setting_path)
                            #output the results of one setting
                            self.output_one_setting_result(host_switch_sample, replace, memory_type, memory_times, freq, one_setting_result)

    def output_one_setting_result(self, host_switch_sample, replace, memory_type, memory_times, freq, one_setting_result):
        with open('result_query1_diff_setting.num', 'a') as out_file:
            #setting info
            out_file.write("{host_switch_sample}\t{replace}\t{memory_type}\t{memory_times}\t{freq}\t{avg_fn}\t{min_fn}\t{max_fn}\t{stdv_fn}\t{avg_fp}\t{min_fp}\t{max_fp}\t{stdv_fp}\t{avg_accuracy}\t{min_accuracy}\t{max_accuracy}\t{stdv_accuracy}\t{avg_sample_map_size}\t{min_sample_map_size}\t{max_sample_map_size}\t{avg_condition_map_size}\t{min_condition_map_size}\t{max_condition_map_size}\t{raw_host_sample_switch_hold_accuracy}\t{avg_real_target_flow_num}\t{avg_fn_num}\n" \
            .format(host_switch_sample=host_switch_sample, \
                replace=replace, memory_type=memory_type, memory_times=memory_times, freq=freq, \
                avg_fn = one_setting_result.avg_fn, \
                min_fn = one_setting_result.min_fn, \
                max_fn = one_setting_result.max_fn, \
                stdv_fn = one_setting_result.stdv_fn, \
                avg_fp = one_setting_result.avg_fp, \
                min_fp = one_setting_result.min_fp, \
                max_fp = one_setting_result.max_fp, \
                stdv_fp = one_setting_result.stdv_fp, \
                avg_accuracy = one_setting_result.avg_accuracy, \
                min_accuracy = one_setting_result.min_accuracy, \
                max_accuracy = one_setting_result.max_accuracy, \
                stdv_accuracy = one_setting_result.stdv_accuracy, \
                avg_sample_map_size = one_setting_result.avg_sample_map_size, \
                min_sample_map_size = one_setting_result.min_sample_map_size, \
                max_sample_map_size = one_setting_result.max_sample_map_size, \
                avg_condition_map_size = one_setting_result.avg_condition_map_size, \
                min_condition_map_size = one_setting_result.min_condition_map_size, \
                max_condition_map_size = one_setting_result.max_condition_map_size, \
                raw_host_sample_switch_hold_accuracy = one_setting_result.raw_host_sample_switch_hold_accuracy, \
                avg_real_target_flow_num = one_setting_result.avg_real_target_flow_num, \
                avg_fn_num = one_setting_result.avg_fn_num
            )) 
        with open('result_query1_diff_setting.txt', 'a') as out_file:
            #setting info
            out_file.write("host_switch_sample={host_switch_sample}\treplace={replace}\tmemory_type={memory_type}\tmemory_times={memory_times}\tfreq={freq}\tavg_fn={avg_fn}\tmin_fn={min_fn}\tmax_fn={max_fn}\tstdv_fn={stdv_fn}\tavg_fp={avg_fp}\tmin_fp={min_fp}\tmax_fp={max_fp}\tstdv_fp={stdv_fp}\tavg_accuracy={avg_accuracy}\tmin_accuracy={min_accuracy}\tmax_accuracy={max_accuracy}\tstdv_accuracy={stdv_accuracy}\tavg_sample_map_size={avg_sample_map_size}\tmin_sample_map_size={min_sample_map_size}\tmax_sample_map_size={max_sample_map_size}\tavg_condition_map_size={avg_condition_map_size}\tmin_condition_map_size={min_condition_map_size}\tmax_sample_map_size={max_sample_map_size}\traw_host_sample_switch_hold_accuracy={raw_host_sample_switch_hold_accuracy}\tavg_real_target_flow_num={avg_real_target_flow_num}\tavg_fn_num={avg_fn_num}\n" \
            .format(host_switch_sample=host_switch_sample, \
                replace=replace, memory_type=memory_type, memory_times=memory_times, freq=freq, \
                avg_fn = one_setting_result.avg_fn, \
                min_fn = one_setting_result.min_fn, \
                max_fn = one_setting_result.max_fn, \
                stdv_fn = one_setting_result.stdv_fn, \
                avg_fp = one_setting_result.avg_fp, \
                min_fp = one_setting_result.min_fp, \
                max_fp = one_setting_result.max_fp, \
                stdv_fp = one_setting_result.stdv_fp, \
                avg_accuracy = one_setting_result.avg_accuracy, \
                min_accuracy = one_setting_result.min_accuracy, \
                max_accuracy = one_setting_result.max_accuracy, \
                stdv_accuracy = one_setting_result.stdv_accuracy, \
                avg_sample_map_size = one_setting_result.avg_sample_map_size, \
                min_sample_map_size = one_setting_result.min_sample_map_size, \
                max_sample_map_size = one_setting_result.max_sample_map_size, \
                avg_condition_map_size = one_setting_result.avg_condition_map_size, \
                min_condition_map_size = one_setting_result.max_condition_map_size, \
                max_condition_map_size = one_setting_result.max_condition_map_size, \
                raw_host_sample_switch_hold_accuracy = one_setting_result.raw_host_sample_switch_hold_accuracy, \
                avg_real_target_flow_num = one_setting_result.avg_real_target_flow_num, \
                avg_fn_num = one_setting_result.avg_fn_num
            )) 

if __name__ == '__main__':
    if len(sys.argv) !=2:
        print("usage: python3 result_analysis.py result_path")
        exit(0)

    result_path = sys.argv[1]
    print(result_path)

    results_analyzer = results_analyzer_c()
    results_analyzer.get_all_setting_result(result_path)
