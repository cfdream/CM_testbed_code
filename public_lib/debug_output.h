#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ovs-thread.h"
#include "global_setting.h"
#include "common_lib.h"

const bool OPEN_DEBUG = true;
const bool OPEN_NOTICE = true;
const bool OPEN_WARNING = true;
const bool OPEN_ERROR = true;

const char DEBUG_FNAME[] =  "/tmp/log/debug.txt";
const char NOTICE_FNAME[] =  "/tmp/log/notice.txt";
const char WARNING_FNAME[] = "/tmp/log/warning.txt";
const char ERROR_FNAME[] = "/tmp/log/error.txt";

void DEBUG(const char* buffer);
void NOTICE(const char* buffer);
void WARNING(const char* buffer);
void ERROR(const char* buffer);

pthread_mutex_t debug_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t notice_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t warn_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t error_file_mutex = PTHREAD_MUTEX_INITIALIZER;

void DEBUG(const char* buffer) {
    FILE * fp;
    if (!OPEN_DEBUG) {
        return;
    }
    request_mutex(&debug_file_mutex);
    fp = fopen(DEBUG_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        release_mutex(&debug_file_mutex);
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
    release_mutex(&debug_file_mutex);
}

void NOTICE(const char* buffer) {
    FILE * fp;
    if (!OPEN_NOTICE) {
        return;
    }
    request_mutex(&notice_file_mutex);
    fp = fopen(NOTICE_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        release_mutex(&notice_file_mutex);
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
    release_mutex(&notice_file_mutex);
}

void WARNING(const char* buffer) {
    FILE * fp;
    if (!OPEN_WARNING) {
        return;
    }
    request_mutex(&warn_file_mutex);
    fp = fopen(WARNING_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        release_mutex(&warn_file_mutex);
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
    release_mutex(&warn_file_mutex);
}

void ERROR(const char* buffer) {
    FILE * fp;
    if (!OPEN_ERROR) {
        return;
    }
    request_mutex(&error_file_mutex);
    fp = fopen(ERROR_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        release_mutex(&error_file_mutex);
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
    release_mutex(&error_file_mutex);
}

#endif
