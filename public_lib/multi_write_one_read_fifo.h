#ifndef __MULTI_WRITE_ONE_READ_FIFO_H__
#define __MULTI_WRITE_ONE_READ_FIFO_H__
/*
 * API of FIFO for multi-writer single-reader usage
 */

#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "receiver_2_sender_proto.h"

#define TEST_FIFO_FNAME "data/fifo_test"

/**
* @brief 
*
* @param fifo_name
*
* @return 0 if success, otherwise failed
*/
int createFIFOFile(const char* fifo_name);

/**
* @brief 
*
* @param fifo_name
*
* @return the handler of the FIFO
*/
int openFIFO(const char* fifo_name);

void closeFIFO(int fifo_handler);

int writeConditionToFIFO(int fifo_handler, recv_2_send_proto_t* p_recv_2_send_proto);

int readConditionFromFIFO(int fifo_handler, recv_2_send_proto_t* p_recv_2_send_proto);

#endif
