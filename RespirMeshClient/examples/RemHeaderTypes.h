

#ifndef REMHEADERTYPES_H_
#define REMHEADERTYPES_H_

#include <inttypes.h>

typedef struct _RemBasicHeader
{
    uint8_t ForwardingType;
    uint8_t HeaderType;
    uint8_t ProtobufType;
} RemBasicHeader;

#endif /* !REMHEADERTYPES_H_ */
