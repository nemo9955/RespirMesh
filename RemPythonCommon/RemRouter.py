#!/usr/bin/env python3






RemOrchestrator = None
RemHeaderTypes = None
log = None
EasyDict = None

def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    global RemHeaderTypes
    global log
    global EasyDict
    RemOrchestrator = remOrchestrator_
    RemHeaderTypes = remOrchestrator_.RemHeaderTypes
    log = remOrchestrator_.log
    EasyDict = RemOrchestrator.utils.EasyDict





def route_packet(packet_):
    # print(f" -X-  AMHERE RemRouter route_packet {RemOrchestrator.RemHardware.time_milis()}")
    # process_packet needs to be called before to ensure packet is updated for current node
    message_ = RemHeaderTypes.get_message(packet_)


    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
        for ch_data in RemOrchestrator.get_ch_closer_to_root():
            ch_data.client_logic.send_raw(ch_data, packet_)
            return # TODO get the closest to root not just the first closest

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Here:
        return

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Drop:
        return

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToEdges:
        for ch_data in RemOrchestrator.get_ch_farther_from_root():
            ch_data.client_logic.send_raw(ch_data, packet_)
        return

    for ch_data in RemOrchestrator.get_ch_clients():
        # TODO check if linked to ROOT !!!!!!!!!!!
        ch_data.client_logic.send_raw(ch_data, packet_)



def route_packet_back(packet_, ch_data):
    pass


def update():
    pass


def process_packet(packet_):
    # RemHeaderTypes.print_packet("before process_packet",packet_)
    message_ = RemHeaderTypes.get_message(packet_)

    # ForwardingType needs to change as it passes from node to node

    if RemOrchestrator.orch_data.is_root == True :
        if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
        elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToRoot)

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Back:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToRoot)
    elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighbor:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToParent:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
    elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborAndBack:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Back)

    # RemHeaderTypes.print_packet("after process_packet",packet_)

    if RemHeaderTypes.get_ContentsType(packet_) == RemHeaderTypes.ContentsType_RemPingPong:
        process_ping_pong(packet_)

    if RemHeaderTypes.get_ContentsType(packet_) == RemHeaderTypes.ContentsType_RemMeshTopo:
        process_mesh_topo(packet_)

    return packet_




def send_mesh_topo():
    # processed in  RemOrchestrator.append_mesh_topo
    if RemOrchestrator.orch_data.is_root != True:
        raise Exception(f"send_mesh_topo should be called only in root/server node: {RemOrchestrator}")

    device_id = RemOrchestrator.RemHardware.device_id()
    for ch_data in RemOrchestrator.get_ch_clients():
        message_ = f":{device_id}:"
        packet_ = RemHeaderTypes.new_packet_message(message_)
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighbor)
        RemHeaderTypes.set_PacketPriority(packet_, RemHeaderTypes.PacketPriority_High)
        RemHeaderTypes.set_ContentsType(packet_, RemHeaderTypes.ContentsType_RemMeshTopo)
        RemHeaderTypes.normalize(packet_)
        ch_data.client_logic.send_raw(ch_data, packet_)


def append_mesh_topo(packet_, server_data):
    # MADE IN send_mesh_topo
    if RemHeaderTypes.get_ContentsType(packet_) != RemHeaderTypes.ContentsType_RemMeshTopo:
        return False

    # return False # !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # log.warn("append_mesh_topo not implemented")

    message_ = RemHeaderTypes.get_message(packet_)
    device_id = RemOrchestrator.RemHardware.device_id()

    if f":{device_id}:" in message_:
        return False

    recv_dev_id = server_data.device_id

    # this node is new in the topo list
    hops_header = RemHeaderTypes.get_HopsCounter(packet_)
    hops_message = message_.count(":") - 1
    if hops_header != hops_message:
        print(f" ~~~~~~~~ {hops_header=} {hops_message=} !!!!!!!!!!!!!!!!!!!!!!!!")
    # RemHeaderTypes.print_packet(f" i am {device_id=} {hops_header=} {hops_message=} mesh topo ",packet_)

    add_me_ = f"{device_id}:"
    RemHeaderTypes.packet_append(packet_, add_me_)

    # RemHeaderTypes.print_packet(f" i am {device_id=} sending mesh topo ",packet_)

    RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighbor)
    for ch_data in RemOrchestrator.get_ch_clients():
        ch_data.client_logic.send_raw(ch_data, packet_)

    RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToRoot)
    # RemHeaderTypes.print_packet(f" i am {device_id=} REsending mesh topo ",packet_)
    route_packet(packet_)

    process_mesh_topo(packet_)

    # for i in range(len(root_hops) -1 ):
    #     now_id_ = root_hops[i]
    #     next_id_ = root_hops[i+1]
    #     if not now_id_   : continue
    #     if not next_id_ : continue
    #     RemOrchestrator.channel_link_2_uuid(pp_data.send_ping.uuid, pp_data.recv_ping.uuid)

    return True

def process_mesh_topo(packet_):
    full_message_ = RemHeaderTypes.get_message(packet_)
    root_hops = full_message_.split(":")
    counted_hops = 0

    for did_ in root_hops:
        if not did_: continue
        RemOrchestrator.set_device_root_distance(did_, counted_hops)
        counted_hops+=1



ContentsMeta_SendPing     = 1
ContentsMeta_RecvPing     = 2
ContentsMeta_SendPong     = 3
ContentsMeta_RecvPong     = 4
ContentsMeta_SendPingPong = 5
ContentsMeta_RecvPingPong = 6

def send_ping():
    # print(f" -X-  AMHERE RemRouter send_ping    {RemOrchestrator.RemHardware.time_milis()} ")
    device_id = RemOrchestrator.RemHardware.device_id()
    for ch_data in RemOrchestrator.get_ch_clients():
        now_milis_ = RemOrchestrator.RemHardware.time_milis()
        message_ = f"{device_id}:{ch_data.uuid}:{ch_data.protocol}:{now_milis_}:"
        packet_ = RemHeaderTypes.new_packet_message(message_)
        # RemHeaderTypes.print_packet("from send_ping", packet_)
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighbor)
        RemHeaderTypes.set_PacketPriority(packet_, RemHeaderTypes.PacketPriority_High)
        RemHeaderTypes.set_ContentsType(packet_, RemHeaderTypes.ContentsType_RemPingPong)
        RemHeaderTypes.set_ContentsMeta(packet_, ContentsMeta_SendPing)
        ch_data.client_logic.send_raw(ch_data, packet_)


def recv_ping(packet_, server_data):
    RemHeaderTypes.set_ContentsMeta(packet_, ContentsMeta_RecvPing)

    # RemHeaderTypes.print_packet("before recv_ping ", packet_)

    device_id = RemOrchestrator.RemHardware.device_id()
    now_milis_ = RemOrchestrator.RemHardware.time_milis()

    ping_recv_data = f"{device_id}:{server_data.uuid}:{server_data.protocol}:{now_milis_}:"
    ping_recv_data = ping_recv_data.encode()

    RemHeaderTypes.packet_append(packet_, ping_recv_data)
    RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighbor)
    RemHeaderTypes.set_ContentsMeta(packet_, ContentsMeta_SendPong)
    RemHeaderTypes.normalize(packet_)

    for ch_data in RemOrchestrator.get_ch_clients():
        copy_packet_ = RemHeaderTypes.clone(packet_)

        now_milis_ = RemOrchestrator.RemHardware.time_milis()
        pong_send_data = f"{device_id}:{ch_data.uuid}:{ch_data.protocol}:{now_milis_}:"
        pong_send_data = pong_send_data.encode()
        RemHeaderTypes.packet_append(copy_packet_, pong_send_data)
        RemHeaderTypes.normalize(copy_packet_)
        ch_data.client_logic.send_raw(ch_data, copy_packet_)

        # RemHeaderTypes.print_packet("after  recv_ping ", copy_packet_)


    # RemOrchestrator.orch_data.packets_queue.append(packet_)
    # route_packet(packet_)
    return True

def recv_pong(packet_, server_data):
    # print(f"! AMHERE RemRouter recv_pong ")

    device_id = RemOrchestrator.RemHardware.device_id()
    now_milis_ = RemOrchestrator.RemHardware.time_milis()

    RemHeaderTypes.set_ContentsMeta(packet_, ContentsMeta_RecvPong)

    pong_recv_data = f"{device_id}:{server_data.uuid}:{server_data.protocol}:{now_milis_}:"
    pong_recv_data = pong_recv_data.encode()

    # RemHeaderTypes.print_packet("before recv_pong ", packet_)
    packet_ = RemHeaderTypes.packet_append(packet_, pong_recv_data)
    # RemHeaderTypes.print_packet("after recv_pong ", packet_)

    # RemOrchestrator.orch_data.packets_queue.append(packet_)
    process_ping_pong(packet_)


    return True


def check_ping_pong(packet_, server_data):
    if RemHeaderTypes.get_ContentsType(packet_) != RemHeaderTypes.ContentsType_RemPingPong:
        return False

    # RemHeaderTypes.print_packet("before check_ping_pong ", packet_)

    if RemHeaderTypes.get_ContentsMeta(packet_) == ContentsMeta_RecvPingPong:
        return False

    if RemHeaderTypes.get_ContentsMeta(packet_) == ContentsMeta_SendPing:
        return recv_ping(packet_, server_data)

    if RemHeaderTypes.get_ContentsMeta(packet_) == ContentsMeta_SendPong:
        return recv_pong(packet_, server_data)

    log.warn(RemHeaderTypes.str_packet("PING PONG case not trated ", packet_))

    return False


def process_ping_pong(packet_):
    # log.info(RemHeaderTypes.str_packet("ping pong: ", packet_))


    # RemHeaderTypes.print_packet("before process_ping_pong ", packet_)

    message_ = RemHeaderTypes.get_message(packet_)
    raw_pp_data = message_.split(":")
    # print(f"len(raw_pp_data) = {len(raw_pp_data)}")
    # print(f"{raw_pp_data=}")

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

    # print(f"\n\n {pp_data=}")


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


    # print(f"{ping_=}")
    # print(f"{pong_=}")


def handle_ping(packet_):
    pass

def stop():
    pass

def begin():
    pass


