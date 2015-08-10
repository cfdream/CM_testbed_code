#include "../public_lib/senderFIFOsManager.h"
#include "srcip_dstip_to_b4_srcip_dstip.h"
#include "writePacketToPcapFile.h"

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int parse_one_line(char* line_buffer, packet_s* p_packet) {
    assert(p_packet != NULL);

    int ith_token = 0;
    char** tokens = str_split(line_buffer, ',');
    int i;

    if (tokens) {
        for (i = 0; *(tokens + i); i++)
        {
            ith_token += 1;
            if (ith_token == 2) {
                p_packet->srcip = strtoul(*(tokens+i), NULL, 10);
            } else if (ith_token == 3) {
                p_packet->dstip = strtoul(*(tokens+i), NULL, 10);
            } else if (ith_token == 4) {
                p_packet->src_port = (u_short)strtol(*(tokens+i), NULL, 10);
            } else if (ith_token == 5) {
                p_packet->dst_port = (u_short)strtol(*(tokens+i), NULL, 10);
            } else if (ith_token == 6) {
                p_packet->protocol = (u_short)strtol(*(tokens+i), NULL, 10);
            } else if (ith_token == 7) {
                p_packet->len = strtoul(*(tokens+i), NULL, 10);
            }
            //printf("month=[%s]\n", *(tokens + i));
            free(*(tokens + i));
        }
        //printf("\n");
        free(tokens);
    }
    if (ith_token < 7 || p_packet->len > MAX_PAYLOAD_SIZE) {
        //the line is in wrong format
        return -1;
    }
    return 0;
}

int init(char* out_fpath) {
    if (init_srcip_dstip_to_b4_srcip_dstip() ) {
        return -1;
    }

    if (init_write_pkts_to_files(out_fpath)) {
        return -1;
    }
    return 0;
}

void destory() {
    destory_srcip_dstip_to_b4_srcip_dstip();
    close_write_pkts_to_files();
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("usage: pcapFielGenerator csv_input_filepath pcap_out_filepath\n \
                e.g., pcapFielGenerator ~/workspace/caida_data/input/ ~/workspace/caida_data/output\n \
                Usage: generate pcap file for all data centers\n");
        return -1;
    }

    FILE* in_file;
    char* in_fpath;
    char* out_fpath;
    char in_fname[LINE_BUFFER_LEN];
    size_t line_len = 1024;
    char* line_buffer = (char*)malloc(LINE_BUFFER_LEN);
    int read_num_lines = 0;
    int fileno;
    in_fpath = argv[1];
    out_fpath = argv[2];

    if (init(out_fpath) < 0) {
        printf("FAIL init()\n");
        return -1;
    }

    packet_s packet;
    fileno = START_FILE_NO - DELTA_FILE_NO;
    while (true) {
        /* 1. open one following file in csv_input_filepath */
        fileno += DELTA_FILE_NO;
        printf("%d\n", fileno);
        snprintf(in_fname, LINE_BUFFER_LEN, "%s/equinix-sanjose.dirA.20120920-%d.UTC.anon.pcap.csv", in_fpath, fileno);
        in_file = fopen(in_fname, "r");
        if (NULL == in_file) {
            printf("cannot open in_file:%s\n", in_fname);
            break;
        }

        /* 2. for each line in one pcap file */
        while (getline(&line_buffer, &line_len, in_file) > 0) {
            ++read_num_lines;
            if (read_num_lines%1000000 == 0) {
                fflush(stdout);
                printf("%2d*10^4 lines read\n", read_num_lines/10000);
            }

            /* 2.1 parse the packet */
            if (parse_one_line(line_buffer, &packet) < 0) {
                //the format of the line is wrong
                continue;
            }

            /* 2.2 get the srcip Nodeid, mapped srcip */
            uint32_t srcip_mapped_nodeid = 0;
            uint32_t srcip_mapped_srcip = 0;
            get_mapped_info(packet.srcip, &srcip_mapped_nodeid, &srcip_mapped_srcip);
            /* 2.3 get the dstip Nodeid, mapped dstip */
            uint32_t dstip_mapped_nodeid = 0;
            uint32_t dstip_mapped_dstip = 0;
            get_mapped_info(packet.dstip, &dstip_mapped_nodeid, &dstip_mapped_dstip);
            //printf("mapped srcip:%08x-%08x-dstip:%08x-%08x-node-%d-%d\n", packet.srcip, srcip_mapped_srcip, packet.dstip, dstip_mapped_dstip, srcip_mapped_nodeid, dstip_mapped_nodeid);

            /* 2.4 write the packet into related pcap file */
            if (!(GET_SENDER_IDX(srcip_mapped_srcip) ^ GET_SENDER_IDX(dstip_mapped_dstip))) {
                //mapped srcip and dstip are in the same node, no need to write to pcap file.
                // i.e., one sender should not send packets to itself.
                continue;
            }
            packet.srcip = srcip_mapped_srcip;
            packet.dstip = dstip_mapped_dstip;
            generate_one_pcap_pkt(&packet, srcip_mapped_nodeid);
        }
    }

    printf("unique ip num:%lu\n", unique_ip_num);
    printf("\ncompleted, %d lines processed\n", read_num_lines);

    destory();

    return 0;
}
