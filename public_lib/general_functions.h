#ifndef __GENERAL_FUNCTIONS_H__
#define __GENERAL_FUNCTIONS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

void ip_to_str(uint32_t ip, char* buffer, int buffer_len) {
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    char* temp = inet_ntoa(addr);
    memset(buffer, 0, buffer_len);
    memcpy(buffer, temp, strlen(temp));
    return;
}

#endif
