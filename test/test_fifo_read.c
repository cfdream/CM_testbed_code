#include "../public_lib/multi_write_one_read_fifo.h"

int main() {
    int fifo_handler = openFIFO(TEST_FIFO_FNAME);
    if (fifo_handler < 0) {
        printf("FAIL: openFIFO %s\n", TEST_FIFO_FNAME);
        return -1;
    }
    recv_2_send_proto_t recv_2_send_proto;

    while(readConditionFromFIFO(fifo_handler, &recv_2_send_proto) == 0)
    {
        printf("read:%u-%u\n", recv_2_send_proto.srcip, recv_2_send_proto.rece_seqid);
    }

    closeFIFO(fifo_handler);
}
