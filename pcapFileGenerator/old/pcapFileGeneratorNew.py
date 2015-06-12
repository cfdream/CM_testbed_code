
#Custom Foo Protocol Packet
message =  ('01 01 00 08'   #Foo Base Header
            '01 02 00 00'   #Foo Message (31 Bytes)
            '00 00 12 30'   
            '00 00 12 31'
            '00 00 12 32' 
            '00 00 12 33' 
            '00 00 12 34' 
            'D7 CD EF'      #Foo flags
            '00 00 12 35')     


"""----------------------------------------------------------------"""
""" Do not edit below this line unless you know what you are doing """
"""----------------------------------------------------------------"""

import sys
import binascii

# number of octest: 24
#Global header for pcap 2.4
pcap_global_header =   ('D4 C3 B2 A1'   
                        '02 00'         #File format major revision (i.e. pcap <2>.4)  
                        '04 00'         #File format minor revision (i.e. pcap 2.<4>)   
                        '00 00 00 00'     
                        '00 00 00 00'     
                        'FF FF 00 00'     
                        '01 00 00 00')

# number of octets: 16
#pcap packet header that must preface every packet
pcap_packet_header =   ('AA 77 9F 47'     
                        '90 A2 04 00'     
                        'XX XX XX XX'   #Frame Size (little endian) 
                        'YY YY YY YY')  #Frame Size (little endian)

# number of octets: 14
eth_header =   ('00 00 00 00 00 00'     #Source Mac    
                '00 00 00 00 00 00'     #Dest Mac  
                '08 00')                #Protocol (0x0800 = IP)

# number of octets: 20
ip_header =    ('45'                    #IP version and header length (multiples of 4 bytes)   
                '00'                      
                'XX XX'                 #Length - will be calculated and replaced later
                '00 00'                   
                '40 00 40'                
                '11'                    #Protocol (0x11 = UDP)          
                'YY YY'                 #Checksum - will be calculated and replaced later      
                'II II II II'           #Source IP (Default: 127.0.0.1)         
                'JJ JJ JJ JJ')          #Dest IP (Default: 127.0.0.1) 

# number of octets: 8
udp_header =   ('AA AA'                   
                'XX XX'                 #Port - will be replaced later                   
                'YY YY'                 #Length - will be calculated and replaced later        
                '00 00')
seqid_str = ('AA AA AA AA')            #seqid 
time_str = ('AA AA AA AA AA AA AA AA')
                
def getByteLength(str1):
    return len(''.join(str1.split())) / 2

def writeByteStringToFile(bytestring, filename):
    if bytestring.find('-') != -1:
        return
    bytelist = bytestring.split()  
    bytestr=''
    try:
        bytestr = binascii.a2b_hex(''.join(bytelist))
    except TypeError:
        print bytelist;
    bitout = open(filename, 'ab')
    bitout.write(bytestr)
    bitout.close()

def generateMessage(messageByteSize):
    #TEST
    #return message;
    message=''
    for i in range(messageByteSize):
        message+='00 '
    return message

def generatePacketHeaderData(ith, timestamp, srcip, dstip, srcport, dstport, messageByteSize): 
    #8bytes time field
    timeMsg = time_str.replace('AA AA AA AA AA AA AA AA', "%016x" %timestamp);
    #4 bytes id field
    idMsg = seqid_str.replace('AA AA AA AA', "%08x" %ith);
    message = generateMessage(messageByteSize)

    udp = udp_header.replace('AA AA',"%04x" %srcport)
    udp = udp.replace('XX XX',"%04x" %dstport)
    udp_len = getByteLength(timeMsg) + getByteLength(idMsg) + getByteLength(message) + getByteLength(udp_header)
    udp = udp.replace('YY YY',"%04x" %udp_len)

    ip_len = udp_len + getByteLength(ip_header)
    ip = ip_header.replace('XX XX',"%04x"%ip_len)
    ip = ip.replace('II II II II', "%08x" %srcip)
    ip = ip.replace('JJ JJ JJ JJ', "%08x" %dstip)
    checksum = ip_checksum(ip.replace('YY YY','00 00'))
    ip = ip.replace('YY YY',"%04x"%checksum)

    #print idMsg
    packetByteString = eth_header + ip + udp + idMsg + timeMsg + message
    #print udp+idMsg
    return packetByteString

def generatePCAP(traceFile, pcapfile): 
    writeByteStringToFile(pcap_global_header, pcapfile)
    #allPacketString=''
    #read the traceFile, and format it
    iFile = open(traceFile, 'r')
    i = 0
    for line in iFile:
        if i % 100000 == 0:
            print i
        items=line.split(',')
        if len(items) == 7:
            #21600000003,1035926474,752121276,80,49434,6,1460
            if items[5] == 'null':
                continue;
            i+=1
            timestamp = long(items[0])
            srcip = int(items[1])
            dstip = int(items[2])
            srcport = int(items[3])
            dstport = int(items[4])
            protocol = int(items[5])
            length = int(items[6])
            #packet
            packetByteString = generatePacketHeaderData(i, timestamp, srcip, dstip, srcport, dstport, length)
            #packetByteString = generatePacketHeaderData(i, srcip, dstip, srcport, dstport, 0)
            #print packetByteString
            #pcap packet header
            pcap_len = getByteLength(packetByteString)
            hex_str = "%08x"%pcap_len
            reverse_hex_str = hex_str[6:] + hex_str[4:6] + hex_str[2:4] + hex_str[:2]
            pcaph = pcap_packet_header.replace('XX XX XX XX',reverse_hex_str)
            pcaph = pcaph.replace('YY YY YY YY',reverse_hex_str)
            packetByteString = pcaph + packetByteString
            writeByteStringToFile(packetByteString, pcapfile)
            #allPacketString += packetByteString
            #print allPacketString

    #bytestring = pcap_global_header + allPacketString
    #print bytestring
    #writeByteStringToFile(bytestring, pcapfile)

#Splits the string into a list of tokens every n characters
def splitN(str1,n):
    return [str1[start:start+n] for start in range(0, len(str1), n)]

#Calculates and returns the IP checksum based on the given IP Header
def ip_checksum(iph):

    #split into bytes    
    words = splitN(''.join(iph.split()),4)

    csum = 0;
    for word in words:
        csum += int(word, base=16)

    csum += (csum >> 16)
    csum = csum & 0xFFFF ^ 0xFFFF

    return csum


"""------------------------------------------"""
""" End of functions, execution starts here: """
"""------------------------------------------"""

if len(sys.argv) < 3:
        print 'usage: pcapgen.py caida_trace_file pcap_output_file'
        exit(0)

generatePCAP(sys.argv[1], sys.argv[2])
