#!/usr/bin/python

import commands
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel
import multiprocessing
import b4_topo_mininet
import generate_rules_into_forwarding_table
from test.multipoll import monitorFiles

class SystemManager():
    def setup(self):
        self.topo = b4_topo_mininet.B4Topo()
        self.net = Mininet(self.topo, link=TCLink)
        self.net.start()
        print "Dumping host connections"
        dumpNodeConnections(self.net.hosts)

    def tearDown(self):
        self.net.stop()

    ##
    # @brief genearte forwarding rules, and install the rules
    #
    # @return 
    def installForwardingRule(self, install_rule_type):
        #generate eth name id map
        commands.getstatusoutput('sudo ovs-appctl dpif/show >eth_id_name_map.txt')
        #generate rules_to_add_flow_table.sh
        generator = generate_rules_into_forwarding_table.GenerateRulesIntoTables()
        generator.read_eth_id_name_topo("eth_id_name_map.txt")
        generator.read_topo("b4_topo_ip_prefixes.txt")
        generator.generate_rules_for_normal_packets("rules_to_add_flow_table.sh", install_rule_type)
        generator.generate_rules_for_condition_packets("rules_to_add_flow_table.sh")
        #run rules_to_add_flow_table.sh
        commands.getstatusoutput('sh -x rules_to_add_flow_table.sh')
        #remove useless files
        #commands.getstatusoutput('rm eth_id_name_map.txt')
        #commands.getstatusoutput('rules_to_add_flow_table.sh')

    def configuMTUSize(self):
        for host in self.net.hosts:
            host.cmd('sudo ifconfig {0}-eth0 mtu 9000' .format(host))
            print 'sudo ifconfig {0}-eth0 mtu 9000' .format(host)

        #generate rules_to_add_flow_table.sh
        generator = generate_rules_into_forwarding_table.GenerateRulesIntoTables()
        generator.read_eth_id_name_topo("eth_id_name_map.txt")
        generator.read_topo("b4_topo_ip_prefixes.txt")
        for switch_eth in generator.switch_eth_name_id_map:
            commands.getstatusoutput('sudo ifconfig {0} mtu 9000' .format(switch_eth))
            print 'sudo ifconfig {0} mtu 9000' .format(switch_eth)

    def configUserspaceThreadNum(self):
        num_cores = multiprocessing.cpu_count()
        ret_str = commands.getstatusoutput('sudo ovs-vsctl show')
        db_str = ret_str[1].split()[0]
        commands.getstatusoutput('ovs-vsctl add Open_vSwitch {0} other_config n-handler-threads={1}' .format(db_str, num_cores))
        #commands.getstatusoutput('ovs-vsctl add Open_vSwitch {0} other_config n-handler-threads={1}' .format(db_str, 2))
        print 'ovs-vsctl add Open_vSwitch {0} other_config n-handler-threads={1}' .format(db_str, num_cores)
        #print 'ovs-vsctl add Open_vSwitch {0} other_config n-handler-threads={1}' .format(db_str, 2)

    ##
    # @brief test the connectivity from host[0] to other hosts
    #
    # @return 
    def testConnectivity(self):
        print "Testing network connectivity"

        hosts = self.net.hosts
        print "Starting test..."
        server = hosts[ 0 ]
        outfiles, errfiles = {}, {}
        for h in hosts:
            # Create and/or erase output files
            outfiles[ h ] = '/tmp/%s.out' % h.name
            errfiles[ h ] = '/tmp/%s.err' % h.name
            h.cmd( 'echo >', outfiles[ h ] )
            h.cmd( 'echo >', errfiles[ h ] )
            # Start pings
            h.cmdPrint('ping', server.IP(),
                       '>', outfiles[ h ],
                       '2>', errfiles[ h ],
                       '&' )
        seconds=3
        print "Monitoring output for", seconds, "seconds"
        for h, line in monitorFiles( outfiles, seconds, timeoutms=500 ):
            if h:
                print '%s: %s' % ( h.name, line )
        for h in hosts:
            h.cmd('kill %ping')
            commands.getstatusoutput('sudo rm /tmp/%s.out' % h.name)
            commands.getstatusoutput('sudo rm /tmp/%s.err' % h.name)
