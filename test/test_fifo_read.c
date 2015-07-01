#include "../public_lib/multi_write_one_read_fifo.h"

int main() {
    int fifo_handler = openFIFO(TEST_FIFO_FNAME);
    condition_t condition;

    readConditionFromFIFO(fifo_handler, &condition);
    printf("read:%u-%u\n", condition.srcip, condition.lost_len);

    closeFIFO(fifo_handler);
}
