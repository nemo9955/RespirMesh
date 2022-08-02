#!/usr/bin/env python3






RemOrchestrator = None
RemHeaderTypes = None
log = None

def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    global RemHeaderTypes
    global log
    RemOrchestrator = remOrchestrator_
    RemHeaderTypes = remOrchestrator_.RemHeaderTypes
    log = remOrchestrator_.log





def route_packet(packet_):
    # print(f" -X-  AMHERE RemRouter route_packet {RemOrchestrator.RemHardware.time_milis()}")
    # process_packet needs to be called before to ensure packet is updated for current node
    message_ = RemHeaderTypes.get_message(packet_)

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Here:
        return

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Drop:
        return

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Back:
        # TODO have multiple levels of precision in sending back
        # least precise is sending to all clients
        # decent is sending to the same protocol
        # perfect is matching the uuid


        # log.warn(RemHeaderTypes.str_packet("Needs back implementation: ", packet_))

        for ch_data in RemOrchestrator.get_ch_clients():
            if ch_data.protocol in message_:
                ch_data.client_logic.send_raw(ch_data, packet_)

        return

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
            # for ch_data in RemOrchestrator.get_ch_clients():
            #     if ch_data.linked_to_root :
            #         ch_data.client_logic.send_raw(ch_data, packet_)

        return

    # TODO proper filter ForwardingType_ToParent

    for ch_data in RemOrchestrator.get_ch_clients():
        # TODO check if linked to ROOT !!!!!!!!!!!
        ch_data.client_logic.send_raw(ch_data, packet_)



def update():
    pass


def process_packet(packet_):
    # RemHeaderTypes.print_packet("before process_packet",packet_)
    message_ = RemHeaderTypes.get_message(packet_)

    RemHeaderTypes.add_HopsCounter(packet_)

    # ForwardingType needs to change as it passes from node to node

    if RemOrchestrator.orch_data.is_root == True :
        if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
        elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToRoot)


    if RemOrchestrator.orch_data.linked_to_root == False :
        if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToParent:
            log.error(RemHeaderTypes.str_packet("No link to parent, dropping packet: ", packet_))
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Drop)
        elif RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
            log.error(RemHeaderTypes.str_packet("No link to root, dropping packet: ", packet_))
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Drop)


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

    if "_PING_" in message_ and "_PONG_" in message_ :
        process_ping_pong(packet_)


    return packet_





def append_mesh_topo(packet_, server_data):
    message_ = RemHeaderTypes.get_message(packet_)
    if "_MESH_TOPO_" not in message_ :
        return False

    if RemHeaderTypes.get_ForwardingType(packet_) != RemHeaderTypes.ForwardingType_ToNeighborToRoot:
        return False

    # MADE IN send_mesh_topo
    mess_parts = message_.split(":")
    type_ = mess_parts[0]
    parent_id_ = mess_parts[1]
    parent_link_root_ = bool(mess_parts[2])

    if parent_link_root_ == True :
        RemOrchestrator.channel_link_root(server_data.uuid)

    device_id = RemOrchestrator.RemHardware.device_id()
    root_rel = int(RemOrchestrator.has_root_path())

    topo_neighbor_data = f"{device_id}:{root_rel}:"
    topo_neighbor_data = topo_neighbor_data.encode()

    # print(f"+++++++ {packet_=}")
    # RemHeaderTypes.print_packet("before topo ",packet_)

    RemHeaderTypes.packet_append(packet_, topo_neighbor_data)

    # we mark it as directly to root after being processed
    RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToRoot)

    packet_ = RemHeaderTypes.normalize(packet_)

    # print(f"+++++++ {packet_=}")
    # RemHeaderTypes.print_packet("after topo ",packet_)

    route_packet(packet_)

    # TODO to process it now or add it to queue to be processed later ???
    # TODO also send it !!!!!!!!!!!!!


def send_mesh_topo():
    # processed in  RemOrchestrator.check_mesh_topo

    device_id = RemOrchestrator.RemHardware.device_id()
    root_rel = int(RemOrchestrator.has_root_path())
    for ch_data in RemOrchestrator.get_ch_clients():
        message_ = f"_MESH_TOPO_:{device_id}:{root_rel}:"
        packet_ = RemHeaderTypes.new_packet_message(message_)
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighborToRoot)
        ch_data.client_logic.send_raw(ch_data, packet_)

def send_ping():
    # print(f" -X-  AMHERE RemRouter send_ping    {RemOrchestrator.RemHardware.time_milis()} ")
    device_id = RemOrchestrator.RemHardware.device_id()
    root_rel = int(RemOrchestrator.has_root_path())
    for ch_data in RemOrchestrator.get_ch_clients():
        now_milis_ = RemOrchestrator.RemHardware.time_milis()
        message_ = f"_PING_:{device_id}:{ch_data.uuid}:{ch_data.protocol}:{now_milis_}:{root_rel}:"
        packet_ = RemHeaderTypes.new_packet_message(message_)
        # RemHeaderTypes.print_packet("from send_ping", packet_)
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighborAndBack)
        ch_data.client_logic.send_raw(ch_data, packet_)
        # RemOrchestrator.RemHardware.sleep_milis(1000)


def append_pong(packet_, server_data):

    message_ = RemHeaderTypes.get_message(packet_)
    if "_PING_" not in message_ :
        return False
    # if it has pong, no other action needed
    if "_PONG_" in message_ :
        return False


    device_id = RemOrchestrator.RemHardware.device_id()
    now_milis_ = RemOrchestrator.RemHardware.time_milis()

    pong_data = f"_PONG_:{device_id}:{server_data.uuid}:{server_data.protocol}:{now_milis_}:"
    pong_data = pong_data.encode()

    RemHeaderTypes.packet_append(packet_, pong_data)
    RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Back)
    packet_ = RemHeaderTypes.normalize(packet_)

    RemOrchestrator.orch_data.packets_queue.append(packet_)

    route_packet(packet_)
    return True

def process_ping_pong(packet_):
    # log.info(RemHeaderTypes.str_packet("ping pong: ", packet_))

    message_ = RemHeaderTypes.get_message(packet_)
    ping_, pong_ = message_.split("_PONG_:")
    ping_ = ping_.replace("_PING_:","")

    ping_ = ping_.split(":")
    pong_ = pong_.split(":")

    ping_time_ = int(ping_[3])
    pong_time_ = int(pong_[3])
    ping_pond_delta = pong_time_ - ping_time_
    # print(f"{ping_pond_delta=}")

    ping_device_id_ = ping_[0]
    pong_device_id_ = pong_[0]
    ping_chid_ = ping_[1]
    pong_chid_ = pong_[1]
    ping_proto_ = ping_[2]
    pong_proto_ = pong_[2]
    ping_root_rel_ = ping_[4]
    pong_root_rel_ = pong_[4]

    if ping_root_rel_ or pong_root_rel_:
        RemOrchestrator.channel_link_root(ping_chid_)
        RemOrchestrator.channel_link_root(pong_chid_)


    RemOrchestrator.channel_link_2_uuid(ping_chid_, pong_chid_)
    RemOrchestrator.channel_set_value(ping_chid_, "device_id", ping_device_id_)
    RemOrchestrator.channel_set_value(pong_chid_, "device_id", pong_device_id_)
    RemOrchestrator.channel_set_value(ping_chid_, "protocol", ping_proto_)
    RemOrchestrator.channel_set_value(pong_chid_, "protocol", pong_proto_)

    # print(f"{ping_=}")
    # print(f"{pong_=}")


def handle_ping(packet_):
    pass

def stop():
    pass

def begin():
    pass


