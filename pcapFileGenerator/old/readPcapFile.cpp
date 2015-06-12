#include <stdio.h>
#include <string.h>
#include <unistd.h>

union Num4bytes{
    char buffer[4];
    unsigned int num;
};

union Num2bytes{
    char buffer[2];
    unsigned short num;
};

union Num8bytes{
    char buffer[8];
    unsigned long long num;
};

class ReadPcapFile{
    public:
    static const int GlobalHeaderLen = 24;
    static const int pcapHeaderLen = 16;
    static const int ethHeaderLen = 14;
    static const int ipHeaderLen = 20;
    static const int udpHeaderLen = 8;

    void swapChars(char* pChar1, char* pChar2){
        char temp = *pChar1;
        *pChar1 = *pChar2;
        *pChar2 = temp;
    }

    void swapOrder(Num8bytes &num) {
        swapChars(&num.buffer[0], &num.buffer[7]);
        swapChars(&num.buffer[1], &num.buffer[6]);
        swapChars(&num.buffer[2], &num.buffer[5]);
        swapChars(&num.buffer[3], &num.buffer[4]);
    }

    void swapOrder(Num4bytes &num) {
        swapChars(&num.buffer[0], &num.buffer[3]);
        swapChars(&num.buffer[1], &num.buffer[2]);    
    }

    void swapOrder(Num2bytes &num) {
        swapChars(&num.buffer[0], &num.buffer[1]);
    }

    void readData(FILE* fp, long long offset, char* buffer, int size, long long &fileSize) {
        while (offset >= fileSize) {
            printf("%lld-%lld\n", fileSize, offset);
            fseek(fp, 0L, SEEK_END);
            fileSize = ftell(fp);
            printf("end of file\n");
            sleep(1);
        }
        fseek(fp, offset, SEEK_SET);
        fread(buffer, size, 1, fp);
        /*
        while (true) {
            fseek(fp, offset, SEEK_SET);
            if ( fread(buffer, size, 1, fp) == 1  && !feof(fp)) {
                break;
            } else {
                printf("offset-%lld\n", offset);
                printf("end of file\n");
                sleep(1);
            }
        }
        */
    }

    void read(const char* fileName) {
        FILE* fp = fopen(fileName, "rb");
        if (NULL == fp) {
            printf("open %s failed\n", fileName);
            return;
        }
        fseek(fp, 0L, SEEK_END);
        long long fileSize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        
        /*
        fseek(fp, 1384L, SEEK_SET);
        char buffer[1000];
        memset(buffer, sizeof(buffer), 0);
        int ret = fread(buffer, 1000, 1, fp);
        printf("ret:%d\n", ret);
        for(int i = 0; i < 1000; ++i) {
            printf("%02x ", (unsigned int)buffer[i]);
        }
        */

        long long offset = GlobalHeaderLen;
        int cnt = 0;
        Num4bytes seqid;
        while (true) {
            if (++cnt % 100000 == 0) {
                printf("%d-%u\n", cnt, seqid.num);
            }

            //---pcapHeader Packet(ethHeader, ipHeader, udpHeader, data)
            //--pcap packet header
            offset += 8;
            unsigned int pcapLen;
            readData(fp, offset, (char*)(&pcapLen), 4, fileSize);
            //printf("offset-%lld, pcaplen:%d\n", offset, pcapLen);

            //--eth header
            unsigned int mac1, mac2, mac3;
            offset += 8;
            readData(fp, offset, (char*)(&mac1), 4, fileSize);
            offset += 4;
            readData(fp, offset, (char*)(&mac2), 4, fileSize);
            offset += 4;
            readData(fp, offset, (char*)(&mac3), 4, fileSize);
            if (mac1 !=0 || mac2 != 0 || mac3 != 0) {
                printf("offset-%lld\n", offset);
                printf("not fake packet\n");
                //not faked packet, ignore
                offset += pcapLen - 8;
                continue;
            }

            //--ip header
            offset += 6;
            //srcip
            offset += 12;
            Num4bytes srcip;
            readData(fp, offset, (char*)(&srcip), sizeof(srcip), fileSize);
            swapOrder(srcip);

            //dstip
            offset += 4;
            Num4bytes dstip;
            readData(fp, offset, (char*)(&dstip), sizeof(dstip), fileSize);
            swapOrder(dstip);

            //srcport
            offset += 4;
            Num2bytes srcport;
            readData(fp, offset, (char*)(&srcport), sizeof(srcport), fileSize);
            swapOrder(srcport);

            //dstport
            offset += 2;
            Num2bytes dstport;
            readData(fp, offset, (char*)(&dstport), sizeof(dstport), fileSize);
            swapOrder(dstport);

            //payload length
            offset += 2;
            Num2bytes len;
            readData(fp, offset, (char*)(&len), sizeof(len), fileSize);
            swapOrder(len);

            //start of payload: seq id
            offset += 4;
            readData(fp, offset, (char*)(&seqid), sizeof(seqid), fileSize);
            swapOrder(seqid);

            //timestamp: offset has not +=, this is to make sure "offset += (pcapLen- 42)" workes correctly
            Num8bytes timestamp;
            readData(fp, offset+4, (char*)(&timestamp), sizeof(timestamp), fileSize);
            swapOrder(timestamp);

            //end of payload: end of packet 
            //offset += (len.num - 8);
            offset += (pcapLen- 42);

            //printf("%llu,%u,%u,%u,%u,%d - %d\n", timestamp.num, srcip.num, dstip.num, srcport.num, dstport.num, len.num, seqid.num);
        }

        fclose(fp);
    }

    void readSeqid(const char* fileName) {
        FILE* fp = fopen(fileName, "rb");
        if (NULL == fp) {
            printf("open %s failed\n", fileName);
            return;
        }
        fseek(fp, 0L, SEEK_END);
        long long fileSize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        long long offset = GlobalHeaderLen;
        int cnt = 0;
        Num4bytes seqid;
        while (true) {
            if (++cnt % 100000 == 0) {
                printf("%d-%u\n", cnt, seqid.num);
            }

            //---pcapHeader Packet(ethHeader, ipHeader, udpHeader, data)
            //--pcap packet header
            offset += 8;
            unsigned int pcapLen;
            readData(fp, offset, (char*)(&pcapLen), 4, fileSize);
            //printf("offset-%d, len:%d\n", offset, pcapLen);

            //--eth header
            unsigned int mac1, mac2, mac3;
            offset += 8;
            readData(fp, offset, (char*)(&mac1), 4, fileSize);
            offset += 4;
            readData(fp, offset, (char*)(&mac2), 4, fileSize);
            offset += 4;
            readData(fp, offset, (char*)(&mac3), 4, fileSize);
            if (mac1 !=0 || mac2 != 0 || mac3 != 0) {
                printf("offset-%lld\n", offset);
                printf("not fake packet\n");
                //not faked packet, ignore
                offset += pcapLen - 8;
                continue;
            }

            //start of payload: seq id
            offset += 34;
            readData(fp, offset, (char*)(&seqid), sizeof(seqid), fileSize);
            swapOrder(seqid);
            
            //end of payload: end of packet 
            //offset += (len.num - 8);
            offset += (pcapLen- 42);

            printf("%d\n", seqid.num);
        }

        fclose(fp);
    }
};

int main(int argc, char* argv[]) {

    ReadPcapFile reader;
    if (argc < 2) {
        printf("a.out pcapfile\n");
    }
    printf("%s-%s\n", argv[0], argv[1]);
    //reader.read(argv[1]);
    reader.readSeqid(argv[1]);
    return 0;
}
