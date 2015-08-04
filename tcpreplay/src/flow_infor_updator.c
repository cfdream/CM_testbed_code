#include <arpa/inet.h>
#include <netinet/in.h>
#include "../../public_lib/flow.h"
#include "../../public_lib/debug_config.h"
#include "flow_infor_updator.h"

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

        hashtable_kfs_vi_t* flow_volume_map = data_warehouse_get_flow_volume_map();
        hashtable_kfs_vi_t* flow_loss_volume_map = data_warehouse_get_flow_loss_volume_map();
        hashtable_kfs_vf_t* flow_loss_rate_map = data_warehouse_get_flow_loss_rate_map();

        // 2. update <srcip> flow infor
        // flow_volume_map, flow_lost_volume_map, flow_loss_rate_map
        if (receV_lostV.received_volume + receV_lostV.lost_volume) {

            //update total volume of flow, account for receV, lostV 
            int volume = ht_kfs_vi_get(flow_volume_map, p_flow);
            if (volume < 0) {
                volume = 0;
            }
            volume += (receV_lostV.received_volume + receV_lostV.lost_volume);
            ht_kfs_vi_set(flow_volume_map, p_flow, volume);

            //update loss volume if necessary
            int loss_volume = ht_kfs_vi_get(flow_loss_volume_map, p_flow);
            if (loss_volume < 0) {
                loss_volume = 0;
            }
            if (receV_lostV.lost_volume) {
                loss_volume += receV_lostV.lost_volume;
                ht_kfs_vi_set(flow_loss_volume_map, p_flow, loss_volume);
            }
            //update loss rate
            float loss_rate = 1.0 * loss_volume / volume;
            ht_kfs_vf_set(flow_loss_rate_map, p_flow, loss_rate);

            //3. update target flows
            hashtable_kfs_vi_t* target_flow_map = data_warehouse_get_target_flow_map();
            if (volume >= TARGET_FLOW_VOLUME && loss_rate >= TARGET_FLOW_LOSS_RATE) {
                //this is a target flow
                ht_kfs_vi_set(target_flow_map, p_flow, 1);
            } else {
                //this is not a target flow.
                //to simplity, just del the flow from target_flow_map
                ht_kfs_vi_del(target_flow_map, p_flow);
            }

            if (ENABLE_DEBUG && recv_2_send_proto.srcip == DEBUG_SRCIP && recv_2_send_proto.dstip == DEBUG_DSTIP &&
                recv_2_send_proto.src_port == DEBUG_SPORT && recv_2_send_proto.dst_port == DEBUG_DPORT) {
                char src_str[100];
                char dst_str[100];
                ip_to_str(recv_2_send_proto.srcip, src_str, 100);
                ip_to_str(recv_2_send_proto.dstip, dst_str, 100);

                printf("receiver=>sender: flow[%s-%s-%u-%u-%u-volume:%d-lossVolume:%d-rate:%f]\n", 
                    src_str, dst_str, 
                    recv_2_send_proto.src_port, recv_2_send_proto.dst_port, recv_2_send_proto.rece_seqid,
                    volume, loss_volume, loss_rate);
            }
        }
    }

    closeFIFO(fifo_handler);
    printf("end flow_infor_updator\n");

    return NULL;
}
