import commands
import sys
import string
import re
from collections import deque
from sets import Set

class NeighborNode:
    def __init__(self, ethid, neigh_name, neigh_ethid):
        self.ethid=ethid
        self.neigh_name=neigh_name
        self.neigh_ethid=neigh_ethid

class GenerateRulesIntoTables:
    HIGH_PRIORITY = 32769
    LOW_PRIORITY = 32768

    ONLY_SINGLE_PATH_RULE = 1
    ONLY_ECMP_PATH_RULE = 2
    BOTH_SINGLE_AND_ECMP_PATH_RULE = 3
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
        #id_name_pattern = re.compile("^\tport (\d+): (\w+-\w+)")
        id_name_pattern = re.compile("^\t\t(\w+-\w+) (\d+)/\w+")
        
        for line in lines:
            match = id_name_pattern.match(line)
            if match == None:
                continue
            #self.switch_eth_name_id_map[match.group(2)] = match.group(1)
            self.switch_eth_name_id_map[match.group(1)] = match.group(2)
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

    ##
    # @brief get all switches between src and dst
    #
    # @param src
    # @param dst
    #
    # @return 
    def get_single_path_between_nodes(self, src, dst):
        next_hops_map = self.get_reverse_pre_nodes(dst)
        single_path_nodes = []
        cur_node = src
        while True:
            next_hops = next_hops_map[cur_node]
            if len(next_hops) > 1:
                return False, []
            next_node = next_hops[0]
            if next_node == dst:
                break
            single_path_nodes.append(next_node)
            cur_node = next_node
        return True, single_path_nodes

    ##
    # @brief If there are ECMP paths between src and dst, find one disjoint switch on each ECMP path
    #
    # @param src
    # @param dst
    #
    # @return 
    def get_ecmp_paths_between_nodes(self, src, dst):
        next_hops_map = self.get_reverse_pre_nodes(dst)
        node_queue = deque([src])
        while len(node_queue) > 0:
            cur_node = node_queue.popleft()
            next_hops = next_hops_map[cur_node]
            if len(next_hops) > 1:
                return True, next_hops
                break
            if next_hops[0] != dst:
                node_queue.append(next_hops[0])
        return False, []

    def get_reverse_pre_nodes(self, dst):
        visited_name_id_map={}
        visited_name_id_map[dst]=0
        ith=0
        expand_name_list=[dst]
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
                neigh_id = visited_name_id_map[neigh_name]
                if neigh_id == now_id+1:
                    #one link: neigh_name=>now_name
                    if neigh_name not in path:
                        next_nodes=[]
                        path[neigh_name] = next_nodes
                    neigh_next_nodes=path[neigh_name]
                    neigh_next_nodes.append(now_name)
                    if neigh_name not in expand_name_list:
                        expand_name_list.append(neigh_name)
            ith+=1
        return path

    def get_reverse_pre_forward_next_nodes_test(self):
        print "get_reverse_pre_nodes for h1"
        path=self.get_reverse_pre_nodes('h1')
        print sorted(path.items(), key=lambda item:item[0])
        print "get_forward_next_nodes h1"
        path=self.get_forward_next_nodes('h1')
        print sorted(path.items(), key=lambda item:item[0])

    def generate_tasks_for_query3(self):
        '''
        Condition: high volume at sender; 
        Capture: one switch at each path for all the paths, one monitor per path
        '''
        with open("query3_task_info.txt", 'w') as out_file:
            ecmp_node_set = Set(['s4', 's6', 's10', 's11'])
            max_node_appear_times = 14
            node_appear_times_map = {}
            num_ecmp_paths = 0
            num_hosts = len(self.hosts)
            for i in range(num_hosts):
                for j in range(num_hosts):
                    src = self.hosts[i]
                    dst = self.hosts[j]
                    if src == dst:
                        continue
                    multipath_exist, disjoint_nodes_for_paths = self.get_ecmp_paths_between_nodes(src, dst)
                    if len(disjoint_nodes_for_paths) > 2:
                        continue
                    if multipath_exist:
                        node_in_set = True
                        for disjoint_node in disjoint_nodes_for_paths:
                            if disjoint_node not in ecmp_node_set:
                                node_in_set = False
                                continue
                        if not node_in_set:
                            continue

                        exceed_max_appear_time = False
                        for disjoint_node in disjoint_nodes_for_paths:
                            if disjoint_node not in node_appear_times_map:
                                node_appear_times_map[disjoint_node] = 0
                            node_appear_times_map[disjoint_node] += 1
                            if node_appear_times_map[disjoint_node] > max_node_appear_times:
                                exceed_max_appear_time = True
                                continue
                        if not exceed_max_appear_time:
                            num_ecmp_paths += 1
                            node_id_list = []
                            for disjoint_node in disjoint_nodes_for_paths:
                                node_id_list.append(disjoint_node[1:])
                            #print num_ecmp_paths, src, dst, disjoint_nodes_for_paths
                            out_file.write("{0} {1} {2} {3}\n" .format(num_ecmp_paths, src[1:], dst[1:], " ".join(node_id_list)))
            print num_ecmp_paths
            for node, times in node_appear_times_map.items():
                print node, "  ",  times
                
    def generate_tasks_for_query1(self):
        '''
        Host measures loss, 
        if the loss rate is high, 
        ask all the switches along the path to measure the volume of the high loss flow
        '''
        #TODO: filter some paths to make number of monitors on each switch similar
        with open("query1_task_info.txt", 'w') as out_file:
            max_node_appear_times = 12
            node_appear_times = {}
            num_single_path = 0
            for src in self.hosts:
                for dst in self.hosts:
                    if src == dst:
                        continue
                    #get path between 
                    single_path_exist, single_path_nodes = self.get_single_path_between_nodes(src, dst)
                    if not single_path_exist:
                        continue
                    #count node appear times
                    has_node_exceed_max_appear_time = False
                    for node in single_path_nodes:
                        if node not in node_appear_times:
                            node_appear_times[node] = 0
                        if node_appear_times[node] > max_node_appear_times:
                            has_node_exceed_max_appear_time = True
                            break
                        node_appear_times[node] += 1
                    if has_node_exceed_max_appear_time:
                        continue
                    #output one task for this path
                    num_single_path += 1
                    node_id_list = []
                    debug_node_exist = False
                    for node in single_path_nodes:
                        node_id_list.append(node[1:])
                        if node == 's10':
                            debug_node_exist = True
                    if debug_node_exist:
                        print node_appear_times['s10']
                        print num_single_path, src, dst, single_path_nodes
                    out_file.write("{0} {1} {2} {3}\n" .format(num_single_path, src[1:], dst[1:], " ".join(node_id_list)))
            for switch in self.switches:
                if switch in node_appear_times:
                    print switch, node_appear_times[switch]

generator = GenerateRulesIntoTables()
generator.read_eth_id_name_topo("eth_id_name_map.txt")
generator.read_topo("b4_topo_ip_prefixes.txt")

generator.generate_tasks_for_query3()
generator.generate_tasks_for_query1()
