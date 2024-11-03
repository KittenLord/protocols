#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <unistd.h>
#include <errno.h>

#include "ipv4.c"
#include "ethernet.c"

#include "../pretty-dump/prettydump.c"

void testloop(bool);

int main(int argc, char **argv) {

    int sock = socket(AF_PACKET, SOCK_RAW, hn16(ETH_P_ALL));
    if(sock < 0) { printf("Failed to create socket\n"); exit(1); }

    if(argv[1][0] == 'a') {

        char *msg = "ABOBA___";

        struct IPv4Header *header = createPacket();
        header = setData(header, strlen(msg), msg);
        header->identification = hn16(69);
        header->timeToLive = 99;
        header->sourceAddress = ipv4(127, 0, 0, 1);
        header->destinationAddress = ipv4(127, 0, 0, 1);
        assignChecksum(header);

        struct EthernetHeader ehv = { 
            /*.sourceAddress = { .a = 0x04, .b = 0x6c, .c = 0x59, .d = 0xde, .e = 0x9f, .f = 0xb3 },*/
            .sourceAddress = { .a = 0xb3, .b = 0x9f, .c = 0xde, .d = 0x59, .e = 0x6c, .f = 0x04 },
            .destinationAddress = { .a = 0x00, .b = 0x00, .c = 0x00, .d = 0x00, .e = 0x00, .f = 0x00 },
            /*.length = packetSize*/
            .length = hn16(ETH_P_IP)
        };

        struct ifreq ifr = { .ifr_name = "wlan0" };
        int result = ioctl(sock, SIOCGIFINDEX, &ifr);
        printf("Result: %d\n", result);
        printf("Errno: %d\n", errno);

        struct sockaddr_ll sa = { .sll_family = AF_PACKET, .sll_ifindex = ifr.ifr_ifindex, .sll_protocol = hn16(ETH_P_ALL) };

        uint16_t packetSize = strlen(msg) + sizeof(struct EthernetHeader) + hn16(header->totalLength);
        struct EthernetHeader *eh = malloc(packetSize);
        *eh = ehv;
        memcpy(eh + 1, header, hn16(header->totalLength));

        result = sendto(sock, eh, packetSize, 0, (struct sockaddr *)&sa, sizeof(sa));
        printf("Result: %d\n", result);
        printf("Errno: %d\n", errno);
        return 0;
    }

    struct EthernetHeader *eh = malloc(65536);
    struct IPv4Header *header;
    while(true) {

        int nread = recv(sock, eh, 65536, 0);
        header = tryGetIPv4Header(eh);
        if(!header) continue;

        prdump(header, hn16(header->totalLength), PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);
        if(hn16(header->identification) == 69) break;
    }

    prdump(header, hn16(header->totalLength), PRD_ASCII, PRD_ONLY_OFFSET, 1, 4, 2);

}
