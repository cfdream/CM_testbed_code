#!/usr/bin/python

import commands
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
import generate_rules_into_forwarding_table

class ReceiverManager():
    def __init__(self, net):
        self.net = net
    def setup(self):
        #get the ip prefixes
        generator = generate_rules_into_forwarding_table.GenerateRulesIntoTables()
        generator.read_eth_id_name_topo("eth_id_name_map.txt")
        generator.read_topo("b4_topo_ip_prefixes.txt")

        hosts = self.net.hosts
        outfiles, errfiles = {}, {}
        for h in hosts:
            #------debug------
            if h.name != "h1" and h.name != "h2":
                continue
            #------debug------

            outfiles[ h ] = '/tmp/log/%s.rece.out' % h.name
            errfiles[ h ] = '/tmp/log/%s.rece.err' % h.name
            #run the receiver detectPacketLossID
            h.cmd( 'echo >', outfiles[ h ] )
            h.cmd( 'echo >', errfiles[ h ] )
            command="sudo ../receiverSendReceFlowSeqid/receiverSendReceFlowSeqid \"{0}\"" .format(generator.host_ipprefix_map[h.name])
            # Start pings
            h.cmdPrint(command,
                       '>', outfiles[ h ],
                       '2>', errfiles[ h ],
                       '&' )
            print command
