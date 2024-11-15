#ifndef KITTENLORD_TCP__
#define KITTENLORD_TCP__

#include <stdint.h>
#include <stdbool.h>
#include "ipv4.c"

struct __attribute__((packed)) TcpHeader {

    uint16_t            sourcePort;
    uint16_t            destinationPort;

    uint32_t            sequenceNumber;

    uint32_t            ackNumber;

    unsigned int        _reserved : 4;
    unsigned int        dataOffset : 4;

    unsigned int        final : 1;
    unsigned int        sync : 1;
    unsigned int        reset : 1;
    unsigned int        push : 1;
    unsigned int        ack : 1;
    unsigned int        urgent : 1;

    unsigned int        __reserved : 2;

    uint16_t            window;

    uint16_t            checksum;
    uint16_t            urgentPointer;
};

struct __attribute__((packed)) TcpPseudoHeader {
    struct IPv4Address  sourceAddress;
    struct IPv4Address  destinationAddress;

    int8_t              _reserved;
    uint8_t             protocol;
    uint16_t            tcpLength;
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
};

struct TcpClient {
    int                 socket;
}

struct TcpHeader *TCP_getHeader(struct IPv4Header *header) {
    return (struct TcpHeader *)getData(header, NULL);
}

uint8_t *TCP_getData(struct IPv4Header *header, size_t *length) {
    struct TcpHeader *tcpheader = TCP_getHeader(header);
    uint8_t *data = (uint8_t *)tcpheader + tcpheader->dataOffset*4;
    int headersLength = (header->length * 4) + (tcpheader->dataOffset * 4);
    if(length) *length = hn16(header->totalLength) - headersLength;
    return data;
}

struct IPv4Header *TCP_setData(struct IPv4Header *header, uint8_t *data, size_t length) {
    struct TcpHeader *tcpheader = TCP_getHeader(header);
    int headersLength = (header->length * 4) + (tcpheader->dataOffset * 4);
    header = realloc(header, headersLength + length);
    header->totalLength = hn16(headersLength + length);
    tcpheader = TCP_getHeader(header);
    uint8_t *dest = (uint8_t *)tcpheader + tcpheader->dataOffset*4;
    memmove(dest, data, length);

    return header;
}

struct IPv4Header *TCP_createPacket() {
    struct IPv4Header *header = createPacket();
    header->protocol = 6;
    struct TcpHeader tcpheader = { .dataOffset = 5 };
    header = setData(header, (uint8_t *)&tcpheader, sizeof(struct TcpHeader));
    return header;
}

void TCP_assignChecksum(struct IPv4Header *header) {
    assignChecksum(header);
    struct TcpHeader *tcpheader = TCP_getHeader(header);
    tcpheader->checksum = 0;
    struct TcpPseudoHeader ph = { 
        .sourceAddress = header->sourceAddress,
        .destinationAddress = header->destinationAddress, 
        .protocol = 6,
        .tcpLength = hn16(header->totalLength) - header->length*4,
    };

    uint16_t checksum = calculateChecksum((uint16_t *)&ph, sizeof(struct TcpPseudoHeader), 0);
    checksum = calculateChecksum((uint16_t *)tcpheader, tcpheader->dataOffset*2, ~hn16(checksum));
    tcpheader->checksum = checksum;
}

bool createTcpClient(struct TcpClient *client, uint16_t port) {
    int sock = socket(AF_PACKET, SOCK_DGRAM, hn16(ETH_P_ALL));
    if(sock < 0) goto error;

    // FIXME: how tf does bind work
    if(!bind(sock, 0, 0)) goto error;

    *client = (struct TcpClient){
        socket = sock
    };

    return true;

error:
    close(sock);
    return false;
}

void TCP_connect(struct TcpClient *client) {

}

#else
#endif
