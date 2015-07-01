#include "../public_lib/multi_write_one_read_fifo.h"

int main() {
    /*
    if (createFIFOFile(TEST_FIFO_FNAME) != 0) {
        printf("createFIFOFile FAIL\n");
        return -1;
    }
    */
    int fifo_handler = openFIFO(TEST_FIFO_FNAME);
    condition_t condition;
    condition.srcip = 10;
    condition.lost_len = 11;
    writeConditionToFIFO(fifo_handler, &condition);
    printf("write:%u\t%u\n", condition.srcip, condition.lost_len);
    closeFIFO(fifo_handler);
    return 0;
}
