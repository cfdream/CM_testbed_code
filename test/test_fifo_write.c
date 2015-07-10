#include <stdbool.h>
#include "../public_lib/multi_write_one_read_fifo.h"

int main() {
    /*
    if (createFIFOFile(TEST_FIFO_FNAME) != 0) {
        printf("createFIFOFile FAIL\n");
        return -1;
    }
    */
    int fifo_handler = openFIFO(TEST_FIFO_FNAME);
    if (fifo_handler < 0) {
        printf("FAIL:openFIFO %s\n", TEST_FIFO_FNAME);
        return -1;
    }
    recv_2_send_proto_t recv_2_send_proto;
    while (true) {
        int i = 0;
        for (; i < 100; i++) {
            recv_2_send_proto.srcip = i;
            recv_2_send_proto.rece_seqid = 12;
            writeConditionToFIFO(fifo_handler, &recv_2_send_proto);
            printf("write:%u\t%u\n", recv_2_send_proto.srcip, recv_2_send_proto.rece_seqid);
        }
        sleep(1);
    }
    closeFIFO(fifo_handler);
    return 0;
}
