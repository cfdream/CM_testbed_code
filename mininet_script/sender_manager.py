#!/usr/bin/python

import commands
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel

class SenderManger():
    def __init__(self, net):
        self.net = net
    def setup(self):
        hosts = self.net.hosts
        outfiles, errfiles = {}, {}
        for h in hosts:
            #------debug------
            if h.name != "h1":
                continue
            #------debug------

            outfiles[ h ] = '/tmp/log/%s.send.out' % h.name
            errfiles[ h ] = '/tmp/log/%s.send.err' % h.name
            #run the receiver detectPacketLossID
            h.cmd( 'echo >', outfiles[ h ] )
            h.cmd( 'echo >', errfiles[ h ] )
            # Start pings
            h.cmdPrint('sudo ../tcpreplay/src/tcpreplay -i h1-eth0   ~/workspace/caida_data/sender1_head1w.pcap',
                       '>', outfiles[ h ],
                       '2>', errfiles[ h ],
                       '&' )
