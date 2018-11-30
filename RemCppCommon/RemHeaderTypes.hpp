#ifndef REMHEADERTYPES_H_
#define REMHEADERTYPES_H_

#include <inttypes.h>

#ifdef ESP8266

#import <Arduino.h>

#define funcf Serial.printf
#define debugf Serial.printf
#define infof Serial.printf
#define logf Serial.printf
#define errorf Serial.printf

#else

#include <iostream>

#define funcf printf
#define debugf printf
#define infof printf
#define logf printf
#define errorf printf

#endif

struct _RemBasicHeader ;

typedef struct _RemBasicHeader
{
    uint8_t PSize000;
    uint8_t PSize256;
    uint8_t ForwardingType;
    uint8_t HeaderType;
    uint8_t ProtobufType;

    void set_size(uint16_t size)
    {

        // funcf("pack_header_size :                \t");
        // for (uint8_t i = 0; i < size; i++)
        //     funcf("%d ", data[i]);
        // funcf("\n");
        // funcf("ENCODED SIZE : %d \n", size);

        PSize000 = (uint8_t)((size >> (8 * 0)) & 0xff);
        PSize256 = (uint8_t)((size >> (8 * 1)) & 0xff);

        // funcf("DECODED SIZE : %d \n", get_size());

    };

    uint16_t get_size()
    {
        return (int)(PSize000) | (int)(PSize256)<<8 ;
    };

} RemBasicHeader;

// typedef struct _RemDataHeaderByte
// {
//     uint8_t ForwardingType;
//     uint8_t HeaderType;
//     uint8_t ProtobufType;
//     uint8_t Data;
// } RemDataHeaderByte;

#endif /* !REMHEADERTYPES_H_ */
