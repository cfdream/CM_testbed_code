#!/usr/bin/python

import time
import commands
import mini_system_manager
from generate_rules_into_forwarding_table import GenerateRulesIntoTables
import receiver_manager
import sender_manager
import re

def run_one_round(install_rule_type):
    #-----------create FIFO files for each sender
    commands.getstatusoutput('mkdir /tmp/fifo')
    commands.getstatusoutput('mkdir /tmp/log')
    commands.getstatusoutput('mkdir /tmp/sender')
    commands.getstatusoutput('mkdir /tmp/switch')
    commands.getstatusoutput('rm /tmp/fifo/*')
    commands.getstatusoutput('rm /tmp/log/*')
    commands.getstatusoutput('rm /tmp/sender/*')
    commands.getstatusoutput('rm /tmp/switch/*')
    commands.getstatusoutput('../public_lib/createFIFOFiles')
    print 'SUCC: rm dir and mkdir'

    #------------restart ovs------------
    commands.getstatusoutput('sudo mn -c')
    ret, cmd = commands.getstatusoutput('cd ../../openvswitch-2.3.2/; sudo python autobuild_ovs_mininet.py 4')
    ret, cmd = commands.getstatusoutput('cd ../../openvswitch-2.3.2/; sudo python autobuild_ovs_mininet.py 1')
    #ret, cmd = commands.getstatusoutput('sudo python ../../openvswitch-2.3.2/autobuild_ovs_mininet.py 2')
    print'{0} {1}' .format(ret, cmd)
    if ret != 0:
        return

    #------------setup the mininet------------
    system_topo = mini_system_manager.SystemManager()
    system_topo.configUserspaceThreadNum()
    system_topo.setup()
    system_topo.installForwardingRule(install_rule_type)
    system_topo.configuMTUSize()

    #-----------test connectivity------------
    #system_topo.testConnectivity()

    #------------setup all receivers------------
    receivermanager = receiver_manager.ReceiverManager(system_topo.net)
    receivermanager.setup()

    #------------setup all senders------------
    sendermanager = sender_manager.SenderManger(system_topo.net)
    sendermanager.setup()

    #------------wait for experiments to run------------
    #time.sleep(15500); #7500 per interval, 2 intervals
    #time.sleep(8500); #7500 per interval, 1 intervals
    time.sleep(5500); #3750 per interval, 1 intervals
    #time.sleep(23000); #750 per interval, 30 intervals
    #time.sleep(11300); #750 per interval, 15 intervals
    #time.sleep(2300); #750 per interval, 2 intervals + wait one interval
    #time.sleep(1550); #750 per interval, 1 intervals + wait one interval

    #------------stop all senders and receivers------------
    commands.getstatusoutput('sudo pkill tcpreplay')
    commands.getstatusoutput('sudo pkill receiver')
    commands.getstatusoutput('sudo mn -c')
    commands.getstatusoutput('sudo pkill ovs')
    print "one setting clear completed"

    #------------tear down the mininet------------
    #system_topo.tearDown()
    #print "system_topo tearDown completed"

def config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet):
    config_fname = '../public_lib/cm_experiment_setting.txt'
    in_file = open(config_fname, 'r')
    lines = in_file.readlines();
    lines=map(lambda x:x[:-1], lines)
    in_file.close()

    #pattern1 = re.compile("^system@(\w+):")
    #id_name_pattern = re.compile("^\tport (\d+): (\w+-\w+)")
    id_name_pattern = re.compile("^\t\t(\w+-\w+) (\d+)/\w+")
    host_switch_sample_pattern = re.compile("^host_or_switch_sample:(\d+)")
    replace_pattern = re.compile("^replacement:(\d+)")
    memory_type_pattern = re.compile("^switch_mem_type:(\d+)")
    memory_times_pattern = re.compile("^switch_memory_times:(\d+)")
    freq_pattern = re.compile("^condition_msec_freq:(\d+)")
    switch_drop_rate_pattern = re.compile("^switch_drop_rate:(\d+\.\d+)")
    target_flow_loss_rate_pattern = re.compile("^target_flow_loss_rate:(\d+\.\d+)")
    target_flow_volume_pattern = re.compile("^target_flow_volume:(\d+)")
    target_floww_loss_volume_pattern = re.compile("^target_flow_loss_volume:(\d+)")
    target_flow_volume_in_sampling_pattern = re.compile("^target_flow_volume_in_sampling:(\d+)")
    inject_or_tag_packet_pattern = re.compile("^inject_or_tag_packet:(\d+)")

    for line in lines:
        #host_or_switch_sample
        match = host_switch_sample_pattern.match(line)
        if match != None:
            #config host_switch_sample
            sed_str = "sed -i 's/^{0}/host_or_switch_sample:{1}/g' {2} " .format(line, host_switch_sample, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #replace
        match = replace_pattern.match(line)
        if match != None:
            #config replace
            sed_str = "sed -i 's/^{0}/replacement:{1}/g' {2} " .format(line, replace, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #memory_type
        match = memory_type_pattern.match(line)
        if match != None:
            #config match
            sed_str = "sed -i 's/^{0}/switch_mem_type:{1}/g' {2} " .format(line, memory_type, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #memory_times
        match = memory_times_pattern.match(line)
        if match != None:
            #config memory_times
            sed_str = "sed -i 's/^{0}/switch_memory_times:{1}/g' {2} " .format(line, memory_times, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #freq
        match = freq_pattern.match(line)
        if match != None:
            #config freq
            sed_str = "sed -i 's/^{0}/condition_msec_freq:{1}/g' {2} " .format(line, freq, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #volume threshold
        match = target_flow_volume_pattern.match(line)
        if match != None:
            #configu target_flow_volume
            sed_str = "sed -i 's/^{0}/target_flow_volume:{1}/g' {2} " .format(line, target_flow_volume, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #loss volume threshold
        match = target_floww_loss_volume_pattern.match(line)
        if match != None:
            #configu target_flow_loss_volume
            sed_str = "sed -i 's/^{0}/target_flow_loss_volume:{1}/g' {2} " .format(line, target_flow_loss_volume, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #target_flow_volume_in_sampling
        match = target_flow_volume_in_sampling_pattern.match(line)
        if match != None:
            #config target_flow_volume_in_sampling
            sed_str = "sed -i 's/^{0}/target_flow_volume_in_sampling:{1}/g' {2} " .format(line, target_flow_volume_in_sampling, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str
            

        #switch_drop_rate
        match = switch_drop_rate_pattern.match(line)
        if match != None:
            #config switch_drop_rate
            sed_str = "sed -i 's/^{0}/switch_drop_rate:{1}/g' {2} " .format(line, switch_drop_rate, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str


        #loss rate threshold
        match = target_flow_loss_rate_pattern.match(line)
        if match != None:
            #config target_flow_loss_rate
            sed_str = "sed -i 's/^{0}/target_flow_loss_rate:{1}/g' {2} " .format(line, target_flow_loss_rate, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str

        #inject_or_tag_packet
        match = inject_or_tag_packet_pattern.match(line)
        if match != None:
            #config inject_or_tag_packet
            sed_str = "sed -i 's/^{0}/inject_or_tag_packet:{1}/g' {2} " .format(line, inject_or_tag_packet, config_fname)
            commands.getstatusoutput(sed_str)
            print sed_str
    
def move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet):
    result_dir = './experiment_log/sample_{0}_replace_{1}_mem_{2}_mem_times_{3}_freq_{4}_tag_{5}' .format(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    commands.getstatusoutput('mkdir {0}' .format(result_dir))
    commands.getstatusoutput('rm -rf {0}/*' .format(result_dir))
    #move result file to result_dir
    commands.getstatusoutput('mv /tmp/switch {0}' .format(result_dir))
    commands.getstatusoutput('mv /tmp/sender {0}' .format(result_dir))
    commands.getstatusoutput('mv /tmp/log {0}' .format(result_dir))
    print "SUCC: move_one_round_data"

def query1_compare_algos():
    '''
    Host measures loss, if the loss is high, 
    ask all the switches along the path to measure the volume of the high loss flow
    '''
    #50000 bytes memory => memory_times = 0.144085991
    #query1 has tasks 2 times comapred to query3, thus memory two times here as well.
    mem50kbytes_memory_times = 0.144085991 * 0.080402435

    #set task_info.txt as query1_task_info.txt
    ret, cmd = commands.getstatusoutput('cp ../public_lib/query1_task_info.txt ../public_lib/task_info.txt')
    
    #------------set switch_drop_rate to make avg. loss rate is 10%
    switch_drop_rate = 0.037

    #------------set target flow threshold
    target_flow_volume = 0
    target_flow_loss_volume = 3000
    target_flow_loss_rate = '0.0'

    #------------set target_flow_volume_in_sampling, according to target_flow_loss_volume (loss rate: ~10%)
    target_flow_volume_in_sampling = 30000

    #------------only single path forwarding rules will be installed in the network
    install_rule_type = GenerateRulesIntoTables.BOTH_SINGLE_AND_ECMP_PATH_RULE

    #------------Coordination + replace + taggingConditionPkt
    #NOTE: in cm_experiment_setting.txt set inject_or_tag_packet = 1
    inject_or_tag_packet = 1
    for host_switch_sample in [0]:
        for memory_type in [0]:
            for replace in [1]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8, 16]:
                    for freq in [500]: #freq is useless in tagging
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
                        run_one_round(install_rule_type)
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
    #2.3. HSSH+fixed memory + replace
    #2.2. HSSH+fixed memory
    #inject_or_tag_packet = 0
    #for host_switch_sample in [0]:
    #    for memory_type in [0]:
    #        for replace in [1, 0]:
    #            for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
    #                for freq in [500]:
    #                    config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
    #                    run_one_round(install_rule_type)
    #                    move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
    #2.4 No-coord <=> Switch sample and hold + no replace
    #2.5 No-coord + replace <=> Switch sample and hold replace : 
    #why 2.5: I want to check whether No-coord is better than HSSH. Coz accuracy of Nocoord with smaller memory is better than HSSH, that's why we need to use HSSH+replace. 
    #However, this is under the assumption that Nocoord+replace is worse than HSSH+replace, 2.5 is used to verify this.
    inject_or_tag_packet = 0
    for host_switch_sample in [1]:
        for memory_type in [0]:
            for replace in [0]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8, 16]:
                    for freq in [500]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
                        run_one_round(install_rule_type)
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)

def query1_change_selected_flow_volume_threshold():
    '''
    Host measures loss, if the loss is high, 
    ask all the switches along the path to measure the volume of the high loss flow
    Change variable: the loss volume threshold to send selected flows. Loss volume threshold is set as 6k, we can send selected flows when loss threshold is 4k, 5k, 6k;
    We want to investigate the effect of (4k,5k,6k) on the performance.
    Different memory might have different senstivity to the change of (4k,5k,6k)
    '''
    #50000 bytes memory => memory_times = 0.144085991
    #query1 has tasks 2 times comapred to query3, thus memory two times here as well.
    mem50kbytes_memory_times = 0.144085991 * 0.080402435

    #set task_info.txt as query1_task_info.txt
    ret, cmd = commands.getstatusoutput('cp ../public_lib/query1_task_info.txt ../public_lib/task_info.txt')
    
    #------------set switch_drop_rate to make avg. loss rate is 10%
    switch_drop_rate = 0.037

    #------------set target flow threshold
    target_flow_volume = 0
    target_flow_loss_volume = 6000
    target_flow_loss_rate = '0.0'

    #------------set target_flow_volume_in_sampling, according to target_flow_loss_volume (loss rate: ~10%)
    target_flow_volume_in_sampling = 60000

    #------------only single path forwarding rules will be installed in the network
    install_rule_type = GenerateRulesIntoTables.BOTH_SINGLE_AND_ECMP_PATH_RULE

    #------------Coordination + replace + taggingConditionPkt
    #NOTE: in cm_experiment_setting.txt set inject_or_tag_packet = 1
    inject_or_tag_packet = 1
    host_switch_sample = 0
    memory_type = 0
    replace = 1
    freq = 500 #freq is useless in tagging
    for memory_times in [0.5, 2, 8]:
        for target_flow_loss_volume  in [6000, 5500, 5000, 4500, 4000, 3500, 3000, 2500, 2000]
            config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
            run_one_round(install_rule_type)
            move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)

def query3_compare_algos():
    '''
    Condition: high volume at sender; 
    Capture: one switch at each path for all the paths, one monitor per path
    '''

    #50000 bytes memory => memory_times = 0.144085991
    #mem50kbytes_memory_times = 0.144085991 #for 8 bytes per bucket
    #previous create 307727 buckets, we need 24742 buckets, which is 300kbytes (<srcip, dstip>, counter, 1 bit)
    mem50kbytes_memory_times = 0.144085991 * 0.080402435 / 2

    #set task_info.txt as query3_task_info.txt
    ret, cmd = commands.getstatusoutput('cp ../public_lib/query3_task_info.txt ../public_lib/task_info.txt')

    #------------set switch_drop_rate to make avg. loss rate is 0%
    switch_drop_rate = '0.0'

    #------------set target flow threshold
    target_flow_volume = 60000
    target_flow_loss_volume = 0
    target_flow_loss_rate = '0.0'

    #------------set target_flow_volume_in_sampling, according to target_flow_loss_volume (loss rate: ~10%)
    target_flow_volume_in_sampling = 60000

    #------------both single and multiple forwarding rules will be installed in the network
    install_rule_type = GenerateRulesIntoTables.BOTH_SINGLE_AND_ECMP_PATH_RULE

    #------------Coordination + replace + taggingConditionPkt
    inject_or_tag_packet = 1
    for host_switch_sample in [0]:
        for memory_type in [0]:
            for replace in [1]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
                    for freq in [500]: #freq is useless in tagging
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
                        run_one_round(install_rule_type)
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
    #2.3. HSSH+fixed memory + replace
    #2.2. HSSH+fixed memory
    #inject_or_tag_packet = 0
    #for host_switch_sample in [0]:
    #    for memory_type in [0]:
    #        for replace in [1, 0]:
    #            for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
    #                for freq in [500]:
    #                    config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
    #                    run_one_round(install_rule_type)
    #                    move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
    #--------------No-coord <=> Switch sample and hold + no replace
    #2.5 No-coord + replace <=> Switch sample and hold replace : 
    #why 2.5: I want to check whether No-coord is better than HSSH. Coz accuracy of Nocoord with smaller memory is better than HSSH, that's why we need to use HSSH+replace. 
    #However, this is under the assumption that Nocoord+replace is worse than HSSH+replace, 2.5 is used to verify this.
    inject_or_tag_packet = 0
    for host_switch_sample in [1]:
        for memory_type in [0]:
            for replace in [0]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
                    for freq in [500]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, switch_drop_rate, target_flow_loss_rate, target_flow_volume, target_flow_loss_volume, target_flow_volume_in_sampling, inject_or_tag_packet)
                        run_one_round(install_rule_type)
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)

def experiment1_compare_algos():
    target_flow_loss_rate = 0.01
    #50000 bytes memory => memory_times = 0.144085991
    mem50kbytes_memory_times = 0.144085991
    #######For figure: memory_size vs. performance
    #2.1. HSSH+fixed memory+replace+ taggingConditionPkt
    #NOTE: in cm_experiment_setting.txt set inject_or_tag_packet = 1
    inject_or_tag_packet = 1
    for host_switch_sample in [0]:
        for memory_type in [0]:
            for replace in [1]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
                    for freq in [500]: #freq is useless in tagging
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, target_flow_loss_rate, inject_or_tag_packet)
                        run_one_round()
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
    #2.3. HSSH+fixed memory + replace
    #inject_or_tag_packet = 0
    #for host_switch_sample in [0]:
    #    for memory_type in [0]:
    #        for replace in [1]:
    #            for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
    #                for freq in [500]:
    #                    config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, target_flow_loss_rate, inject_or_tag_packet)
    #                    run_one_round()
    #                    move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    #2.2. HSSH+fixed memory
    
    #2.4 No-coord <=> Switch sample and hold + no replace
    #2.5 No-coord + replace <=> Switch sample and hold replace : 
    #why 2.5: I want to check whether No-coord is better than HSSH. Coz accuracy of Nocoord with smaller memory is better than HSSH, that's why we need to use HSSH+replace. 
    #However, this is under the assumption that Nocoord+replace is worse than HSSH+replace, 2.5 is used to verify this.
    inject_or_tag_packet = 0
    for host_switch_sample in [1]:
        for memory_type in [0]:
            for replace in [0, 1]:
                for memory_times in [0.25, 0.5, 1, 2, 4, 8]:
                    for freq in [500]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times*mem50kbytes_memory_times, freq, target_flow_loss_rate, inject_or_tag_packet)
                        run_one_round()
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq, inject_or_tag_packet)
    
def experiment2_freq_on_performance():
    #######For figure: condition frequency vs. performance
    #based on HSSH+Replace
    target_flow_loss_rate = 0.01
    for host_switch_sample in [0]:
        for memory_type in [0]:
            for replace in [1]:
                for memory_times in [0.2]:
                    for freq in [1000, 2000, 4000, 8000, 16000]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq, target_flow_loss_rate)
                        run_one_round()
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq)

def experiment3_numFlows_vs_overhead():
    '''
    run experiments to get the relationship between #flows and network overhead and memory overhead
    '''
    host_switch_sample = 0
    memory_type = 0
    replace = 1
    freq = 1000

if __name__ == "__main__":
    query1_compare_algos()
    #query3_compare_algos()
    #query1_change_selected_flow_volume_threshold
    #experiment1_compare_algos()
    #experiment2_freq_on_performance()
    #experiment3_numFlows_vs_overhead()

