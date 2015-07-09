#include "senderFIFOsManager.h"

/**
* @brief for receiver, get the fifo name of one sender
*
* @param ith_sender
* @param buffer
* @param buffer_len
*/
void get_sender_fifo_fname(int ith_sender, char* buffer, int buffer_len) {
    assert(buffer != NULL);
    snprintf(buffer, buffer_len, "/tmp/h%d_fifo", ith_sender);
}


/**
* @brief for sender: get its fifo name.
* 1. get_mininet_host_name
* 2. sender_host_name_fifo is the fifo for the sender
*
* @param buffer
* @param buffer_len
*
* @return 0-succ, -1-fail
*/
int sender_get_fifo_fname(char* buffer, int buffer_len ) {
    //1. get hostname
    char hostname[100];
    if (get_mininet_host_name(hostname, 100) != 0) {
        printf("FAIL:get_mininet_host_name\n");
        return -1;
    }
    
    //2. get fifo name
    snprintf(buffer, buffer_len, "%s_fifo", hostname);

    return 0;
}

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
}

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
}


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
