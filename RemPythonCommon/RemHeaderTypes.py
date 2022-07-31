#!/usr/bin/env python3




# typedef struct _RemBasicHeader
# {
#     uint8_t PSize000;
#     uint8_t PSize256;
#     uint8_t ForwardingType;
#     uint8_t HeaderType;
#     uint8_t ProtobufType;
#     void set_size(uint16_t size)
#         PSize000 = (uint8_t)((size >> (8 * 0)) & 0xff);
#         PSize256 = (uint8_t)((size >> (8 * 1)) & 0xff);
#     uint16_t get_size()
#         return (int)(PSize000) | (int)(PSize256)<<8 ;
# }

PacketHeaderType_Small = 11
PacketHeaderType_Default = 20

ForwardingType_Here = 11
ForwardingType_ToRoot = 12
ForwardingType_ToParent = 13
ForwardingType_ToNeighbor = 14
ForwardingType_ToNeighborToRoot = 15

HeaderType_Default = 11

PacketDataType_Default = 11


HEADER_PacketHeaderType_FIRST = 0
HEADER_ForwardingType_FIRST = 1
HEADER_HeaderType_FIRST = 2
HEADER_PacketDataType_FIRST = 3
HEADER_SIZE_FIRST = 4

HEADER_SIZE_SMALL = 5
HEADER_SIZE_DEFAULT = 6

def new_packet_small():
    # smaller simpler packet, with 1 byte for size so 256 max packet size
    raw_packet = [
        PacketHeaderType_Small ,
        ForwardingType_ToNeighbor ,
        HeaderType_Default ,
        PacketDataType_Default ,
        0 ,
    ]
    set_size(len(raw_packet), raw_packet)
    return raw_packet

def new_packet_default():
    # bigger packet, with 2 bytes for size so 256^2 max packet size
    raw_packet = [
        PacketHeaderType_Default ,
        ForwardingType_ToNeighbor ,
        HeaderType_Default ,
        PacketDataType_Default ,
        0 ,
        0 ,
    ]
    set_size(len(raw_packet), raw_packet)
    return raw_packet


def new_packet_message(message_):
    if isinstance(message_, str):
        message_=message_.encode()

    if len(message_) < 250:
        packet_ = new_packet_small()
    else:
        packet_ = new_packet_default()
    packet_.extend(message_)
    set_size(len(packet_), packet_)
    return packet_

def print_packet_compact(details_, packet_):
    header_, message_ = split_packet(packet_)
    # message_ = bytearray(message_).decode()
    # message_ = str(message_)
    message_ = str(bytearray(message_))
    size_ = get_size(packet_)
    print(f"{details_}{size_}|{header_}|{message_}")

def print_packet(details_, packet_):
    header_, message_ = split_packet(packet_)
    message_ = bytearray(message_).decode()
    # message_ = str(message_)
    # message_ = str(bytearray(message_))
    size_ = get_size(packet_)
    print(f"{details_}{size_}|{header_}|{message_}")

def split_packet(packet_):
    header_ = get_header(packet_)
    message_ = packet_[len(header_):]
    return header_, message_

def get_header(packet_):
    if packet_[0] == PacketHeaderType_Small :
        return list(packet_[:HEADER_SIZE_SMALL])
    else:
        return list(packet_[:HEADER_SIZE_DEFAULT])

def get_message(packet_):
    header_ = get_header(packet_)
    message_ = packet_[len(header_):]
    return message_
    # return str(message_)


def encode(packet_):
    set_size(len(packet_), packet_)
    return bytearray(packet_)

def decode(packet_):
    # set_size(len(packet_), packet_)
    header_, message_ = split_packet(packet_)
    dec_packet_ = []
    dec_packet_.extend(header_)
    dec_packet_.extend(message_)
    # print(f"{header_=}")
    # print(f"{message_=}")
    # print(f"{dec_packet_=}")
    return dec_packet_


def set_PacketHeaderType(packet_, value_):
    packet_[HEADER_PacketHeaderType_FIRST] = value_

def get_PacketHeaderType(packet_):
    return packet_[HEADER_PacketHeaderType_FIRST]

def set_ForwardingType(packet_, value_):
    packet_[HEADER_ForwardingType_FIRST] = value_

def get_ForwardingType(packet_):
    return packet_[HEADER_ForwardingType_FIRST]

def set_HeaderType(packet_, value_):
    packet_[HEADER_HeaderType_FIRST] = value_

def get_HeaderType(packet_):
    return packet_[HEADER_HeaderType_FIRST]

def set_PacketDataType(packet_, value_):
    packet_[HEADER_PacketDataType_FIRST] = value_

def get_PacketDataType(packet_):
    return packet_[HEADER_PacketDataType_FIRST]

def set_size(size, packet_):
    if packet_[0] == PacketHeaderType_Small :
        if size > 255:
            raise Exception(f"Invalid size for the header type: {size=} {packet_=}")
        else:
            packet_[HEADER_SIZE_FIRST] = size
    else :
        packet_[HEADER_SIZE_FIRST] = (size >> (8 * 0)) & 0xff
        packet_[HEADER_SIZE_FIRST+1] = (size >> (8 * 1)) & 0xff

def get_size(packet_):
    if len(packet_) < 5:
        raise Exception(f"Invalid packet {packet_=}")
    if packet_[0] == PacketHeaderType_Small :
        return packet_[HEADER_SIZE_FIRST]
    else:
        return packet_[HEADER_SIZE_FIRST] | packet_[HEADER_SIZE_FIRST+1] << 8
