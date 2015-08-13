import sys
def analyze(file_name):
    iFile = open(file_name, 'r')
    srcip_volume_map = {}
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
            srcip &= 0xFF000000
            srcip = srcip>>24
            dstip = int(items[2])
            dstip &= 0xFF000000
            dstip = dstip>>24
            length = int(items[6]) 
            if srcip not in srcip_volume_map:
                srcip_volume_map[srcip] = 0
            if dstip not in dstip_volume_map:
                dstip_volume_map[dstip] = 0
            if dstip not in dstip_pktnum_map:
                dstip_pktnum_map[dstip] = 0

            srcip_volume_map[srcip] += length
            dstip_volume_map[dstip] += length
            dstip_pktnum_map[dstip] += 1 
            #print(dstip, srcip, dstip_volume_map[dstip])
    
    oFile1 = open("{0}_srcDC_statistics.txt" .format(file_name), 'w')
    oFile2 = open("{0}_dstDC_statistics.txt" .format(file_name), 'w')
    for k, v in srcip_volume_map.iteritems():
        #print(k,v)
        oFile1.write("{0}\t{1}\n" .format(k, v))
    for k, v in dstip_volume_map.iteritems():
        #print(k,v)
        oFile2.write("{0}\t{1}\t{2}\n" .format(k, v, dstip_pktnum_map[k]))
    iFile.close()
    oFile1.close()
    oFile2.close()

if len(sys.argv) < 2:
        print 'usage: pcapFile_srcip_dstip_distribution.py caida_trace_file(.csv)'
        exit(0)

analyze(sys.argv[1])
