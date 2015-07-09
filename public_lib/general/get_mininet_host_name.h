#ifndef __GET_MININET_HOST_NAME_H__
#define __GET_MININET_HOST_NAME_H__

int get_mininet_host_name(char* buffer, int buffer_len) {
    //1. get NIC name
    char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
    char *dev;			/* The device to sniff on */
    dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
       fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
       return -1;
    }

    //2. get hostname
    char* slash_ptr = strchr(dev, '-');
    if (slash_ptr == NULL) {
        printf("mininet NIC format not right. It should be 'hx-ethx'\n");
        return -1;
    }
    memcpy(buffer, dev, min(strlen(dev), buffer_len));
    buffer[slash_ptr-dev] = '\0';
    return 0;
}

#endif
