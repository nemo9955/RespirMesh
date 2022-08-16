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
ForwardingType_ToEdges = 19

# TODO change ForwardingType to Destination1 and Destination2
# packets that need to be processed in 2 nodes might be common enough to
# justify 2 bytes and shift them as the target is reached


PacketPriority_SuperLow  = 50
PacketPriority_Low       = 70
PacketPriority_Medium    = 100
PacketPriority_High      = 110
PacketPriority_Critical  = 130
PacketPriority_Realtime  = 150

ContentsType_RemInternal            = 20
ContentsType_RemMeshTopo            = 50
ContentsType_RemPingPong            = 51
ContentsType_RemMakeClient          = 52
ContentsType_RemMaintenance         = 100
ContentsType_External               = 110

_counter_ = 0

HEADER_ForwardingType   = _counter_ ; _counter_+=1
HEADER_PacketPriority   = _counter_ ; _counter_+=1
HEADER_ContentsType     = _counter_ ; _counter_+=1
HEADER_ContentsMeta     = _counter_ ; _counter_+=1
HEADER_PacketSize_1     = _counter_ ; _counter_+=1
HEADER_PacketSize_2     = _counter_ ; _counter_+=1
HEADER_HopsCounter      = _counter_ ; _counter_+=1
HEADER_SIZE             = _counter_ ; _counter_+=1



def new_packet():
    # smaller simpler packet, with 1 byte for size so 256 max packet size
    raw_packet = [0] * HEADER_SIZE
    set_ForwardingType(raw_packet, ForwardingType_Drop)
    set_PacketPriority(raw_packet, PacketPriority_Medium)
    set_ContentsType(raw_packet, ContentsType_External)
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
    if isinstance(extra_, str):
        extra_ = extra_.encode()
    packet_.extend( extra_ )
    normalize(packet_)
    return packet_

def clone(packet_):
    # return packet_.copy()
    return list(packet_)

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

def set_PacketPriority(packet_, value_):
    packet_[HEADER_PacketPriority] = value_

def get_PacketPriority(packet_):
    return packet_[HEADER_PacketPriority]

def set_ContentsType(packet_, value_):
    packet_[HEADER_ContentsType] = value_

def get_ContentsType(packet_):
    return packet_[HEADER_ContentsType]

def set_ContentsMeta(packet_, value_):
    packet_[HEADER_ContentsMeta] = value_

def get_ContentsMeta(packet_):
    return packet_[HEADER_ContentsMeta]


