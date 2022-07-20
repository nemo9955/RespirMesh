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


ForwardingType_ToRoot = 10
ForwardingType_ToParent = 11
ForwardingType_ToNeighbor = 12

HeaderType_Default = 10

PacketDataType_Default = 10



HEADER_SIZE = 5

def new_packet():
    raw_packet = [
        0 ,
        0 ,
        ForwardingType_ToRoot ,
        HeaderType_Default ,
        PacketDataType_Default ,
    ]
    # print(f"{raw_packet=}")
    set_size(len(raw_packet), raw_packet)
    # return raw_packet
    enc_packet = bytearray(raw_packet)
    # print(f"{enc_packet=}")
    return enc_packet


def print_packet(packet_):
    header_ = list(packet_[:HEADER_SIZE])
    message = packet_[HEADER_SIZE:]
    size_ = get_size(packet_)
    print(f"   PACKET:{size_}|{header_}|{message}")


def decode_header(packet_):
    header_ = packet_[:HEADER_SIZE]


def set_size(size, packet_):
    packet_[0] = (size >> (8 * 0)) & 0xff
    packet_[1] = (size >> (8 * 1)) & 0xff

def get_size(packet_):
    if len(packet_) >= 2 :
        return packet_[0] | packet_[1]<<8
    else:
        # return -1
        raise Exception(f"Invalid packet {packet_=}")


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