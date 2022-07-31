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

ForwardingType_ToRoot = 11
ForwardingType_ToParent = 11
ForwardingType_ToNeighbor = 12

HeaderType_Default = 11

PacketDataType_Default = 11



HEADER_SIZE_SMALL = 5
HEADER_SIZE_DEFAULT = 6

def new_packet_small():
    # smaller simpler packet, with 1 byte for size so 256 max packet size
    raw_packet = [
        PacketHeaderType_Small ,
        0 ,
        ForwardingType_ToRoot ,
        HeaderType_Default ,
        PacketDataType_Default ,
    ]
    set_size(len(raw_packet), raw_packet)
    enc_packet = bytearray(raw_packet)
    return enc_packet

def new_packet_default():
    # bigger packet, with 2 bytes for size so 256^2 max packet size
    raw_packet = [
        PacketHeaderType_Default ,
        0 ,
        0 ,
        ForwardingType_ToRoot ,
        HeaderType_Default ,
        PacketDataType_Default ,
    ]
    set_size(len(raw_packet), raw_packet)
    enc_packet = bytearray(raw_packet)
    return enc_packet


def new_packet_message(message_):
    if len(message_) < 250:
        packet_ = new_packet_small()
    else:
        packet_ = new_packet_default()
    packet_.extend(message_)
    set_size(len(packet_), packet_)
    return packet_

def print_packet(details_, packet_):
    header_ = get_header(packet_)
    message = packet_[len(header_):]
    size_ = get_size(packet_)
    print(f"   {details_} PACKET:{size_}|{header_}|{message}")

def get_header(packet_):
    if packet_[0] == PacketHeaderType_Small :
        return list(packet_[:HEADER_SIZE_SMALL])
    else:
        return list(packet_[:HEADER_SIZE_DEFAULT])


def set_size(size, packet_):
    if packet_[0] == PacketHeaderType_Small :
        if size > 255:
            raise Exception(f"Invalid size for the header type: {size=} {packet_=}")
        else:
            packet_[1] = size
    else :
        packet_[1] = (size >> (8 * 0)) & 0xff
        packet_[2] = (size >> (8 * 1)) & 0xff

def get_size(packet_):
    if len(packet_) < 5:
        raise Exception(f"Invalid packet {packet_=}")
    if packet_[0] == PacketHeaderType_Small :
        return packet_[1]
    else:
        return packet_[1] | packet_[2]<<8


def set_ForwardingType(packet_):
    pass

def get_ForwardingType(packet_):
    pass


def set_HeaderType(packet_):
    pass

def get_HeaderType(packet_):
    pass


def set_PacketDataType(packet_):
    pass

def get_PacketDataType(packet_):
    pass