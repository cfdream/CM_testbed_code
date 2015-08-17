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
                    for freq in [5, 10, 20, 40, 60]:
                        #one setting result path
                        one_setting_path = '{0}/sample_{1}_replace_{2}_mem_{3}_freq_{4}' .format(result_path, host_switch_sample, replace, memory_type, freq)
                        if not os.path.lexists(one_setting_path):
                            continue
                        #get the results of one setting
                        one_setting_result_calculator = one_setting_result_calculator_c()
                        one_setting_result = one_setting_result_calculator.get_one_setting_result(one_setting_path)
                        #output the results of one setting
                        self.output_one_setting_result(host_switch_sample, replace, memory_type, freq, one_setting_result)

    def output_one_setting_result(self, host_switch_sample, replace, memory_type, freq, one_setting_result):
        with open('result_query1_diff_setting.txt', 'a') as out_file:
            #setting info
            out_file.write("{host_switch_sample}\t{replace}\t{memory_type}\t{freq}\t{avg_fn}\t{min_fn}\t{max_fn}\t{stdv_fn}\t{avg_fp}\t{min_fp}\t{max_fp}\t{stdv_fp}\t{avg_accuracy}\t{min_accuracy}\t{max_accuracy}\t{stdv_accuracy}\t{avg_sample_map_size}\t{min_sample_map_size}\t{max_sample_map_size}\t{avg_condition_map_size}\t{min_condition_map_size}\t{max_sample_map_size}\n" \
            .format(host_switch_sample=host_switch_sample, \
                replace=replace, memory_type=memory_type, freq=freq, \
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
                max_condition_map_size = one_setting_result.max_condition_map_size
            )) 

if __name__ == '__main__':
    if len(sys.argv) !=2:
        print("usage: python result_analysis.py result_path")
        exit(0)

    result_path = sys.argv[1]
    print(result_path)

    results_analyzer = results_analyzer_c()
    results_analyzer.get_all_setting_result(result_path)