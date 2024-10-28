#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_ether.h>

#include "ipv4.c"

int main() {

    char *test = "Hello, World!";

    struct IPv4Header *header = createPacket();
    struct IPv4OptionType opt = { .isCopied = 0, .class = 0, .number = 1 };

    header = addOption(header, opt, 0, 0);
    header = setData(header, strlen(test), test);

    int sfd = socket(AF_PACKET, SOCK_RAW, ETH_P_ALL);
    printf("%d\n", sfd);

    return 0;
}
