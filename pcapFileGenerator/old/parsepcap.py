#from scapy.all import PcapReader
import sys
import binascii
from scapy.all import PcapReader

def ReadPcap(file):
    try:
      # scapy.utils.PcapReader
      print('Reading: %s' % file)
      pcap = PcapReader(file)
    except:
      # yes, logging.exception should be used here, but it doesn't add any value
      print('Not a valid pcap file: %s' % file)
      raise
  
    # Build a list of streams that match the search regex
    num = 0
    while pcap:
      try:
        packet = pcap.read_packet()
        if not 'IP' in packet:
            continue;
        srcip = packet['IP'].src
        dstip = packet['IP'].dst
        sport = packet[2].sport
        dport = packet[2].dport
        length = packet[2].len
        #print srcip, dstip, sport, dport, length
        #print packet.summary();
        #print binascii.hexlify(packet[2].payload['Raw'].load)
        num+=1
        if num%1000 == 0:
            print num
      except TypeError:
        print 'exception'
        break

    pcap.close()
    del pcap

if len(sys.argv) < 2:
    print 'usage: parsepcap.py a.pcap'

ReadPcap(sys.argv[1]);
