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

typedef struct _RemBasicHeader
{
    uint8_t ForwardingType;
    uint8_t HeaderType;
    uint8_t ProtobufType;
} RemBasicHeader;

typedef struct _RemDataHeaderByte
{
    uint8_t ForwardingType;
    uint8_t HeaderType;
    uint8_t ProtobufType;
    uint8_t Data;
} RemDataHeaderByte;

#endif /* !REMHEADERTYPES_H_ */
