#ifndef KITTENLORD_ETHERNET_
#define KITTENLORD_ETHERNET_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "ipv4.c"

struct __attribute__((packed)) MacAddress {

    uint8_t                 a;
    uint8_t                 b;
    uint8_t                 c;
    uint8_t                 d;
    uint8_t                 e;
    uint8_t                 f;

};

struct __attribute__((packed)) EthernetHeader {

    struct MacAddress       destinationAddress;
    struct MacAddress       sourceAddress;

    uint16_t                length;

};

struct IPv4Header *tryGetIPv4Header(struct EthernetHeader *eh) {

    struct IPv4Header *header = (struct IPv4Header *)(eh + 1);
    uint8_t version = (*(uint8_t *)header) >> 4;
    if(version != 4) return NULL;
    return header;

}

#else
#endif

