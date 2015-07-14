#!/usr/bin/python

import commands
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel

class ReceiverManager():
    def __init__(self, net):
        self.net = net
    def setup(self):
        hosts = self.net.hosts
        outfiles, errfiles = {}, {}
        for h in hosts:
            outfiles[ h ] = '/tmp/log/%s.rece.out' % h.name
            errfiles[ h ] = '/tmp/log/%s.rece.err' % h.name
            #run the receiver detectPacketLossID
            h.cmd( 'echo >', outfiles[ h ] )
            h.cmd( 'echo >', errfiles[ h ] )
            # Start pings
            h.cmdPrint('sudo ../receiverSendReceFlowSeqid/receiverSendReceFlowSeqid',
                       '>', outfiles[ h ],
                       '2>', errfiles[ h ],
                       '&' )
