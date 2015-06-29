import commands
import sys
import string
import re

class NeighborNode:
    def __init__(self, ethid, neigh_name, neigh_ethid):
        self.ethid=ethid
        self.neigh_name=neigh_name
        self.neigh_ethid=neigh_ethid

class GenerateRulesIntoTables:
    HIGH_PRIORITY = 32769
    LOW_PRIORITY = 32768
    def __init__(self):
        self.hosts=[]
        self.switches=[]
        self.switch_eth_name_id_map={}
        self.graph={}
        self.host_ipprefix_map={}

    def read_eth_id_name_topo(self, filename):
        in_file = open(filename, 'r')
        lines = in_file.readlines();
        in_file.close()
        #lines=map(lambda x:x[:-1], lines)

        #pattern1 = re.compile("^system@(\w+):")
        id_name_pattern = re.compile("^\tport (\d+): (\w+-\w+)")
        
        for line in lines:
            match = id_name_pattern.match(line)
            if match == None:
                continue
            self.switch_eth_name_id_map[match.group(2)] = match.group(1)
            print match.groups()

    def read_topo(self, filename):
        file=open(filename, 'r')
        lines=file.readlines()
        lines=map(lambda x:x[:-1], lines)
        file.close();

        #hosts
        ith_line=0
        while ith_line < len(lines) and lines[ith_line][0]=='#':
            ith_line+=1
        self.hosts=lines[ith_line].split(' ')
        ith_line+=1
        #print self.hosts

        #switches
        while ith_line < len(lines) and lines[ith_line][0]=='#':
            ith_line+=1
        self.switches=lines[ith_line].split(' ')
        ith_line+=1
        #print self.switches

        #host ip prefix
        self.host_ipprefix_map={}
        while ith_line < len(lines) and lines[ith_line][0]=='#':
            ith_line+=1
        #print lines[ith_line]
        while ith_line < len(lines) and lines[ith_line][0]!='#':
            temp=lines[ith_line].split(' ')
            host=temp[0]
            ip_prefix=temp[1]
            self.host_ipprefix_map[host] = ip_prefix
            ith_line+=1
        #print self.host_ipprefix_map

        switch_or_host_eth_num={}
        #switch host connections, switch switch connections
        while ith_line < len(lines):
            if lines[ith_line][0]=='#':
                ith_line+=1
                continue
            temp=lines[ith_line].split(' ')
            assert(len(temp)==2)
            entity1=temp[0]
            entity2=temp[1]
            #get the ethid of entity1 and entity2
            entity1_ethid=1
            if entity1 in switch_or_host_eth_num:
                entity1_ethid=switch_or_host_eth_num[entity1]+1
            switch_or_host_eth_num[entity1]=entity1_ethid
            entity2_ethid=1
            if entity2 in switch_or_host_eth_num:
                entity2_ethid=switch_or_host_eth_num[entity2]+1
            switch_or_host_eth_num[entity2]=entity2_ethid
            #<entity1, entity1_ethid> <==> <entity2, entity2_ethid>
            if entity1 not in self.graph:
                neighbors=[]
                self.graph[entity1]=neighbors
            entity1_neighs=self.graph[entity1]
            #get the eth-x to id mapping
            eth1_str = "{0}-eth{1}" .format(entity1, entity1_ethid)
            eth2_str = "{0}-eth{1}" .format(entity2, entity2_ethid)
            eth1_portid = 0
            if re.match("^h(\d+)", entity1) == None:
                #entity1 is a switch
                eth1_portid = self.switch_eth_name_id_map[eth1_str]
            eth2_portid = 0
            if re.match("^h(\d+)", entity2) == None:
                #entity2 is a switch
                eth2_portid = self.switch_eth_name_id_map[eth2_str]
            entity1_one_neigh = NeighborNode(eth1_portid, entity2, eth2_portid)
            entity1_neighs.append(entity1_one_neigh)

            if entity2 not in self.graph:
                neighbors=[]
                self.graph[entity2]=neighbors
            entity2_neighs=self.graph[entity2]
            entity2_one_neigh = NeighborNode(eth2_portid, entity1, eth1_portid)
            entity2_neighs.append(entity2_one_neigh)
            
            ith_line+=1

    def read_topo_test(self):
        self.read_topo("b4_topo_ip_prefixes.txt")
        #print len(self.hosts)
        assert(len(self.hosts)==12)
        assert(len(self.switches)==12)
        assert(len(self.graph['h1'])==1)
        assert(len(self.graph['s1'])==3)
        for i in range(0, len(self.graph['s1'])):
            if self.graph['s1'][i].neigh_name=='h1':
                assert(self.graph['s1'][i].ethid==0)
            elif self.graph['s1'][i].neigh_name=='s2':
                assert(self.graph['s1'][i].ethid==1)
            elif self.graph['s1'][i].neigh_name=='s3':
                assert(self.graph['s1'][i].ethid==2)

    def get_shortest_paths_from_src_to_dst(self, src, dst):
        visited_name_id_map={}
        visited_name_id_map[src]=0
        ith=0
        expand_name_list=[src]
        while ith < len(expand_name_list):
            now_name=expand_name_list[ith]
            now_id=visited_name_id_map[now_name]
            neighbors = self.graph[now_name]
            dst_reached=False
            for neigh in neighbors:
                neigh_name=neigh.neigh_name
                if neigh_name in visited_name_id_map:
                    continue
                expand_name_list.append(neigh_name)
                visited_name_id_map[neigh_name]=now_id+1
                if neigh_name == dst:
                    dst_reached=True
                    break
            if dst_reached:
                break
            ith+=1
        #reverse to get the ECMP paths
        path={}
        expand_name_list=[]
        expand_name_list.append(dst)
        ith = 0
        while ith < len(expand_name_list):
            now_name=expand_name_list[ith]
            now_id=visited_name_id_map[now_name]
            neighbors = self.graph[now_name]
            for neigh in neighbors:
                neigh_name=neigh.neigh_name
                if neigh_name not in visited_name_id_map:
                    continue
                neigh_id = visited_name_id_map[neigh_name]
                if neigh_id == now_id-1:
                    #one link: neigh_name=>now_name
                    if neigh_name not in path:
                        next_nodes=[]
                        path[neigh_name] = next_nodes
                    next_nodes=path[neigh_name]
                    next_nodes.append(now_name)
                    if neigh_name not in expand_name_list:
                        expand_name_list.append(neigh_name)
            ith+=1
        return path
    
    def get_shortest_paths_from_src_to_dst_test(self):
        #self.read_topo("b4.topo")
        path=self.get_shortest_paths_from_src_to_dst('h1', 'h2')
        print path
        #assert(len(path)==4)
        path=self.get_shortest_paths_from_src_to_dst('h1', 'h12')
        print path

    def generate_rules_for_normal_packets(self, filename):
        out_file = open(filename, 'w')
        for entity1 in self.hosts:
            for entity2 in self.hosts:
                if entity1 == entity2:
                    continue
                #from entity1 ===> entity2
                path = self.get_shortest_paths_from_src_to_dst(entity1, entity2)
                #add rule for each node in the path
                for entity, next_nodes in path.items():
                    if re.match("^h(\d+)", entity) != None:
                        #only add forwarding rules to switches
                        continue
                    if len(next_nodes) > 1:
                        neighbor_nodes=self.graph[entity]
                        ports=[]
                        for next_node in next_nodes:
                            for neighbor in neighbor_nodes:
                                if next_node==neighbor.neigh_name:
                                    ports.append(neighbor.ethid)
                                    break
                        #get next forwarding ports
                        port_str=""
                        for i in range(0, len(ports)-1):
                            port_str+=("{0}," .format(ports[i]))
                        port_str+=("{0}" .format(ports[len(ports)-1]))
                        #add UDP pkts forwarding rules, high priority
                        #condition signal will use UDP pkts, which need to travel through all paths. Thus, multi-casting implemented here.
                        out_str="""sudo ovs-ofctl add-flow {0} 'dl_type=0x0800,nw_proto=17,nw_dst={1},priority={priority},action=output:{2}'""" .format(entity, self.host_ipprefix_map[entity2], port_str, priority=GenerateRulesIntoTables.HIGH_PRIORITY)
                        out_file.write(out_str + "\n")
                        #add other IP pkts (UDP pkts exclusive) forwarding rules, low priority
                        out_str="""sudo ovs-ofctl add-flow {0} 'dl_type=0x0800,nw_dst={1},priority={priority},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{2})'""" .format(entity, self.host_ipprefix_map[entity2], port_str, priority=GenerateRulesIntoTables.LOW_PRIORITY)
                        #out_str="""sudo ovs-ofctl add-flow {0} 'dl_type=0x0800,nw_dst={1},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{2})'""" .format(entity, self.host_ipprefix_map[entity2], port_str)
                        out_file.write(out_str + "\n")
                        #add ARP pkts forwarding rules
                        out_str="""sudo ovs-ofctl add-flow {0} 'dl_type=0x0806,nw_dst={1},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{2})'""" .format(entity, self.host_ipprefix_map[entity2], port_str)
                        out_file.write(out_str + "\n")
                    else:
                        portid=1
                        neighbor_nodes=self.graph[entity]
                        for neighbor in neighbor_nodes:
                            if neighbor.neigh_name == next_nodes[0]:
                                portid=neighbor.ethid
                        #add IP pkts forwarding rules
                        out_str = """sudo ovs-ofctl add-flow {0} 'dl_type=0x0800,nw_dst={1},action=output:{2}'""" .format(entity, self.host_ipprefix_map[entity2], portid)
                        out_file.write(out_str + "\n")
                        #add ARP pkts forwarding rules
                        out_str = """sudo ovs-ofctl add-flow {0} 'dl_type=0x0806,nw_dst={1},action=output:{2}'""" .format(entity, self.host_ipprefix_map[entity2], portid)
                        out_file.write(out_str + "\n")
        out_file.close()                    
        

generator = GenerateRulesIntoTables()
generator.read_eth_id_name_topo("eth_id_name_map.txt")
generator.read_topo("b4.topo")
generator.generate_rules_for_normal_packets("rules_to_add_flow_table.sh")

#generator.read_topo_test()
#generator.get_shortest_paths_from_src_to_dst()
#generator.get_shortest_paths_from_src_to_dst_test()
