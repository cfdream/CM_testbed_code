#ifndef __GENERAL_FUNCTIONS_H__
#define __GENERAL_FUNCTIONS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

void ip_to_str(uint32_t ip, char* buffer, int buffer_len);

#endif
