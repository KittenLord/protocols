#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <unistd.h>

#include "ipv4.c"

#include "../pretty-dump/prettydump.c"

void testloop(bool);

int main(int argc, char **argv) {


    testloop(argv[1][0] == 'a');
    return 0;
}

void testloop(bool client) {

    int sock = socket(AF_PACKET, SOCK_DGRAM, hn16(ETH_P_ALL));
    printf("Created socket: %d\n", sock);

    if(client) {

        struct IPv4Header *header = createPacket();
        header->identification = hn16(0x1c46);
        printf("IDENTIFICATION: %d", hn16(header->identification));

        char *msg = "AAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHH";
        header = setData(header, strlen(msg), msg);

        header->timeToLive = 0x40;
        header->protocol = 6;

        header->sourceAddress = ipv4(172, 16, 10, 99);
        header->destinationAddress = ipv4(172, 16, 10, 12);

        header->dontFragment = 1;

        assignChecksum(header);

        prdump(header, hn16(header->totalLength), PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);

        write(sock, header, hn16(header->totalLength));
    }
    else {

        int16_t id = 0;
        struct IPv4Header *header = malloc(1);
        while(id != 69) {
            free(header);
            header = malloc(sizeof(struct IPv4Header));
            int n = read(sock, header, 4);
            if(header->version != 4) continue;

            int tl = hn16(header->totalLength);
            header = realloc(header, tl);
            int end = read(sock, (int8_t *)header, tl - 4);
            id = hn16(header->identification);

            struct IPv4Address s = header->sourceAddress;
            struct IPv4Address d = header->destinationAddress;

            printf("Received id: %d | SOURCE: %d.%d.%d.%d | DEST: %d.%d.%d.%d | END: %d\n", id, s.a, s.b, s.c, s.d, d.a, d.b, d.c, d.d, end);
        }

        prdump(header, hn16(header->totalLength), PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);
    }
}
