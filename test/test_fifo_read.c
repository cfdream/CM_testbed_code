#include "../public_lib/fifo/multi_write_one_read_fifo.h"

int main() {
    int fifo_handler = openFIFO(TEST_FIFO_FNAME);
    if (fifo_handler < 0) {
        printf("FAIL: openFIFO %s\n", TEST_FIFO_FNAME);
        return -1;
    }
    condition_t condition;

    while(readConditionFromFIFO(fifo_handler, &condition) == 0)
    {
        printf("read:%u-%u\n", condition.srcip, condition.lost_len);
    }

    closeFIFO(fifo_handler);
}
