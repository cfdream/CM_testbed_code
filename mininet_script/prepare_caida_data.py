#!/usr/bin/python

import commands
import sys

class CaidaData():
    NUM_FILE_PER_HOST = 1
    NUM_HOSTS=12
    START_FILE_NO=130000
    DELTA_FILE_NO=100

    def process(self, s_host_idx, e_host_idx):
        fileno = CaidaData.START_FILE_NO-CaidaData.DELTA_FILE_NO+(s_host_idx-1)*CaidaData.NUM_FILE_PER_HOST
        for id_host in range(1, CaidaData.NUM_HOSTS+1):
            #for each host
            filenames=[]
            for id_file in range(1, CaidaData.NUM_FILE_PER_HOST+1):
                fileno = fileno + CaidaData.DELTA_FILE_NO
                #file name
                filename="equinix-sanjose.dirA.20120920-{0}.UTC.anon.pcap" .format(fileno)
                print filename

                #download
                download_cmd="wget https://data.caida.org/datasets/passive-2012/equinix-sanjose/20120920-130000.UTC/{0}.gz --user=xuemeili@usc.edu --password=caida" .format(filename)
                commands.getstatusoutput(download_cmd)
                print "SUCC:{0}" .format(download_cmd)

                #extract
                extract_cmd="sh -x ./script.sh {0}" .format(filename)
                commands.getstatusoutput(extract_cmd)
                print "SUCC:{0}" .format(extract_cmd)

                filenames.append(filename)
            #cat all extracted files into one file
            concated_file = "h{0}.concated.csv" .format(id_host)
            for filename in filenames:
                concate_cmd = "cat {0}.csv >> {1}" .format(filename, concated_file)
                commands.getstatusoutput(concate_cmd)
            print "SUCC: concatate"

            #generate pcap file from the concatated file
            pcap_file="h{0}.pcap" .format(id_host)
            pcap_cmd="../CM_testbed_code/pcapFileGenerator/pcapFileGenerator {0} {1} {2}" .format(id_host, concated_file, pcap_file)
            commands.getstatusoutput(pcap_cmd)
            print "SUCC: {0}" .format(pcap_cmd)

            #remove middle temp files
            for filename in filenames:
                remove_cmd = "rm {0}.csv" .format(filename)
                commands.getstatusoutput(remove_cmd)
                remove_cmd = "rm {0}.gz" .format(filename)
                commands.getstatusoutput(remove_cmd)
            commands.getstatusoutput("rm {0}" .format(concated_file))
            print "remove succ"

if len(sys.argv) !=3:
    print "usage: python prepare_caida_data.py s_host_idx[0-12] e_host_idx[0-12]\n"
    exit(0)
s_host_idx = int(sys.argv[1])
e_host_idx = int(sys.argv[2])
if s_host_idx > e_host_idx:
    print "s_host_idx > e_host_idx, s_host_idx should be <= e_host_idx\n"
    exit(0)
if s_host_idx <=0 or s_host_idx > CaidaData.NUM_HOSTS or e_host_idx <= 0 or e_host_idx > CaidaData.NUM_HOSTS:
    print "s_host_idx, e_host_idx should be within [1,12]\n"
    exit(0)
    
caidadata=CaidaData()
caidadata.process(s_host_idx, e_host_idx)
