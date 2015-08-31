#ifndef __TIME_LIBRARY_H__
#define __TIME_LIBRARY_H__

#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

void print_current_time_with_ms (void);

/**
* @brief postpone till the next time interval starts, then return
* e.g., time_interval_len = 500 milliseconds
* then, will return at the next millisecond whose value mod 500 == 0
* delta(returned value, expected value) < 1ms
*
* @param time_interval_len : length of time interval in seconds.
*/
uint64_t get_next_sec_interval_start(int time_interval_secs);

/**
* @brief similar to get_next_sec_interval_start(). The differene is that here the granularity is millisecond,
*
* @param time_interval_msecs
*
* @return 
*/
uint64_t get_next_interval_start(uint32_t time_interval_msecs);

#endif
