#!/usr/bin/python

import time
import commands
import mini_system_manager
import receiver_manager
import sender_manager

if __name__ == "__main__":
    #-----------create FIFO files for each sender
    commands.getstatusoutput('mkdir data/fifo')
    commands.getstatusoutput('mkdir data/log')
    commands.getstatusoutput('mkdir data/sender')
    commands.getstatusoutput('mkdir data/switch')
    commands.getstatusoutput('rm data/fifo/*')
    commands.getstatusoutput('rm data/log/*')
    commands.getstatusoutput('rm data/sender/*')
    commands.getstatusoutput('rm data/switch/*')
    commands.getstatusoutput('../public_lib/createFIFOFiles')

    #------------setup the mininet------------
    system_topo = mini_system_manager.SystemManager()
    system_topo.configUserspaceThreadNum()
    system_topo.setup()
    system_topo.installForwardingRule()
    system_topo.configuMTUSize()

    #-----------test connectivity------------
    #system_topo.testConnectivity()

    #------------setup all receivers------------
    receiver_manager = receiver_manager.ReceiverManager(system_topo.net)
    receiver_manager.setup()

    #------------setup all senders------------
    sender_manager = sender_manager.SenderManger(system_topo.net)
    sender_manager.setup()

    #------------wait for experiments to run------------
    time.sleep(20); #600 seconds

    #------------tear down the mininet------------
    system_topo.tearDown()
