#include "../get_mininet_host_name.h"

int main() {
    char buffer[100];
    get_mininet_host_name(buffer, 100);
    printf("hostname:%s\n", buffer);
    return 0;
}
