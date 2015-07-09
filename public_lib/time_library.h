#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

void print_current_time_with_ms (void)
{
    long ms;    // Milliseconds
    time_t s;   // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = spec.tv_nsec / 1.0e6; // Convert nanoseconds to milliseconds

    printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",
           (intmax_t)s, ms);
}


/**
* @brief postpone till the next time interval starts, then return
* e.g., time_interval_len = 30 seconds
* then, will return at every xx:xx:00 or xx:xx:30
* It is usually less than 0.1 ms larger than xx:xx:00, or xx:xx:30
*
* @param time_interval_len : length of time interval in seconds. It should be dividable by 60 (1min)
*/
int get_next_interval_start(int time_interval_secs) {
    struct timespec sleep_len;
    sleep_len.tv_sec = 0;
    sleep_len.tv_nsec = 1000; //check the time every 1us = 1e3 nanoseconds

    const struct timespec* c_sleep_len = &sleep_len;

    struct timespec spec;
    while (true) {
        clock_gettime(CLOCK_REALTIME, &spec);
        if (((intmax_t)spec.tv_sec) % time_interval_secs != 0) {
            break;
        }
        nanosleep(c_sleep_len, NULL);
    }

    while (true) {
        clock_gettime(CLOCK_REALTIME, &spec);
        if (((intmax_t)spec.tv_sec) % time_interval_secs == 0) {
            break;
        }
        nanosleep(c_sleep_len, NULL);
    }
    uint64_t sec = (intmax_t)((time_t)spec.tv_sec);
    printf("Current time: %lu seconds, %ld nanosec since the Epoch\n",
           sec, spec.tv_nsec);
    return sec;
}
