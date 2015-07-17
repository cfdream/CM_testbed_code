#include "debug_output.h"

pthread_mutex_t debug_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t notice_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t warn_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t error_file_mutex = PTHREAD_MUTEX_INITIALIZER;

void request_mutex(pthread_mutex_t* p_mutex) {
    if (OPEN_LOCK ) {
        pthread_mutex_lock(p_mutex);
    }
}

void release_mutex(pthread_mutex_t* p_mutex) {
    if (OPEN_LOCK) {
        pthread_mutex_unlock(p_mutex);
    }
}

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
    fflush(fp);
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
    fflush(fp);
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
    fflush(fp);
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
    fflush(fp);
    fclose(fp);
    release_mutex(&error_file_mutex);
}

