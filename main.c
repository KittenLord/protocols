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
#include "tcp.c"

#include "../pretty-dump/prettydump.c"

int main() {

    char *data = "ABOBA";

    struct IPv4Header *header = TCP_createPacket();
    header = TCP_setData(header, data, strlen(data));
    TCP_assignChecksum(header);

    printf(TCP_getData(header, NULL));

    prdump(header, hn16(header->totalLength), PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);
    prdump(header, hn16(header->totalLength), PRD_BINARY, PRD_ONLY_OFFSET, 1, 4, 2);

}
