#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../public_lib/cm_experiment_setting.h"
#include "../../public_lib/flow.h"
#include "../../public_lib/debug_config.h"
#include "../../public_lib/general_functions.h"
#include "flow_infor_updator.h"

extern cm_experiment_setting_t cm_experiment_setting;

void* flow_infor_update(void* param_ptr) {
    /*---- 1. read FIFO to get received seqid for each flow ----*/
    /*---- 2. update flow information, i.e., total_volume, loss_volume, loss_rate ----*/
    /*---- 3. update target flows ----*/
    //get fifo_name
    char fifo_fname[100];
    if (sender_get_fifo_fname(fifo_fname, 100) != 0) {
        printf("FAIL:get_sender_fifo_name\n");
        return NULL;
    }
    //open fifo
    int fifo_handler = openFIFO(fifo_fname);
    if (fifo_handler < 0) {
        printf("FAIL: openFIFO %s\n", fifo_fname);
        return NULL;
    }
    printf("SUCC-openFIFO:%s\n", fifo_fname);

    //1. read fifo
    int total_lost_pkt_num = 0;
    int pre_print_lost_pkt_num_times = 0;
    recv_2_send_proto_t recv_2_send_proto;
    while(readConditionFromFIFO(fifo_handler, &recv_2_send_proto) == 0)
    {
        //printf("read:%u-%u\n", recv_2_send_proto.srcip, recv_2_send_proto.rece_seqid);

        //get the key of the 5-tuple flow
        //flow_t* p_flow = (flow_t*)(&recv_2_send_proto);
        flow_t flow;
        flow.srcip = recv_2_send_proto.srcip;
        flow.dstip = recv_2_send_proto.dstip;
        flow.src_port = recv_2_send_proto.src_port;
        flow.dst_port = recv_2_send_proto.dst_port;
        flow.protocol = recv_2_send_proto.protocol;
        flow_t* p_flow = &flow;

        //get <rece volume, loss volume> for the flow
        hashtable_vl_t* flow_recePktList_map = data_warehouse_get_flow_recePktList_map();
        receV_lostV_t receV_lostV = ht_vl_get_rece_lost_volume(flow_recePktList_map, p_flow, recv_2_send_proto.rece_seqid);
        if (receV_lostV.lost_pkt_num > 100) {
            printf("one time lost pkt num:%d, %u,%u,%u,%u\n", receV_lostV.lost_pkt_num, p_flow->srcip, p_flow->dstip, p_flow->src_port, p_flow->dst_port);
        }
        total_lost_pkt_num += receV_lostV.lost_pkt_num;

        // 2. update <srcip> flow infor
        if (receV_lostV.lost_volume > 0) {
            update_flow_loss_volume(p_flow, receV_lostV.lost_volume);
        }

        // flow_volume_map, flow_lost_volume_map, flow_loss_rate_map
        if (receV_lostV.received_volume + receV_lostV.lost_volume) {
            if (ENABLE_DEBUG && recv_2_send_proto.srcip == DEBUG_SRCIP && recv_2_send_proto.dstip == DEBUG_DSTIP &&
                recv_2_send_proto.src_port == DEBUG_SPORT && recv_2_send_proto.dst_port == DEBUG_DPORT) {
                char src_str[100];
                char dst_str[100];
                ip_to_str(recv_2_send_proto.srcip, src_str, 100);
                ip_to_str(recv_2_send_proto.dstip, dst_str, 100);

                printf("receiver=>sender: flow[%s-%s-%u-%u-%u-volume:%d-lossVolume:%d]\n", 
                    src_str, dst_str, 
                    recv_2_send_proto.src_port, recv_2_send_proto.dst_port, recv_2_send_proto.rece_seqid,
                    receV_lostV.received_volume, receV_lostV.lost_volume);
            }
            if (ENABLE_DEBUG && 
                (total_lost_pkt_num > pre_print_lost_pkt_num_times*NUM_LOST_PKTS_TO_DEBUG) &&
                (total_lost_pkt_num / NUM_LOST_PKTS_TO_DEBUG != pre_print_lost_pkt_num_times)) {
                pre_print_lost_pkt_num_times= total_lost_pkt_num / NUM_LOST_PKTS_TO_DEBUG;
                printf("lost packets:%d\n", total_lost_pkt_num);
            }
        }
    }

    closeFIFO(fifo_handler);
    printf("end flow_infor_updator\n");

    return NULL;
}
