#!/usr/bin/env python3






RemOrchestrator = None
RemHeaderTypes = None
log = None
EasyDict = None

def set_orchestrator(set_value_):
    global RemOrchestrator
    global RemHeaderTypes
    global log
    global EasyDict
    RemOrchestrator = set_value_
    RemHeaderTypes = set_value_.RemHeaderTypes
    log = set_value_.log
    EasyDict = RemOrchestrator.utils.EasyDict




def route_packet(packet):
    # print(f" -X-  AMHERE RemRouter route_packet {RemOrchestrator.RemHardware.time_milis()}")
    # process_packet needs to be called before to ensure packet is updated for current node
    message_ = RemHeaderTypes.get_message(packet)


    if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToRoot:
        if RemOrchestrator.orch_data.is_root == True:
            return

        for ch_data in RemOrchestrator.get_ch_closer_to_root():
            ch_data.client_logic.send_raw(ch_data, packet)
            return # TODO get the closest to root not just the first closest

    if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_Here:
        return

    if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_Drop:
        return

    if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToEdges:
        for ch_data in RemOrchestrator.get_ch_farther_from_root():
            ch_data.client_logic.send_raw(ch_data, packet)
        return

    for ch_data in RemOrchestrator.get_ch_clients():
        # TODO check if linked to ROOT !!!!!!!!!!!
        ch_data.client_logic.send_raw(ch_data, packet)



def process_packet(packet_wrapper_):
    if isinstance(packet_wrapper_, tuple):
        packet, server_data = packet_wrapper_
    else:
        packet = packet_wrapper_

    RemHeaderTypes.add_HopsCounter(packet)

    if RemHeaderTypes.get_ContentsType(packet) == RemHeaderTypes.ContentsType_RemMakeClient1:
        RemOrchestrator.process_make_client_type_1(packet, server_data)
        send_ping()
        return

    if RemHeaderTypes.get_ContentsType(packet) == RemHeaderTypes.ContentsType_RemPingPong:
        check_ping_pong(packet, server_data)
        return

    if RemHeaderTypes.get_ContentsType(packet) == RemHeaderTypes.ContentsType_RemMeshTopo:
        # RemHeaderTypes.print_packet("mesh topo ",packet)
        check_mesh_topo(packet, server_data)
        return


    # ForwardingType needs to change as it passes from node to node

    if RemOrchestrator.orch_data.is_root == True :
        if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToRoot:
            RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_Here)
        elif RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
            RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToRoot)

    if RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_Back:
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToRoot)
    elif RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToNeighbor:
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToParent:
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet) == RemHeaderTypes.ForwardingType_ToNeighborAndBack:
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_Back)

    # RemHeaderTypes.print_packet("after process_packet",packet)

    route_packet(packet)



ContentsMeta_SendMeshTopo     = 1
ContentsMeta_ReturnMeshTopo   = 2


def send_mesh_topo():
    # processed in  RemOrchestrator.append_mesh_topo
    if RemOrchestrator.orch_data.is_root != True:
        raise Exception(f"send_mesh_topo should be called only in root/server node: {RemOrchestrator}")

    device_id = RemOrchestrator.RemHardware.device_id()
    for ch_data in RemOrchestrator.get_ch_clients():
        message_ = f":{device_id}:"
        packet = RemHeaderTypes.new_packet_message(message_)
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToNeighbor)
        RemHeaderTypes.set_PacketPriority(packet, RemHeaderTypes.PacketPriority_Medium)
        RemHeaderTypes.set_ContentsType(packet, RemHeaderTypes.ContentsType_RemMeshTopo)
        RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_SendMeshTopo)
        RemHeaderTypes.normalize(packet)
        ch_data.client_logic.send_raw(ch_data, packet)
        # RemHeaderTypes.print_packet(f"send_mesh_topo~ {ch_data.uuid}  ",packet)


def append_mesh_topo(packet, server_data):
    # MADE IN send_mesh_topo
    if RemHeaderTypes.get_ContentsType(packet) != RemHeaderTypes.ContentsType_RemMeshTopo:
        raise Exception(f"append_mesh_topo needs ContentsType_RemMeshTopo: {packet=} {server_data=} ")

    message_ = RemHeaderTypes.get_message(packet)
    device_id = RemOrchestrator.RemHardware.device_id()

    if f":{device_id}:" in message_:
        return

    recv_dev_id = server_data.device_id

    # this node is new in the topo list
    hops_header = RemHeaderTypes.get_HopsCounter(packet)
    hops_message = message_.count(":") - 1
    if hops_header != hops_message:
        log.warn(f"Different values {hops_header=} {hops_message=} !!!!!!!!!!!!!!!!!!!!!!!!")
    # RemHeaderTypes.print_packet(f" i am {device_id=} {hops_header=} {hops_message=} mesh topo ",packet)

    add_me_ = f"{device_id}:"
    RemHeaderTypes.packet_append(packet, add_me_)

    RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToNeighbor)
    for ch_data in RemOrchestrator.get_ch_clients():
        ch_data.client_logic.send_raw(ch_data, packet)

    # RemHeaderTypes.print_packet(f"append_mesh_topo~  {server_data.uuid}  ",packet)

    route_packet(packet)

    process_mesh_topo(packet, server_data)

def return_mesh_topo(packet):
    # send mesh_topo to root

    if RemHeaderTypes.get_ContentsType(packet) != RemHeaderTypes.ContentsType_RemMeshTopo:
        raise Exception(f"append_mesh_topo needs ContentsType_RemMeshTopo: {packet=} {server_data=} ")
    if RemOrchestrator.orch_data.is_root == True:
        return

    # print(f"! AMHERE RemRouter return_mesh_topo")
    # RemHeaderTypes.print_packet(f"return_mesh_topo~got~  ",packet)

    RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToRoot)
    RemHeaderTypes.set_PacketPriority(packet, RemHeaderTypes.PacketPriority_Medium)
    RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_ReturnMeshTopo)
    RemHeaderTypes.normalize(packet)
    # RemHeaderTypes.print_packet(f"return_mesh_topo~send~ ",packet)
    route_packet(packet)

def check_mesh_topo(packet, server_data):
    if RemHeaderTypes.get_ContentsType(packet) != RemHeaderTypes.ContentsType_RemMeshTopo:
        raise Exception(f"check_mesh_topo needs ContentsType_RemMeshTopo: {packet=} {server_data=} ")

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_ReturnMeshTopo:
        if RemHeaderTypes.get_HopsCounter(packet) > 200 :
            RemHeaderTypes.print_packet(f"check_mesh_topo~ReturnMeshTopo~ {server_data.device_id} {server_data.uuid} ",packet)
        if RemHeaderTypes.get_HopsCounter(packet) > 220 :
            return ####################################### FIXME !!!!!!!!!!!!!!!!!

        if RemOrchestrator.orch_data.is_root == True:
            process_mesh_topo(packet, server_data)
        else:
            route_packet(packet)
        return

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_SendMeshTopo:
        append_mesh_topo(packet, server_data)
        return_mesh_topo(packet)
        return

    log.warn(RemHeaderTypes.str_packet("Mesh Topo case not trated ", packet))

def process_mesh_topo(packet, server_data):
    if RemHeaderTypes.get_ContentsType(packet) != RemHeaderTypes.ContentsType_RemMeshTopo:
        raise Exception(f"process_mesh_topo needs ContentsType_RemMeshTopo: {packet=} {server_data=} ")

    full_message_ = RemHeaderTypes.get_message(packet)
    root_hops = full_message_.split(":")
    counted_hops = 0

    for did_ in root_hops:
        if not did_: continue
        RemOrchestrator.set_device_root_distance(did_, counted_hops)
        counted_hops+=1

    # RemHeaderTypes.print_packet(f"process_mesh_topo~ {server_data.uuid}  ",packet)



ContentsMeta_SendPing     = 1
ContentsMeta_RecvPing     = 2
ContentsMeta_SendPong     = 3
ContentsMeta_RecvPong     = 4
ContentsMeta_PingPongRoot = 5

def send_ping(return_to_root = False):
    # print(f" -X-  AMHERE RemRouter send_ping    {RemOrchestrator.RemHardware.time_milis()} ")
    device_id = RemOrchestrator.RemHardware.device_id()

    send_flags="flags"

    if return_to_root :
        send_flags+=";return_to_root"

    for ch_data in RemOrchestrator.get_ch_clients():
        now_milis_ = RemOrchestrator.RemHardware.time_milis()
        message_ = f"{send_flags}:{device_id}:{ch_data.uuid}:{ch_data.protocol}:{now_milis_}:"
        packet = RemHeaderTypes.new_packet_message(message_)
        # RemHeaderTypes.print_packet("from send_ping", packet)
        RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToNeighbor)
        RemHeaderTypes.set_PacketPriority(packet, RemHeaderTypes.PacketPriority_Medium)
        RemHeaderTypes.set_ContentsType(packet, RemHeaderTypes.ContentsType_RemPingPong)
        RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_SendPing)
        ch_data.client_logic.send_raw(ch_data, packet)
        # RemHeaderTypes.print_packet(f"send_ping~ {ch_data.uuid}\t",packet)


def recv_ping(packet, server_data):
    RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_RecvPing)

    device_id = RemOrchestrator.RemHardware.device_id()
    now_milis_ = RemOrchestrator.RemHardware.time_milis()

    ping_recv_data = f"{device_id}:{server_data.uuid}:{server_data.protocol}:{now_milis_}:"
    ping_recv_data = ping_recv_data.encode()

    RemHeaderTypes.packet_append(packet, ping_recv_data)
    RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToNeighbor)
    RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_SendPong)
    RemHeaderTypes.normalize(packet)

    # message_ = RemHeaderTypes.get_message(packet)
    # all_pp_data = message_.split(":")
    # raw_pp_data = all_pp_data[1:]

    for ch_data in RemOrchestrator.get_ch_clients():
        copy_packet_ = RemHeaderTypes.clone(packet)

        now_milis_ = RemOrchestrator.RemHardware.time_milis()
        pong_send_data = f"{device_id}:{ch_data.uuid}:{ch_data.protocol}:{now_milis_}:"
        pong_send_data = pong_send_data.encode()
        RemHeaderTypes.packet_append(copy_packet_, pong_send_data)
        RemHeaderTypes.normalize(copy_packet_)
        ch_data.client_logic.send_raw(ch_data, copy_packet_)
        # RemHeaderTypes.print_packet(f"recv_ping~ {ch_data.uuid}\t",copy_packet_)


def recv_pong(packet, server_data):
    # print(f"! AMHERE RemRouter recv_pong ")

    device_id = RemOrchestrator.RemHardware.device_id()
    now_milis_ = RemOrchestrator.RemHardware.time_milis()

    RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_RecvPong)
    pong_recv_data = f"{device_id}:{server_data.uuid}:{server_data.protocol}:{now_milis_}:"
    packet = RemHeaderTypes.packet_append(packet, pong_recv_data)
    # RemHeaderTypes.print_packet(f"recv_pong~ {server_data.uuid}\t",packet)

    process_ping_pong(packet, server_data)


def check_ping_pong(packet, server_data):
    if RemHeaderTypes.get_ContentsType(packet) != RemHeaderTypes.ContentsType_RemPingPong:
        raise Exception(f"check_ping_pong needs ContentsType_RemPingPong: {packet=} {server_data=} ")

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_SendPing:
        recv_ping(packet, server_data)
        return

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_SendPong:
        recv_pong(packet, server_data)
        return

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_RecvPong:
        process_ping_pong(packet, server_data)
        return

    if RemHeaderTypes.get_ContentsMeta(packet) == ContentsMeta_PingPongRoot:

        if RemHeaderTypes.get_HopsCounter(packet) > 200 :
            RemHeaderTypes.print_packet(f"check_ping_pong~PingPongRoot~ {server_data.device_id} {server_data.uuid} ",packet)
        if RemHeaderTypes.get_HopsCounter(packet) > 220 :
            return ####################################### FIXME !!!!!!!!!!!!!!!!!

        if RemOrchestrator.orch_data.is_root == True:
            process_ping_pong(packet, server_data)
        else:
            route_packet(packet)
        return

    log.warn(RemHeaderTypes.str_packet("PING PONG case not trated ", packet))




def process_ping_pong(packet, server_data):
    # log.info(RemHeaderTypes.str_packet("ping pong: ", packet))
    message_ = RemHeaderTypes.get_message(packet)
    all_pp_data = message_.split(":")
    pp_flags = all_pp_data[0]
    raw_pp_data = all_pp_data[1:]

    pp_data = EasyDict()
    pp_data.send_ping = EasyDict()
    pp_data.recv_ping = EasyDict()
    pp_data.send_pong = EasyDict()
    pp_data.recv_pong = EasyDict()

    pp_size = 4

    pp_data.send_ping.device_id = raw_pp_data[0 + pp_size * 0]
    pp_data.send_ping.uuid      = raw_pp_data[1 + pp_size * 0]
    pp_data.send_ping.protocol  = raw_pp_data[2 + pp_size * 0]
    pp_data.send_ping.milis     = raw_pp_data[3 + pp_size * 0]

    pp_data.recv_ping.device_id = raw_pp_data[0 + pp_size * 1]
    pp_data.recv_ping.uuid      = raw_pp_data[1 + pp_size * 1]
    pp_data.recv_ping.protocol  = raw_pp_data[2 + pp_size * 1]
    pp_data.recv_ping.milis     = raw_pp_data[3 + pp_size * 1]

    pp_data.send_pong.device_id = raw_pp_data[0 + pp_size * 2]
    pp_data.send_pong.uuid      = raw_pp_data[1 + pp_size * 2]
    pp_data.send_pong.protocol  = raw_pp_data[2 + pp_size * 2]
    pp_data.send_pong.milis     = raw_pp_data[3 + pp_size * 2]

    pp_data.recv_pong.device_id = raw_pp_data[0 + pp_size * 3]
    pp_data.recv_pong.uuid      = raw_pp_data[1 + pp_size * 3]
    pp_data.recv_pong.protocol  = raw_pp_data[2 + pp_size * 3]
    pp_data.recv_pong.milis     = raw_pp_data[3 + pp_size * 3]


    # TODO store link as tuple and have a dict for it to save things like ping deltas
    RemOrchestrator.channel_link_2_uuid(pp_data.send_ping.uuid, pp_data.recv_ping.uuid)
    RemOrchestrator.channel_link_2_uuid(pp_data.send_pong.uuid, pp_data.recv_pong.uuid)

    RemOrchestrator.channel_set_value(pp_data.send_ping.uuid, "device_id", pp_data.send_ping.device_id)
    RemOrchestrator.channel_set_value(pp_data.send_ping.uuid, "protocol", pp_data.send_ping.protocol)

    RemOrchestrator.channel_set_value(pp_data.recv_ping.uuid, "device_id", pp_data.recv_ping.device_id)
    RemOrchestrator.channel_set_value(pp_data.recv_ping.uuid, "protocol", pp_data.recv_ping.protocol)

    RemOrchestrator.channel_set_value(pp_data.send_pong.uuid, "device_id", pp_data.send_pong.device_id)
    RemOrchestrator.channel_set_value(pp_data.send_pong.uuid, "protocol", pp_data.send_pong.protocol)

    RemOrchestrator.channel_set_value(pp_data.recv_pong.uuid, "device_id", pp_data.recv_pong.device_id)
    RemOrchestrator.channel_set_value(pp_data.recv_pong.uuid, "protocol", pp_data.recv_pong.protocol)


    # RemHeaderTypes.print_packet(f"process_ping_pong~ ",packet)
    if "return_to_root" in pp_flags :
        if pp_data.send_ping.device_id == pp_data.recv_pong.device_id:
            if pp_data.recv_ping.device_id == pp_data.send_pong.device_id:
                if RemOrchestrator.orch_data.is_root != True:
                    return_ping_pong(packet)


def return_ping_pong(packet):
    # send PingPong to root
    if RemOrchestrator.orch_data.is_root == True:
        return

    # print(f"! AMHERE RemRouter return_ping_pong")

    # RemHeaderTypes.print_packet(f"return_ping_pong~got~  ",packet)
    RemHeaderTypes.set_ContentsMeta(packet, ContentsMeta_PingPongRoot)
    RemHeaderTypes.set_ForwardingType(packet, RemHeaderTypes.ForwardingType_ToRoot)
    RemHeaderTypes.set_PacketPriority(packet, RemHeaderTypes.PacketPriority_Medium)
    RemHeaderTypes.normalize(packet)
    # RemHeaderTypes.print_packet(f"return_ping_pong~send~ ",packet)
    route_packet(packet)


def stop():
    pass

def begin():
    pass


