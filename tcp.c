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

#define TCP_Listen 1
#define TCP_SyncSent 2
#define TCP_SyncReceived 3
#define TCP_Established 4
#define TCP_FinWait1 5
#define TCP_FinWait2 6
#define TCP_CloseWait 7
#define TCP_Closing 8
#define TCP_LastAck 9
#define TCP_TimeWait 10
#define TCP_Closed 0
struct TcpConnection {
    uint8_t             state;

    uint32_t            sendUnack;
    uint32_t            sendNext;
    uint32_t            sendWindow;
    uint32_t            sendUrgentPointer;
    uint32_t            sendWindowUpdateSeq;
    uint32_t            sendWindowUpdateAck;
    uint32_t            sendInitialSeqNumber;

    uint32_t            recvNext;
    uint32_t            recvWindow;
    uint32_t            recvUrgentPointer;
    uint32_t            recvInitialSeqNumber;
}

#else
#endif
