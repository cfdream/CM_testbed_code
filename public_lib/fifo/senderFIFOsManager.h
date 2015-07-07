/*
 * all sender side FIFO manager
 * get the FIFO handlers for all senders,
 * and then each receiver will be used to send information to different senders
 */

#ifndef __SENDER_FIFO_MANAGER__
#define __SENDER_FIFO_MANAGER__

#include "multi_write_one_read_fifo.h"

#define NUM_SENDERS 12

#define IP_MASK 0xFF000000
//ip prefix of first sender is 10/24
//ip prefix of second sender is 11/24
//...
#define FIRST_SENDER_IP_PREFIX 0x0A000000

#define GET_SENDER_IDX(ip) {(((ip&IP_MASK)>>24) - FIRST_SENDER_IP_PREFIX>>24)}

int fifo_handlers[NUM_SENDERS];

void get_sender_fifo_fname(int ith_sender, char* buffer, int buffer_len) {
    assert(buffer != NULL);
    snprintf(buffer, buffer_len, "/tmp/h%d_fifo", ith_sender);
};

int createFIFOFiles() {
    char fifo_fname[100];
    int i = 1;
    for (i = 1; i <= NUM_SENDERS; ++i) {
        get_sender_fifo_fname(i, fifo_fname, 100);
        printf("FIFO:%s\n", fifo_fname);
        if (createFIFOFile(fifo_fname) != 0) {
            printf("FAIL: createFIFOFile %s\n", fifo_fname);
            return -1;
        }
    }
    return 0;
};

/**
* @brief get the handler for all FIFO files for each sender
*
* @return 0:success, -1: fail
*/
int open_fifos() {
    char fifo_fname[100];
    int i = 1;
    for (i = 1; i <= NUM_SENDERS; ++i) {
        get_sender_fifo_fname(i, fifo_fname, 100);
        int fifo_handler = openFIFO(fifo_fname);
        if (fifo_handler < 0) {
            printf("FAIL:openFIFO for h%d\n", i);
            return -1;
        }
        fifo_fname[i] = fifo_handler;
    }
    return 0;
};


/**
* @brief get the fifo_handler of the sender the srcip belonging to
*
* @param srcip
*
* @return sender fifo handler
*/
int get_sender_fifo_handler(uint32_t srcip) {
    int fifo_idx = GET_SENDER_IDX(srcip);
    return fifo_handlers[fifo_idx];
};

#endif
