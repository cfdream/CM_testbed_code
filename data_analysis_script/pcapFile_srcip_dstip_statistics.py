import sys
def analyze(file_name):
    iFile = open(file_name, 'r')
    srcip_volume_map = {}
    srcip_pktnum_map = {}
    dstip_volume_map = {}
    dstip_pktnum_map = {}
    for line in iFile:
        items=line.split(',')
        if len(items) == 7:
            #21600000003,1035926474,752121276,80,49434,6,1460
            if items[5] == 'null':
                continue;
            #timestamp = long(items[0])
            srcip = int(items[1])
            dstip = int(items[2])
            length = int(items[6]) 
            length += 58
            if srcip not in srcip_volume_map:
                srcip_volume_map[srcip] = 0
            if srcip not in srcip_pktnum_map:
                srcip_pktnum_map[srcip] = 0
            if dstip not in dstip_volume_map:
                dstip_volume_map[dstip] = 0
            if dstip not in dstip_pktnum_map:
                dstip_pktnum_map[dstip] = 0

            srcip_volume_map[srcip] += length
            srcip_pktnum_map[srcip] += 1
            dstip_volume_map[dstip] += length
            dstip_pktnum_map[dstip] += 1
            #print(dstip, srcip, dstip_volume_map[dstip])
    iFile.close()
    
    #print srcip distribution info
    oFile1 = open("{0}_srcip_flow_statistics.txt" .format(file_name), 'w')
    oFile1.write("srcip\tvolume\tpacket_num\n")
    srcip_volume_list = srcip_volume_map.iteritems()
    for k, v in sorted(srcip_volume_list, key=lambda pair: pair[1], reverse=True):
        oFile1.write("{0}\t{1}\t{2}\n" .format(k, v, srcip_pktnum_map[k]))
    oFile1.close()

    #print dstip distribution info
    oFile2 = open("{0}_dstip_flow_statistics.txt" .format(file_name), 'w')
    oFile2.write("dstip\tvolume\tpacket_num\n")
    dstip_volume_list = dstip_volume_map.iteritems()
    for k, v in sorted(dstip_volume_list, key=lambda pair: pair[1], reverse=True):
        oFile2.write("{0}\t{1}\t{2}\n" .format(k, v, dstip_pktnum_map[k]))
    oFile2.close()

if len(sys.argv) < 2:
        print 'usage: pcapFile_srcip_dstip_distribution.py caida_trace_file(.csv)'
        exit(0)

analyze(sys.argv[1])
