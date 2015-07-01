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
