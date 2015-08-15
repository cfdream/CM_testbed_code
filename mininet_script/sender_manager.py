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
            #------debug------ 4 senders
            #if h.name != "h1":
            #if h.name != "h1" and h.name != "h5" and h.name != "h8" and h.name != "h12":
            #    continue
            #------debug------
            print h.name

            outfiles[ h ] = '/tmp/log/%s.send.out' % h.name
            errfiles[ h ] = '/tmp/log/%s.send.err' % h.name
            #run the receiver detectPacketLossID
            h.cmd( 'echo >', outfiles[ h ] )
            h.cmd( 'echo >', errfiles[ h ] )
            # Start pings
            cmdstr = 'sudo ../tcpreplay/src/tcpreplay --mbps=1 -i {0}-eth0   ../../caida_data/output/{1}.pcap' .format(h.name, h.name)
            #cmdstr = 'sudo ../tcpreplay/src/tcpreplay --mbps=1 -i {0}-eth0   ../../caida_data/s{1}_head100w.pcap' .format(h.name, int(h.name[1:]))
            #cmdstr = 'sudo ../tcpreplay/src/tcpreplay -i {0}-eth0   ../pcapFileGenerator/three_pkt_repeat.pcap' .format(h.name)
            h.cmdPrint( cmdstr,
                       '>', outfiles[ h ],
                       '2>', errfiles[ h ],
                       '&' )
            print cmdstr

