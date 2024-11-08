#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

#include "ipv4.c"
#include "ethernet.c"

#include "../pretty-dump/prettydump.c"

int createSocket() {
    int sock = socket(AF_PACKET, SOCK_DGRAM, hn16(ETH_P_ALL));

    // struct ifreq ifr = { .ifr_name = "wlan0" };
    // int result = ioctl(sock, SIOCGIFINDEX, &ifr);
    // setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));

    return sock;
}

struct IPv4Header *waitForIPv4Packet(int sock, bool (*predicate)(struct IPv4Header *, void *), void *arg) {
    uint8_t data[65535];
    struct IPv4Header *header;

    while(true) {
        struct sockaddr_ll src;
        socklen_t srclen = sizeof(src);

        int result = recvfrom(sock, data, sizeof(data), 0, (struct sockaddr *)&src, &srclen);
        header = (struct IPv4Header *)data;
        if(header->version == 4 && predicate(header, arg)) break;
        memset(data, 0, 65535);
    }

    uint16_t totalLength = hn16(header->totalLength);
    header = calloc(1, totalLength);
    memcpy(header, data, totalLength);
    return header;
}

bool __constant(struct IPv4Header *header, void *_) { return true; }
struct IPv4Header *waitForAnyIPv4Packet(int sock) {
    return waitForIPv4Packet(sock, __constant, 0);
}

bool checkForIdentification69(struct IPv4Header *header, void *_) {
    return hn16(header->identification) == 69;
}

void mainServer() {
    int sock = createSocket();

    printf("Socket number: %d\n", sock);
    if(sock < 0) return;

    struct IPv4Header *header;

    header = waitForIPv4Packet(sock, checkForIdentification69, 0);
        
    // header = waitForAnyIPv4Packet(sock);

    prdump(header, 50, PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);
}

int sendIpPacket(int sock, struct IPv4Header *header) {
    // TODO: bad hardcode
    struct ifreq ifr = { .ifr_name = "wlan0" };
    int result = ioctl(sock, SIOCGIFINDEX, &ifr);
    if(result < 0) return result;

    struct sockaddr_ll sa = { .sll_family = AF_PACKET, .sll_ifindex = ifr.ifr_ifindex, .sll_protocol = hn16(ETH_P_ALL) };
    // TODO: manual fragmentation?
    result = sendto(sock, header, hn16(header->totalLength), 0, (struct sockaddr *)&sa, sizeof(sa));
    if(result < 0) return result;

    return result;
}

void mainClient() {
    int sock = createSocket();
    printf("Socket number: %d\n", sock);
    if(sock < 0) return;

    uint8_t *data = "ABOBA___";
    uint8_t length = 8;

    struct IPv4Header *header = createPacket();
    header->timeToLive = 100;
    // header->sourceAddress = ipv4(127, 0, 0, 1);
    header->destinationAddress = ipv4(55, 55, 0, 1);
    setData(header, data, length);
    header->identification = hn16(69);
    assignChecksum(header);
    prdump(header, 50, PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);

    int result = sendIpPacket(sock, header);
}

int main(int argc, char **argv) {

    if(!strcmp("server", argv[1])) {
        mainServer();
    }
    else if(!strcmp("client", argv[1])) {
        mainClient();
    }
    else { printf("KILL YOURSELF\n"); return 1; }
    return 0;
}
