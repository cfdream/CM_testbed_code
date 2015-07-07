#include <stdbool.h>
#include "../public_lib/fifo/multi_write_one_read_fifo.h"

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
    condition_t condition;
    while (true) {
        int i = 0;
        for (; i < 100; i++) {
            condition.srcip = i;
            condition.lost_len = 12;
            writeConditionToFIFO(fifo_handler, &condition);
            printf("write:%u\t%u\n", condition.srcip, condition.lost_len);
        }
        sleep(1);
    }
    closeFIFO(fifo_handler);
    return 0;
}
