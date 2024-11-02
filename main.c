#include <stdio.h>

#include "ipv4.c"

#include "../pretty-dump/prettydump.c"

int main() {

    char *msg = "Hello, World!";

    struct IPv4Header *header = createPacket();
    struct IPv4OptionType opt = { .isCopied = 0, .class = 0, .number = 1 };

    printf("%d\n", sizeof(struct IPv4Header));

    header->_reserved = 1;
    header->dontFragment = 1;
    header->moreFragments = 1;

    setFragmentOffset(header, 7000);

    uint16_t offset = getFragmentOffset(header);
    printf("Offset %d\n", offset);

    header->timeToLive = 0xFF;
    header->protocol = 6;

    header = addOption(header, opt, 0, 0);
    header = setData(header, strlen(msg), msg);

    prdump(header, hn16(header->totalLength), PRD_HEX, PRD_ONLY_OFFSET, 1, 4, 2);
    prdump(header, hn16(header->totalLength), PRD_ASCII, PRD_ONLY_OFFSET, 1, 4, 2);

    return 0;
}
