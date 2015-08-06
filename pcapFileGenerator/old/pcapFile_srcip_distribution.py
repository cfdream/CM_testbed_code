import sys
def analyze(file_name):
    iFile = open(file_name, 'r')
    srcip_volume_map = {}
    dstip_volume_map = {}
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
            if dstip not in dstip_volume_map:
                dstip_volume_map[dstip] = 0

            srcip_volume_map[srcip] += length
            dstip_volume_map[dstip] += length
            #print(dstip, srcip, dstip_volume_map[dstip])
    iFile.close()
    
    oFile1 = open("srcip_volume_map.txt", 'w')
    srcip_volume_list = srcip_volume_map.iteritems()
    for k, v in sorted(srcip_volume_list, key=lambda pair: pair[1], reverse=True):
        #print(k,v)
        oFile1.write("{0}\t{1}\n" .format(k, v))
    oFile1.close()

if len(sys.argv) < 2:
        print 'usage: pcapFile_srcip_dstip_distribution.py caida_trace_file(.csv)'
        exit(0)

analyze(sys.argv[1])
