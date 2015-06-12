#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
//#include <fstream>

#include "../public_lib/packet.h"
#include "writePacketToPcapFile.h"

//using namespace std;

#define u_int unsigned int 
#define u_short unsigned short

#define START_IP 0x0A000000
#define END_IP 0x15FFFFFF
#define SECTIONS 12

#define LINE_BUFFER_LEN 1024

#define SECTION_DELTA (END_IP-START_IP+1)/SECTIONS
#define ITH_SECTION_START_IP(i) START_IP+SECTION_DELTA*(i) //i=0,1,...,SECTIONS-1

u_int get_matched_srcip(u_int original_srcip, u_int original_start_srcip, u_int original_srcip_delta, 
                        u_int ith_matched_section/*0, 1, ..., 11*/, u_int matched_delta) {
    u_int matched_start_srcip = ITH_SECTION_START_IP(ith_matched_section);
    u_int match_srcip = (u_int)(matched_start_srcip + 1.0*(original_srcip-original_start_srcip) / original_srcip_delta * matched_delta);
    assert(match_srcip >= matched_start_srcip);
    u_int next_section_start_srcip = ITH_SECTION_START_IP(ith_matched_section+1);
    //printf("%u-%u-%u\n", matched_start_srcip, match_srcip, next_section_start_srcip);
    assert(match_srcip < next_section_start_srcip);
    return match_srcip;
}

u_int get_matched_dstip(u_int ith_section, /*0, 1, ..., 11*/
                        u_int original_dstip, u_int original_start_dstip, u_int original_dstip_delta) {
   double ratio = 1.0 * (original_dstip - original_start_dstip) / original_dstip_delta;
   ratio *= (SECTIONS-1);
   u_int ith_matched_section = (u_int) ratio;
   double matched_dist = ratio - ith_matched_section;
   if(ith_matched_section >= ith_section) {
        //donot match dstip to the current data center
        ith_matched_section += 1;   
   }
   //if (ith_matched_section == 0) {
   //    printf("dstip matched to section:%u\n", ith_matched_section);
   //}

   u_int matched_dstip = (u_int) (ITH_SECTION_START_IP(ith_matched_section) + matched_dist * SECTION_DELTA);
   assert(matched_dstip >= ITH_SECTION_START_IP(ith_matched_section));
   assert(matched_dstip < ITH_SECTION_START_IP(ith_matched_section+1));
   return matched_dstip;
}

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
    if (ith_token < 7 || p_packet->len > 2048) {
        //the line is in wrong format
        return -1;
    }
    return 0;
}

/*
int write_one_packet(ofstream*  out_file, packet_s* p_packet) {
    char buffer[LINE_BUFFER_LEN];
    char line_buffer[LINE_BUFFER_LEN];

    //timestamp, useless field
    (*out_file)<<"0,";
    //srcip
    (*out_file)<<p_packet->srcip<<",";
    //dstip
    (*out_file)<<p_packet->dstip<<",";
    //src port
    (*out_file)<<p_packet->src_port<<",";
    //dst port
    (*out_file)<<p_packet->dst_port<<",";
    //protocol
    (*out_file)<<p_packet->protocol<<",";
    //len
    (*out_file)<<p_packet->len<<endl;
    return 0;
}
*/

int main(int argc, char** argv) {
    FILE* in_file;
    //ofstream out_file;
    char* in_fname;
    char* out_fname;
    //char line_buffer[LINE_BUFFER_LEN];
    size_t line_len = 1024;
    char* line_buffer = (char*)malloc(LINE_BUFFER_LEN);

    u_int ith_section;

    //for progress visulization
    int num_lines = 0;
    int read_num_lines = 0;

    packet_s packet;
    u_int max_srcip = 0;
    u_int min_srcip = UINT_MAX;
    u_int max_dstip = 0;
    u_int min_dstip = UINT_MAX;

    //get file name
    //get ith section (means generate pcap data for ith data center)
    if (argc != 4) {
        printf("usage: pcapFielGenerator ith_data_center(1, 2, ...,12) input_packet_file out_pcap_file\n \
                Usage: generate pcap file for ith data center\n");
        return -1;
    }
    ith_section = atoi(argv[1]);
    ith_section -= 1;
    in_fname = argv[2];
    out_fname = argv[3];

    in_file = fopen(in_fname, "r");
    if (NULL == in_file) {
        printf("cannot open in_file:%s\n", in_fname);
        return -1;
    }
    //out_file.open(out_fname, std::fstream::out);

    // 1. read the file through
    // get min_ip, max_ip
    // calculate delta
    while (getline(&line_buffer, &line_len, in_file) > 0) {
        if (parse_one_line(line_buffer, &packet) < 0) {
            //the format of the line is wrong
            continue;
        }
        if (packet.srcip > max_srcip) {
            max_srcip = packet.srcip;
        }
        if (packet.srcip < min_srcip) {
            min_srcip = packet.srcip;
        }
        if (packet.dstip > max_dstip) {
            max_dstip = packet.dstip;
        }
        if (packet.dstip < min_dstip) {
            min_dstip = packet.dstip;
        }

        ++num_lines;
    }
    printf("min_srcip:%u, max_srcip:%u\n", min_srcip, max_srcip);
    printf("min_dstip:%u, max_dstip:%u\n", min_dstip, max_dstip);

    /*genearte_pcap_file*/
    init_generate_pcpa_file(out_fname);
    /*genearte_pcap_file*/

    // 2. read the file again
    // for each packet
    //     match srcip to one IP in the ith section
    //     match dstip to one IP in the other SECTIONS-1 sections
    printf("progress:%2d%%", 0);
    fseek(in_file, 0, SEEK_SET);
    while (getline(&line_buffer, &line_len, in_file) > 0) {
        ++read_num_lines;
        if (read_num_lines%1000 == 0) {
            fflush(stdout);
            printf("\b\b\b%2d%%", (int)(1.0*read_num_lines/num_lines*100));
        }

        if (parse_one_line(line_buffer, &packet) < 0) {
            //the format of the line is wrong
            continue;
        }

        //match the srcip and dstip
        u_int matched_srcip = get_matched_srcip(packet.srcip, min_srcip, max_srcip - min_srcip + 1,
                                                ith_section, SECTION_DELTA);
        u_int matched_dstip = get_matched_dstip(ith_section, packet.dstip, min_dstip, max_dstip-min_dstip+1);
        packet.srcip = matched_srcip;
        packet.dstip = matched_dstip;

        /*genearte_pcap_file*/
        generate_one_pcap_pkt(&packet);
        /*genearte_pcap_file*/
        
        //re-write the packet to another file
        //write_one_packet(&out_file, &packet);
    }

    /*genearte_pcap_file*/
    close_generate_pcpa_file();
    /*genearte_pcap_file*/
    
    free(line_buffer);
    fclose(in_file);
    //out_file.close();
    printf("\ncompleted, %d lines processed\n", read_num_lines);
}
