#include "../../public_lib/flow.h"
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

    //1. read fifo
    recv_2_send_proto_t recv_2_send_proto;
    while(readConditionFromFIFO(fifo_handler, &recv_2_send_proto) == 0)
    {
        printf("read:%u-%u\n", recv_2_send_proto.srcip, recv_2_send_proto.rece_seqid);

        //get the key of the flow
        flow_t* p_flow = (flow_t*)(&recv_2_send_proto);

        //get <rece volume, loss volume> for the flow
        hashtable_vl_t* flow_recePktList_map = data_warehouse_get_flow_recePktList_map();
        receV_lostV_t receV_lostV = ht_vl_get_rece_lost_volume(flow_recePktList_map, p_flow, recv_2_send_proto.rece_seqid);

        hashtable_vi_t* flow_volume_map = data_warehouse_get_flow_volume_map();
        hashtable_vi_t* flow_loss_volume_map = data_warehouse_get_flow_loss_volume_map();
        hashtable_vf_t* flow_loss_rate_map = data_warehouse_get_flow_loss_rate_map();

        //2. update flow_volume_map, flow_lost_volume_map, flow_loss_rate_map
        if (receV_lostV.received_volume + receV_lostV.lost_volume) {

            //update total volume of flow, account for receV, lostV 
            uint32_t volume = ht_vi_get(flow_volume_map, p_flow);
            volume += (receV_lostV.received_volume + receV_lostV.lost_volume);
            ht_vi_set(flow_volume_map, p_flow, volume);

            //update loss volume if necessary
            uint32_t loss_volume = ht_vi_get(flow_loss_volume_map, p_flow);
            if (receV_lostV.lost_volume) {
                loss_volume += receV_lostV.lost_volume;
                ht_vi_set(flow_loss_volume_map, p_flow, loss_volume);
            }
            //update loss rate
            float loss_rate = 1.0 * loss_volume / volume;
            ht_vf_set(flow_loss_rate_map, p_flow, loss_rate);

            //3. update target flows
            hashtable_kfs_t* target_flow_map = data_warehouse_get_target_flow_map();
            if (volume >= TARGET_FLOW_VOLUME && loss_rate >= TARGET_FLOW_LOSS_RATE) {
                //this is a target flow
                ht_kfs_set(target_flow_map, p_flow, 1);
            } else {
                //this is not a target flow.
                //to simplity, just del the flow from target_flow_map
                ht_kfs_del(target_flow_map, p_flow);
            }
        }
    }

    closeFIFO(fifo_handler);

    return NULL;
}
