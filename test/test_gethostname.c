#include <stdio.h>
#include "../public_lib/fifo/senderFIFOsManager.h"

int main() {
    char fifo_name[100];
    if (sender_get_fifo_fname(fifo_name, 100) == 0) {
        printf("fifo_name:%s\n", fifo_name);
    } else {
        printf("sender_get_fifo_fname fail\n");
    }
}
