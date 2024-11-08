#ifndef KITTENLORD_IPV4_
#define KITTENLORD_IPV4_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "ethernet.c"

#define to(ty, val) (*(ty *)&val)

struct __attribute__((packed)) IPv4TypeOfService {
    unsigned int                precedence : 3;
    unsigned int                isLowDelay : 1;
    unsigned int                isHighThoughput : 1;
    unsigned int                isHighReliability : 1;
    unsigned int                _reserved : 2;
};

struct __attribute__((packed)) IPv4Flags {
    unsigned int                _reserved : 1;
    unsigned int                dontFragment : 1;
    unsigned int                moreFragments : 1;
};

struct __attribute__((packed)) IPv4Address {
    uint8_t                     a;
    uint8_t                     b;
    uint8_t                     c;
    uint8_t                     d;
};
struct IPv4Address ipv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    return (struct IPv4Address){ .a = a, .b = b, .c = c, .d = d };
}

#define IPV4_OPTION_CLASS_CONTROL 0
// #define _IPV4_OPTION_CLASS_RESERVED 1
#define IPV4_OPTION_CLASS_DEBUG 2
// #define __IPV4_OPTION_CLASS_RESERVED 3
struct __attribute__((packed)) IPv4OptionType {
    unsigned int                isCopied : 1;
    unsigned int                class : 2;
    unsigned int                number : 5;
};

#define MIN_IPV4_HEADER_LENGTH 5
struct __attribute__((packed)) IPv4Header {
    // version has to take the most significant 4 bits of the octet, hence the order
    unsigned int                length : 4;
    unsigned int                version : 4;

    struct IPv4TypeOfService    typeOfService;
    int16_t                     totalLength;

    uint16_t                    identification;

    unsigned int                fragmentOffsetHi : 5;

    unsigned int                _reserved : 1;
    unsigned int                dontFragment : 1;
    unsigned int                moreFragments : 1;

    unsigned int                fragmentOffsetLo : 8;

    int8_t                      timeToLive;
    int8_t                      protocol;
    int16_t                     checksum;

    struct IPv4Address          sourceAddress;
    struct IPv4Address          destinationAddress;
};

bool isLittleEndian() {
    int16_t n = 1;
    return *(int8_t *)&n;
}

int16_t hn16(int16_t n) {
    if(!isLittleEndian()) return n;
    return ((n >> 8) & 0x00FF) | ((n << 8) & 0xFF00);
}

int32_t hn32(int32_t n) {
    if(!isLittleEndian()) return n;
    return ((n >> 8*3) & 0x000000FF) | 
           ((n >> 8*2) & 0x0000FF00) |
           ((n << 8*2) & 0x00FF0000) |
           ((n << 8*3) & 0xFF000000) ;
}

void setFragmentOffset(struct IPv4Header *header, uint16_t offset) {
    header->fragmentOffsetLo = offset & 0xFF;
    header->fragmentOffsetHi = (offset >> 8) & 0b11111;
}

uint16_t getFragmentOffset(struct IPv4Header *header) {
    return (header->fragmentOffsetHi << 8) | header->fragmentOffsetLo;
}

uint16_t calculateChecksum(uint16_t *ptr, size_t length, uint16_t init) {
    uint16_t sum = init;
    while(length--) {
        uint32_t psum = sum + hn16(*ptr);
        sum = (psum + ((psum >> 16) & 1)) & 0xFFFF;
        ptr++;
    }
    return hn16(~sum);
}

void assignChecksum(struct IPv4Header *header) {
    header->checksum = 0;
    uint16_t checksum = calculateChecksum((uint16_t *)header, header->length * 2, 0);
    header->checksum = checksum;
}

bool hasOptions(struct IPv4Header *header) {
    return header->length > MIN_IPV4_HEADER_LENGTH;
}

bool hasData(struct IPv4Header *header) {
    return (header->length * 4) < (hn16(header->totalLength));
}

int findHeaderOptionsOffset(struct IPv4Header *header) {
    if(!hasOptions(header)) return (header->length * 4); 

    int8_t *hb = (int8_t *)header;
    int current = MIN_IPV4_HEADER_LENGTH * 4;

    while(current < header->length * 4) {
        int8_t value = *(hb + current);

        // idk if i wanna actually use this macro, but it looks cool
        struct IPv4OptionType opt = to(struct IPv4OptionType, value);

        if(opt.class == 0 && opt.number == 0) return current; // the terminator of the options list
        if(opt.class == 0 && opt.number == 1) { current++; continue; } // the noop option, but may be not the last

        int optionLength = *(hb + current + 1);
        current += optionLength;
    }

    return current; // we exited the boundary
}

struct IPv4Header *addOption(struct IPv4Header *header, struct IPv4OptionType optionType, int8_t optionLength, int8_t *optionData) {
    int oldLength = header->length;
    int oldTotalLength = hn16(header->totalLength);

    int insertOffset = findHeaderOptionsOffset(header);
    int newLength = insertOffset + (optionLength == 0 ? 1 : optionLength);
    while(newLength % 4 != 0) newLength++;

    newLength /= 4;
    int newTotalLength = oldTotalLength + (newLength - oldLength)*4;

    if(newLength >= oldLength) header = realloc(header, newTotalLength);
    header->length = newLength;
    header->totalLength = hn16(newTotalLength);

    if(hasData(header)) memmove((int8_t *)header + newLength*4, (int8_t *)header + oldLength*4, oldTotalLength - oldLength*4);

    *((int8_t *)header + insertOffset++) = *(int8_t *)&optionType;
    if(!optionLength || !optionData) return header;

    *((int8_t *)header + insertOffset++) = optionLength;
    for(int i = 0; i < optionLength - 2; i++) {
        *((int8_t *)header + insertOffset++) = optionData[i];
    }

    while(insertOffset < header->length*4) {
        *((int8_t *)header + insertOffset++) = 0;
    }

    return header;
}

struct IPv4Header *setData(struct IPv4Header *header, int8_t *data, int16_t size) {
    int16_t newTotalLength = header->length*4 + size;
    header = realloc(header, newTotalLength);
    header->totalLength = hn16(newTotalLength);
    int8_t *dest = (int8_t *)header + header->length*4;
    memmove(dest, data, size);
    return header;
}

uint8_t *getData(struct IPv4Header *header, size_t *length) {
    int headerLength = header->length * 4;
    if(length) *length = hn16(header->totalLength) - headerLength;
    return (uint8_t *)header + headerLength;
}

struct IPv4Header *createPacket() {
    struct IPv4Header *header = calloc(1, sizeof(struct IPv4Header));
    header->version = 4;
    header->length = MIN_IPV4_HEADER_LENGTH;
    header->totalLength = hn16(MIN_IPV4_HEADER_LENGTH*4);

    srand(time(NULL));
    header->identification = rand() % 0xFFFF; 
    header->timeToLive = 100;

    return header;
}

#endif
