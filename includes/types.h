#ifndef TYPES_H
#define TYPES_H

#ifndef CUSTOMTYPES_H
    #include <stdint.h>
    #include <stddef.h>
#else
    typedef char int8_t;
    typedef short int16_t;
    typedef long int32_t;
    typedef long long int64_t;

    typedef unsigned char uint8_t;
    typedef unsigned short uint16_t;
    typedef unsigned long uint32_t;
    typedef unsigned long long uint64_t;

    typedef unsigned long size_t;
#endif

#endif

#define PACKED __attribute__((__packed__))