#include "time_library.h"

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
* @param time_interval_len : length of time interval in miniseconds.
*/
uint64_t get_next_interval_start(uint32_t time_interval_msecs) {
    struct timespec sleep_len;
    sleep_len.tv_sec = 0;
    sleep_len.tv_nsec = 1000; //check the time every 1us = 1e3 nanoseconds
    //sleep_len.tv_nsec = 1000000; //check the time every 1ms = 1e6 nanoseconds

    const struct timespec* c_sleep_len = &sleep_len;

    struct timespec spec;
    while (true) {
        clock_gettime(CLOCK_REALTIME, &spec);
        //1s = 10^3 msec, 1 ns = 10^-6 msec
        uint64_t msec = (intmax_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
        //printf("msec:%lu\n", msec);
        if (msec % time_interval_msecs != 0) {
            break;
        }
        nanosleep(c_sleep_len, NULL);
    }

    while (true) {
        clock_gettime(CLOCK_REALTIME, &spec);
        //1s = 10^3 msec, 1 ns = 10^-6 msec
        uint64_t msec = (intmax_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
        //printf("msec:%lu\n", msec);
        if (msec % time_interval_msecs == 0) {
            break;
        }
        nanosleep(c_sleep_len, NULL);
    }
    uint64_t msec = (intmax_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
    printf("Current time: %lu milliseconds since the Epoch\n", msec);
    return msec;
}
