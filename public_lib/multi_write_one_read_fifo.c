/*
 * API of FIFO for multi-writer single-reader usage
 */

#include "multi_write_one_read_fifo.h"

/**
* @brief 
*
* @param fifo_name
*
* @return 0 if success, otherwise failed
*/
int createFIFOFile(const char* fifo_name) {
    if(access(fifo_name, F_OK) != -1) {
        //file exist, remove the file
        remove(fifo_name);
    }
    //create a new FIFO file
    int ret = mkfifo(fifo_name, 0666);
    return ret;
}

/**
* @brief 
*
* @param fifo_name
*
* @return the handler of the FIFO
*/
int openFIFO(const char* fifo_name) {
    assert(fifo_name != NULL);

    //O_NONBLOCK is not set, and sizeof(recv_2_send_proto_t) is smaller, thus multiple writer is ok
    int fifo_handle = open(fifo_name, O_RDWR);
    if (fifo_handle < 0) {
        printf("Error opening FIFO file\n");
    }
    return fifo_handle;
}

void closeFIFO(int fifo_handler) {
    close(fifo_handler);
}

int writeConditionToFIFO(int fifo_handler, recv_2_send_proto_t* p_recv_2_send_proto) {
    int wlen = write(fifo_handler, p_recv_2_send_proto, sizeof(recv_2_send_proto_t));
    fsync(fifo_handler);
    if (wlen != sizeof(recv_2_send_proto_t)) {
        printf("Error write FIFO\n");
        return -1;
    }
    return 0;
}

int readConditionFromFIFO(int fifo_handler, recv_2_send_proto_t* p_recv_2_send_proto) {
    assert(p_recv_2_send_proto != NULL);
    int rlen = read(fifo_handler, p_recv_2_send_proto, sizeof(recv_2_send_proto_t));
    if (rlen != sizeof(recv_2_send_proto_t)) {
        printf("Error read FIFO\n");
        return -1;
    }
    return 0;
}

