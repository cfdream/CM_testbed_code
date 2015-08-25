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
* e.g., time_interval_len = 30 seconds
* then, will return at every xx:xx:00 or xx:xx:30
* It is usually less than 0.1 ms larger than xx:xx:00, or xx:xx:30
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
