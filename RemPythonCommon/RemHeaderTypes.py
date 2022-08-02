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


ForwardingType_Here = 11
ForwardingType_ToRoot = 12
ForwardingType_ToParent = 13
ForwardingType_ToNeighbor = 14
ForwardingType_ToNeighborToRoot = 15
ForwardingType_ToNeighborAndBack = 16
ForwardingType_Drop = 17
ForwardingType_Back = 18

# TODO change ForwardingType to Destination1 and Destination2
# packets that need to be processed in 2 nodes might be common enough to
# justify 2 bytes and shift them as the target is reached


_counter_ = 0

HEADER_ForwardingType   = _counter_ ; _counter_+=1
HEADER_PacketSize_1     = _counter_ ; _counter_+=1
HEADER_PacketSize_2     = _counter_ ; _counter_+=1
HEADER_HopsCounter      = _counter_ ; _counter_+=1
# HEADER_SendChannelId    = _counter_ ; _counter_+=1
# HEADER_RecvChannelId    = _counter_ ; _counter_+=1
HEADER_SIZE             = _counter_ ; _counter_+=1



def new_packet():
    # smaller simpler packet, with 1 byte for size so 256 max packet size
    raw_packet = [0] * HEADER_SIZE
    set_ForwardingType(raw_packet, ForwardingType_Drop)
    set_size(len(raw_packet), raw_packet)
    return raw_packet

def new_packet_message(message_):
    if isinstance(message_, str):
        message_=message_.encode()

    packet_ = new_packet()
    packet_.extend(message_)
    set_size(len(packet_), packet_)
    return packet_

def str_packet_compact(details_, packet_):
    header_, message_ = split_packet_raw(packet_)
    # message_ = bytearray(message_).decode()
    # message_ = str(message_)
    message_ = str(bytearray(message_))
    size_ = get_size(packet_)
    return f"{details_}{size_}|{header_}|{message_}"

def str_packet(details_, packet_):
    header_, message_ = split_packet_raw(packet_)
    message_ = bytearray(message_).decode()
    # message_ = str(message_)
    # message_ = str(bytearray(message_))
    size_ = get_size(packet_)
    return f"{details_}{size_}|{header_}|{message_}"

def print_packet_compact(details_, packet_):
    print(str_packet_compact(details_, packet_))

def print_packet(details_, packet_):
    print(str_packet(details_, packet_))

def split_packet_raw(packet_):
    header_ = get_header(packet_)
    message_ = packet_[len(header_):]
    return header_, message_

def split_packet(packet_):
    header_ = get_header(packet_)
    message_ = packet_[len(header_):]
    message_ = bytearray(message_).decode()
    return header_, message_

def get_header(packet_):
    return list(packet_[:HEADER_SIZE])

def get_message(packet_):
    header_ = get_header(packet_)
    message_ = packet_[len(header_):]
    return bytearray(message_).decode()

def normalize(packet_):
    set_size(len(packet_), packet_)
    return packet_

def packet_append(packet_, extra_):
    packet_.extend( extra_ )
    set_size(len(packet_), packet_)
    return packet_


def encode_send(packet_, chan_data):
    normalize(packet_)
    # set_SendChannelId(packet_, chan_data.channel_id)
    return bytearray(packet_)

def decode_recv(packet_, chan_data):
    # normalize(packet_)
    header_, message_ = split_packet_raw(packet_)
    dec_packet_ = []
    dec_packet_.extend(header_)
    dec_packet_.extend(message_)
    # set_RecvChannelId(dec_packet_, chan_data.channel_id)
    # print(f"{header_=}")
    # print(f"{message_=}")
    # print(f"{dec_packet_=}")
    return dec_packet_




def set_size(size, packet_):
    packet_[HEADER_PacketSize_1] = (size >> (8 * 0)) & 0xff
    packet_[HEADER_PacketSize_2] = (size >> (8 * 1)) & 0xff

def get_size(packet_):
    if len(packet_) < 5:
        raise Exception(f"Invalid packet {packet_=}")
    return packet_[HEADER_PacketSize_1] | packet_[HEADER_PacketSize_2] << 8


def set_ForwardingType(packet_, value_):
    packet_[HEADER_ForwardingType] = value_

def get_ForwardingType(packet_):
    return packet_[HEADER_ForwardingType]

def add_HopsCounter(packet_):
    packet_[HEADER_HopsCounter] += 1

def get_HopsCounter(packet_):
    return packet_[HEADER_HopsCounter]

# def set_SendChannelId(packet_, value_):
#     packet_[HEADER_SendChannelId] = value_

# def get_SendChannelId(packet_):
#     return packet_[HEADER_SendChannelId]

# def set_RecvChannelId(packet_, value_):
#     packet_[HEADER_RecvChannelId] = value_

# def get_RecvChannelId(packet_):
#     return packet_[HEADER_RecvChannelId]

