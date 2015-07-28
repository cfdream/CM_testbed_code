#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define OPEN_DEBUG 1
#define OPEN_NOTICE 1
#define OPEN_WARNING 1
#define OPEN_ERROR 1
#define OPEN_LOCK 0

#define DEBUG_FNAME "data/log/debug.txt"
#define NOTICE_FNAME "data/log/notice.txt"
#define WARNING_FNAME "data/log/warning.txt"
#define ERROR_FNAME "data/log/error.txt"

void DEBUG(const char* buffer);
void NOTICE(const char* buffer);
void WARNING(const char* buffer);
void ERROR(const char* buffer);
void request_mutex(pthread_mutex_t* p_mutex);
void release_mutex(pthread_mutex_t* p_mutex);

#endif
