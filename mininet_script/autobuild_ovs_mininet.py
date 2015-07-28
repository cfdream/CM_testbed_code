import commands
import sys
import string
import re

def kill_server():
    print 'sudo kill `cd /usr/local/var/run/openvswitch && cat ovsdb-server.pid ovs-vswitchd.pid`'
    ret,output=commands.getstatusoutput('sudo kill -9 `cd /usr/local/var/run/openvswitch && cat ovsdb-server.pid ovs-vswitchd.pid`')

def remove_module():
    print 'sudo rmmod openvswitch'
    ret,output=commands.getstatusoutput('sudo rmmod openvswitch')

def build(flag):
    kill_server()
    remove_module()

    if (flag==0):
        print './configure --with-linux=/lib/modules/`uname -r`/build'
        ret,output=commands.getstatusoutput('./configure --with-linux=/lib/modules/`uname -r`/build')
        if ret!=0:
            print output
            print 'configure error, return value=%d'%ret
            exit(ret)

    print 'make'
    ret,output=commands.getstatusoutput('make')
    if ret!=0:
        print output
        print 'make error, return value=%d'%ret
        exit(ret)

    print 'sudo make install'
    ret,output=commands.getstatusoutput('sudo make install')
    if ret!=0:
        print output
        print 'make install error, return value=%d'%ret
        exit(ret)

    print 'sudo make modules_install'
    ret,output=commands.getstatusoutput('sudo make modules_install')
    if ret!=0:
        print output
        print 'make modules_install error, return value=%d'%ret
        exit(ret)
        
    print 'sudo /sbin/modprobe openvswitch'
    ret,output=commands.getstatusoutput('sudo /sbin/modprobe openvswitch')
    if ret!=0:
        print output
        print 'modprobe error, return value=%d'%ret
        exit(ret)

def start_server():#ovsdb-server, ovs-vsctl, ovs-vswitchd, 
    print 'sudo ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock --remote=db:Open_vSwitch,Open_vSwitch,manager_options --private-key=db:Open_vSwitch,SSL,private_key --certificate=db:Open_vSwitch,SSL,certificate --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert --pidfile --detach'
    ret,output=commands.getstatusoutput('sudo ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock --remote=db:Open_vSwitch,Open_vSwitch,manager_options --private-key=db:Open_vSwitch,SSL,private_key --certificate=db:Open_vSwitch,SSL,certificate --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert --pidfile --detach')
    if ret!=0:
        print output
        print 'ovsdb-server error, return value=%d'%ret
        exit(ret)

    print 'sudo ovs-vsctl --no-wait init'
    ret,output=commands.getstatusoutput('sudo ovs-vsctl --no-wait init')
    if ret!=0:
        print output
        print 'ovs-vsctl error, return value=%d'%ret
        exit(ret)

    print 'sudo ovs-vswitchd --pidfile --detach'
    ret,output=commands.getstatusoutput('sudo ovs-vswitchd --pidfile --detach')
    if ret!=0:
        print output
        print 'ovs-vswitchd error, return value=%d'%ret
        exit(ret)
def reset_db():
    print 'sudo mkdir -p /usr/local/etc/openvswitch'
    ret,output=commands.getstatusoutput('sudo mkdir -p /usr/local/etc/openvswitch')
    if ret!=0:
        print output
        print 'mkdir error, return value=%d'%ret
        exit(ret)

    print 'ls -a /usr/local/etc/openvswitch'
    ret,output=commands.getstatusoutput('ls -a /usr/local/etc/openvswitch')
    if 'conf.db' in output.split('\n'):
        print 'sudo rm /usr/local/etc/openvswitch/conf.db'
        commands.getstatusoutput('sudo rm /usr/local/etc/openvswitch/conf.db')
    if '.conf.db.~lock~' in output.split('\n'):
        print 'sudo rm /usr/local/etc/openvswitch/.conf.db.~lock~'
        commands.getstatusoutput('sudo rm /usr/local/etc/openvswitch/.conf.db.~lock~')

    print 'sudo ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema'
    ret,output=commands.getstatusoutput('sudo ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema')
    if ret!=0:
        print output
        print 'ovsdb-tool create error, return value=%d'%ret
        exit(ret)

class Neighbor:
    def __init__(self, myPort, neighbor, neighborPort):
        self.myPort=myPort
        self.neigMachine=neighbor
        self.neigPort=neighborPort


def get_network_topo(me, hosts, machinePortId, connections, distance):
    #read the topology
    file=open("../topoNew.txt", 'r')
    lines=file.readlines()
    lines=map(lambda x:x[:-1], lines)

    #get hosts
    temphosts=lines[0].split(' ')
    for host in temphosts:
        hosts.append(host)
    #get switches
    switches=lines[1].split(' ')
    #get host-port information & connection information
    #bind NIC
    portId=0
    for i in range(2, len(lines)):
        items=lines[i].split(' ')
        if len(items) == 2:
            #get ports for hosts and switches
            #could be got after the ports are binded
            machine=items[0]
            port=items[1]
            #portId=items[2]
            if me == machine:
                key="{0} {1}" .format(machine, port)
                ret,output=commands.getstatusoutput('sudo ovs-vsctl add-port s1 {0}' .format(port))
                print('sudo ovs-vsctl add-port s1 {0}' .format(port))
                portId+=1
                machinePortId[key]=portId

        elif len(items) == 4:
            #get connection relation
            m1=items[0]
            m2=items[1]
            m1port=items[2]
            m2port=items[3]
            #add m1 into connections[m2]
            neigM1=Neighbor(m2port, m1, m1port)
            if not connections.has_key(m2):
                connections[m2]=[neigM1]
            else:
                connections[m2].append(neigM1)
            #add m2 into connections[m1]
            neigM2=Neighbor(m1port,m2,m2port)
            if not connections.has_key(m1):
                connections[m1]=[neigM2]
            else:
                connections[m1].append(neigM2)
    print "connections:"
    print connections

    #use bfs to get distance from switches to hosts
    for host in hosts:
        #record the distance from host to switches
        distance[host]={}
        distance[host][host] = 0
        #current queue
        q=set([])
        q.add(host)
        #next candidate queue
        nextQ=set([])
        #distance to host
        dist=0
        while True:
            if len(q) == 0:
                break
            dist+=1
            for machine in q:
                neighbors=connections[machine]
                for neig in neighbors:
                    neigMachine=neig.neigMachine
                    if not neigMachine in distance[host]:
                        if neigMachine not in distance[host]:
                            distance[host][neigMachine]=dist
                            nextQ.add(neigMachine)
                        elif neighbordist < distance[host][neigMachine]:
                            distance[host][neigMachine]=dist
                            nextQ.add(neigMachine)
                            
            q.clear()
            q=q.union(nextQ)
            nextQ.clear()
    print "distance"
    print distance

def start_switch_xuemei_add_rules():
    #get the name of this host
    ret,output=commands.getstatusoutput('hostname')
    me=output.split('.')[0]
    #me='s1'

    #bind the ovs
    ret,output=commands.getstatusoutput('sudo ovs-vsctl add-br s1')
    print('sudo ovs-vsctl add-br s1')

    hosts=[]
    machinePortId={}
    connections={}
    distance={}
    get_network_topo(me, hosts, machinePortId, connections, distance)
    print "hosts:{0}" .format(hosts)

    #set arp packets forwarding rule
    ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0806,actions=flood'")
    #ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0806,actions=normal'")
    print("sudo ovs-ofctl add-flow s1 'dl_type=0x0806,actions=flood'")

    neighbors=connections[me]
    for neighbor in neighbors:
        #suppose packets from neighbor=>me
        #find source host, and forward to switches further away
        neigMachine=neighbor.neigMachine
        #get my port connected to neighbor
        myPort=neighbor.myPort
        machinePort="{0} {1}" .format(me, myPort)
        print "from neighbor:{0}, to myport{1}" .format(neigMachine, neighbor.myPort)
        myPortId=machinePortId[machinePort]
        for host in hosts:
            if distance[host][me] == (distance[host][neigMachine]+1):
                #from neigh==>me, dist-1=>dist
                #forward to next neighbors, dist=>dist+1
                nextNeighbors=[]
                for neighbor2 in neighbors:
                    neigMachine2 = neighbor2.neigMachine
                    if distance[host][neigMachine2] == (distance[host][me]+1):
                        nextNeighbors.append(neighbor2)
                print "next step neighbors:{0}" .format(nextNeighbors)        
                #set up forwarding rules
                if len(nextNeighbors) > 1:
                    port_str = ""
                    for i in range(0, len(nextNeighbors)-1):
                        nextNeigh = nextNeighbors[i]
                        machinePort="{0} {1}" .format(me, nextNeigh.myPort)
                        print "nextPort:{0} {1}" .format(me, nextNeigh.myPort)
                        port_str += ("{0}," .format(machinePortId[machinePort]));
                    nextNeigh = nextNeighbors[len(nextNeighbors) - 1]
                    machinePort="{0} {1}" .format(me, nextNeigh.myPort)
                    print "nextPort:{0} {1}" .format(me, nextNeigh.myPort)
                    port_str += ("{0}" .format(machinePortId[machinePort]))
                    print "from host:{0}, forward to ports:{1}" .format(host, port_str)
                    if host=="h1" or me != "s4":
                        commandStr="sudo ovs-ofctl add-flow s1 'dl_type=0x0800,in_port={0},actions=bundle(symmetric_l4,50,hrw,ofport,slaves:{1})'" .format(myPortId, port_str)
                    else:
                        commandStr="sudo ovs-ofctl add-flow s1 'dl_type=0x0800,in_port={0},actions=flood'" .format(myPortId)
                    ret,output=commands.getstatusoutput(commandStr)
                    print(output)
                    if ret != 0:
                        print "fail:{0}" .format(commandStr)
                    else:
                        print "succ:{0}" .format(commandStr)
                else:
                    nextNeigh = nextNeighbors[len(nextNeighbors) - 1]
                    machinePort="{0} {1}" .format(me, nextNeigh.myPort)
                    print "nextPort:{0} {1}" .format(me, nextNeigh.myPort)
                    commandStr="sudo ovs-ofctl add-flow s1 'dl_type=0x0800,in_port={0},actions=output:{1}'" .format(myPortId, machinePortId[machinePort])
                    ret,output=commands.getstatusoutput(commandStr)
                    if ret != 0:
                        print "fail:{0}" .format(commandStr)
                    else:
                        print "succ:{0}" .format(commandStr)


def start_switch():
    #get the name of this host
    ret,output=commands.getstatusoutput('hostname')
    #me=output.split('.')[0]
    me='s1'

    #read the topology
    file=open("topo.txt","r")
    lines=file.readlines()
    lines=map(lambda x:x[:-1], lines)
    [nh,ns]=map(int,lines[0].split(' '))

    lines=lines[1:]
    g={}
    pre_hop={}
    for line in lines:
        [x,y,c]=line.split(' ')
        if not g.has_key(x):
            g[x]=[y]
        else:
            g[x].append(y)
        if not g.has_key(y):
            g[y]=[x]
        else:
            g[y].append(x)
    pre_hop[me]=(0,set([]))
    q=[]
    for y in g[me]:
        pre_hop[y]=(1,set([y]))
        q.append(y)
    for x in q:
        d=pre_hop[x][0]
        for y in g[x]:
            if not pre_hop.has_key(y):
                pre_hop[y]=(d+1,set([]))
                q.append(y)
            if pre_hop[y][0]==d+1:
                pre_hop[y][1].update(pre_hop[x][1])
    print "pre_hop:\n{0}" .format(pre_hop)
    
    
    ret,output=commands.getstatusoutput('sudo ovs-vsctl add-br s1')

    ret,output=commands.getstatusoutput("ifconfig")
    ips=map(lambda x: "10.0.1.%d"%x,range(1,256))
    links=map(lambda x: "10.2.0.%d"%x,range(1,256))
    h=[]
    s=[]
    #ips=['10.0.1.1','10.0.1.2','10.0.1.3','10.0.1.4']
    output=output.split('\n')
    ports={}
    eth_now=''
    prog=re.compile('inet addr:(\d+\.\d+\.\d+\.\d+)')
    for i in range(len(output)):
        line=output[i]
        if len(line)==0:
            continue
        if not line[0] in [' ','\t','\n']:
            eth_now=line.split(' ')[0]
        res=prog.search(line)
        if res!=None:
            print res.group(1)
            ports[res.group(1)]=eth_now
    print "ports:\n{0}" .format(ports)

    n_port=0
    port_id={}
    for ip in ips:
        if not ip in ports.keys():
            continue
        ret,output=commands.getstatusoutput('sudo ovs-vsctl add-port s1 %s'%ports[ip])
        n_port=n_port+1
        port_id["h%s"%ip[7:]]=n_port
        print ('sudo ovs-vsctl add-port s1 %s'%ports[ip])
    for ip in links:
        if not ip in ports.keys():
            continue
        ret,output=commands.getstatusoutput('sudo ovs-vsctl add-port s1 %s'%ports[ip])
        n_port=n_port+1
        port_id["s%s"%ip[7:]]=n_port
        print ('sudo ovs-vsctl add-port s1 %s'%ports[ip])
    print "port_id:\n{0}" .format(port_id)

    ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0806,actions=flood'")

    #add flows
    for h in range(1,nh+1):
        next_hops = list(pre_hop["h%d"%h][1]);
        next_hops_len = len(next_hops);
        print "h{0}:{1}".format(h, next_hops_len);
        if next_hops_len > 1:
            port_str = ""
            for i in range(0, next_hops_len-1):
                port_str += ("{0}," .format(port_id[next_hops[i]]));
            port_str += ("{0}" .format(port_id[next_hops[next_hops_len-1]]));
            print port_str;
            ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.{0},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{1})'" .format(h, port_str))
            print(output)
            if ret != 0:
                print("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.{0},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{1})'" .format(h, port_str))
            else:
                print("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.{0},action=bundle(symmetric_l4,50,hrw,ofport,slaves:{1})'" .format(h, port_str))
        else:
            ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,action=output:%d'"%(h,port_id[list(pre_hop["h%d"%h][1])[0]]))
            print "sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,action=output:%d'"%(h,port_id[list(pre_hop["h%d"%h][1])[0]])
    '''
    n_host=0
    n_switch=0
    for ip in ips:
        if not ip in ports.keys():
            continue
        n_host+=1
        ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,actions=output:%d'"%(ip[7:],n_host))
        print ("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,actions=output:%d'"%(ip[7:],n_host))

    for ip in links:
        if not ip in ports.keys():
            continue
        n_switch+=1
        ret,output=commands.getstatusoutput("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,actions=output:%d'"%(ip[7:],n_host+n_switch))
        print ("sudo ovs-ofctl add-flow s1 'dl_type=0x0800,nw_dst=10.0.0.%s,actions=output:%d'"%(ip[7:],n_host+n_switch))
    '''

if len(sys.argv)!=2 or not sys.argv[1] in ['0','1','2','3','4']:
    print 'usage: python autobuild.py k\n\tk=0:\tbuild from scratch\n\tk=1:\tbuild from modification\n\tk=2:\trestart\n\tk=3:\tstart_switch\n\tk=4:\tkill_switch\n'
    exit(0)


flag=0
if sys.argv[1]=='0':
    flag=0
elif sys.argv[1]=='1':
    flag=1
elif sys.argv[1]=='2':
    flag=2
elif sys.argv[1]=='3':
    flag=3
else:
    flag=4

if (flag==0 or flag==1):
    build(flag)

if (flag==0 or flag==1 or flag==2):
    reset_db()

if (flag==2):
    kill_server()
if (flag==0 or flag==1 or flag==2 or flag==3):
    start_server()

if (flag==4):
    kill_server()

#start_switch_xuemei_add_rules()

print 'succeed!'

