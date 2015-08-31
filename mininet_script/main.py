#!/usr/bin/python

import time
import commands
import mini_system_manager
import receiver_manager
import sender_manager
import re

def run_one_round():
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
    system_topo.installForwardingRule()
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
    #time.sleep(8000); #750 per interval, 10 intervals
    time.sleep(23000); #750 per interval, 30 intervals
    #time.sleep(2300); #750 per interval, 2 intervals + wait one interval

    #------------tear down the mininet------------
    system_topo.tearDown()

    #------------stop all senders and receivers------------
    commands.getstatusoutput('sudo pkill tcpreplay')
    commands.getstatusoutput('sudo pkill receiver')
    commands.getstatusoutput('sudo mn -c')

def config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq):
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
    
def move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq):
    result_dir = './experiment_log/sample_{0}_replace_{1}_mem_{2}_mem_times_{3}_freq_{4}' .format(host_switch_sample, replace, memory_type, memory_times, freq)
    commands.getstatusoutput('mkdir {0}' .format(result_dir))
    commands.getstatusoutput('rm -rf {0}/*' .format(result_dir))
    #move result file to result_dir
    commands.getstatusoutput('mv /tmp/switch {0}' .format(result_dir))
    commands.getstatusoutput('mv /tmp/sender {0}' .format(result_dir))
    commands.getstatusoutput('mv /tmp/log {0}' .format(result_dir))
    print "SUCC: move_one_round_data"

if __name__ == "__main__":
    #######For figure: memory_size vs. performance
    #2.1. HSSH+fixed memory
    #not necessary-2.2. HSSH+fixed memory + replace
    for host_switch_sample in [0]:
        for memory_type in [0]:
            for replace in [0]:
                for memory_times in [8, 4, 2, 1]:
                    for freq in [500]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq)
                        run_one_round()
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq)
    
    #2.3. HSSH+diverse memory
    #2.4. HSSH+diverse memory + replace
    #for host_switch_sample in [0]:
    #    for memory_type in [1]:
    #        for replace in [0, 1]:
    #            for memory_times in [8, 4, 2, 1]:
    #                for freq in [500]:
    #                    config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq)
    #                    run_one_round()
    #                    move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq)

    #for experiment of SH +- no replace + memory_type:1-diverse +- memory_times + freq:5s
    #2.5 No-coord
    for host_switch_sample in [1]:
        for memory_type in [0]:
            for replace in [0]:
                for memory_times in [8, 4, 2, 1]:
                    for freq in [500]:
                        config_experiment_setting_file(host_switch_sample, replace, memory_type, memory_times, freq)
                        run_one_round()
                        move_one_round_data(host_switch_sample, replace, memory_type, memory_times, freq)
