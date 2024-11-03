#ifndef KITTENLORD_TCP__
#define KITTENLORD_TCP__

#include <stdint.h>
#include "ipv4.c"

struct __attribute__((packed)) TcpPseudoHeader {
    struct IPv4Address  sourceAddress;
    struct IPv4Address  destinationAddress;

    int8_t              _reserved;
    uint8_t             protocol;
    uint16_t            tcpLength;

    struct TcpHeader    rest;
}

struct __attribute__((packed)) TcpHeader {

    uint16_t            sourcePort;
    uint16_t            destinationPort;

    uint32_t            sequenceNumber;

    uint32_t            ackNumber;

    unsigned int        dataOffset : 4;
    unsigned int        _reserved : 6;

    unsigned int        urgent : 1;
    unsigned int        ack : 1;
    unsigned int        push : 1;
    unsigned int        reset : 1;
    unsigned int        sync : 1;
    unsigned int        final : 1;

    uint16_t            window;

    uint16_t            checksum;
    uint16_t            urgentPointer;
};

#else
#endif
