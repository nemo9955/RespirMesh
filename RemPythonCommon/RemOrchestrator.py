#!/usr/bin/env python3

RemOrchestrator = None
RemRouter = None
log = None
RemConnectionController = None
RemHardware = None
RemChannel = None
RemHeaderTypes = None
RemDebugger = None
RemServer = None

utils = None
EasyDict = None

orch_data = None


def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    RemOrchestrator = remOrchestrator_

def set_server(RemServer_):
    global RemServer
    RemServer = RemServer_

def set_debugger(RemDebugger_):
    global RemDebugger
    RemDebugger = RemDebugger_

def set_hardware(remHardware_):
    global RemHardware
    RemHardware = remHardware_

def set_router(remRouter_):
    global RemRouter
    RemRouter = remRouter_

def set_scanner(remConnectionController_):
    global RemConnectionController
    RemConnectionController = remConnectionController_

def set_logger(remLogger_):
    global log
    log = remLogger_
    # log.set_orchestrator(RemOrchestrator)

def set_header(remHeaderTypes_):
    global RemHeaderTypes
    RemHeaderTypes = remHeaderTypes_

def set_channel(remChannel_):
    global RemChannel
    RemChannel = remChannel_

def set_utils(utils_):
    global utils
    global EasyDict
    utils = utils_
    EasyDict = utils.EasyDict

def set_packets_queue(packets_queue_):
    orch_data.packets_queue = packets_queue_


def set_logic_queue(logic_queue_):
    orch_data.logic_queue = logic_queue_


def set_root():
    orch_data.is_root = True


def set_other_orchs():
    # RemHardware.set_orchestrator(RemOrchestrator)
    RemRouter.set_orchestrator(RemOrchestrator)
    # RemConnectionController.set_orchestrator(RemOrchestrator)
    # RemHeaderTypes.set_orchestrator(RemOrchestrator)
    RemChannel.set_orchestrator(RemOrchestrator)
    RemDebugger and RemDebugger.set_orchestrator(RemOrchestrator)
    RemServer and RemServer.set_orchestrator(RemOrchestrator)

def init():
    global RemHardware
    global RemRouter
    global RemConnectionController
    global RemHeaderTypes
    global RemChannel
    global RemDebugger
    global RemServer


    global orch_data
    orch_data = EasyDict()


    orch_data.conn_data = EasyDict()
    orch_data.channels_list = EasyDict()
    orch_data.packets_queue = None
    orch_data.logic_queue = None
    orch_data.is_root = False
    orch_data.update_counter = 0
    # orch_data.channels_counter = 1

    set_other_orchs()



def begin():
    update_logic()
    RemDebugger and RemDebugger.begin()
    RemServer and RemServer.begin()

def get_ch_clients():
    chlist=[]
    for ch_uuid, ch_data in orch_data.channels_list.items():
        if ch_data.get("is_here", False) != True: continue
        # if ch_data.get("status", None) == None: continue
        if ch_data.get("type", None) != "client": continue
        chlist.append(ch_data)
    return chlist

def get_ch_servers():
    chlist=[]
    for ch_uuid, ch_data in orch_data.channels_list.items():
        if ch_data.get("is_here", False) != True: continue
        # if ch_data.get("status", None) == None: continue
        if ch_data.get("type", None) != "server": continue
        chlist.append(ch_data)
    return chlist

def get_ch_valid():
    chlist=get_ch_servers()
    chlist.extend(get_ch_clients())
    return chlist

def update_logic():
    for ch_data in get_ch_servers():
        if ch_data.status == "off":
            ch_data.server_logic.start_auto(ch_data)
    for ch_data in get_ch_clients():
        if ch_data.status == "off" :
            ch_data.client_logic.start_auto(ch_data)


    # print(f"{orch_data.logic_queue=}")
    while len(orch_data.logic_queue) > 0 :
        task_ = orch_data.logic_queue.pop()
        # print(f"{task_=}")

        if task_.type == "send_server_make_client":
            send_logic_task(task_)
        else:
            log.error(f"Task type not handeled: {task_=}")


def channel_link_root(chan_uuid):
    orch_data.channels_list.setdefault(chan_uuid, EasyDict())
    orch_data.channels_list[chan_uuid].towards_root = True
    orch_data.channels_list[chan_uuid].uuid = chan_uuid


def channel_link_2_uuid(src_uuid_, dest_uuid_):
    orch_data.conn_data.setdefault(src_uuid_, list())
    if dest_uuid_ not in orch_data.conn_data[src_uuid_]:
        orch_data.conn_data[src_uuid_].append(dest_uuid_)

    orch_data.channels_list.setdefault(src_uuid_, EasyDict())
    orch_data.channels_list[src_uuid_].uuid = src_uuid_

    orch_data.channels_list.setdefault(dest_uuid_, EasyDict())
    orch_data.channels_list[dest_uuid_].uuid = dest_uuid_

    updated_conn_data()

def channel_set_value(chan_uuid, chan_field, chan_val):
    orch_data.channels_list.setdefault(chan_uuid, EasyDict())
    orch_data.channels_list[chan_uuid].setdefault(chan_field, chan_val)



def set_device_root_distance(recv_dev_id, distance):
    for ch_uuid, ch_data in orch_data.channels_list.items():
        if ch_data.device_id == recv_dev_id:
            ch_data.root_distance = distance




def get_ch_closer_to_root():
    chlist=[]
    for ch_data in get_ch_clients():
        ch_uuid = ch_data.uuid
        ch_root_dist = ch_data.root_distance
        is_closer = False
        if ch_uuid not in orch_data.conn_data : continue
        for dest_uuid_ in orch_data.conn_data[ch_uuid]:
            dest_data = orch_data.channels_list[dest_uuid_]
            dest_root_dist = dest_data.get("root_distance", -1)
            if dest_root_dist == -1 : continue

            if dest_root_dist < ch_root_dist:
                # print(f"{dest_root_dist=} {ch_root_dist=} ...... {dest_uuid_=} {ch_uuid=} ")
                is_closer = True
                break
        if is_closer:
            chlist.append(ch_data)
    return chlist

def get_ch_farther_from_root():
    chlist=[]
    for ch_data in get_ch_clients():
        ch_uuid = ch_data.uuid
        ch_root_dist = ch_data.root_distance
        is_closer = False
        if ch_uuid not in orch_data.conn_data : continue
        for dest_uuid_ in orch_data.conn_data[ch_uuid]:
            dest_data = orch_data.channels_list[dest_uuid_]
            dest_root_dist = dest_data.get("root_distance", -1)
            if dest_root_dist == -1 : continue

            if dest_root_dist > ch_root_dist:
                # print(f"{dest_root_dist=} {ch_root_dist=} ...... {dest_uuid_=} {ch_uuid=} ")
                is_closer = True
                break
        if is_closer:
            chlist.append(ch_data)
    return chlist


def updated_conn_data():
    pass
    # for src_uuid_, dest_list_ in orch_data.conn_data.items():
    #     for dest_uuid_ in dest_list_:
    #         src_dev_id_  = orch_data.channels_list[src_uuid_].device_id
    #         dest_dev_id_ = orch_data.channels_list[dest_uuid_].device_id




def update():
    orch_data.update_counter += 1

    update_logic()

    if orch_data.update_counter < 10 and orch_data.update_counter % 2 == 1:
        RemRouter.send_ping()
    elif orch_data.update_counter % 30 == 10:
        RemRouter.send_ping()

    if orch_data.is_root == True and orch_data.update_counter < 10 and orch_data.update_counter % 3 == 1:
        RemRouter.send_mesh_topo()
    elif orch_data.is_root == True and orch_data.update_counter % 10 == 1:
        RemRouter.send_mesh_topo()

    # if orch_data.update_counter % 30 == 5:
    # for ch_uuid, ch_data in orch_data.channels_list.items():
    #         print(f"i am {RemHardware.device_id()} : {ch_data.name=}")



    while len(orch_data.packets_queue) > 0 :
        packet_ = orch_data.packets_queue.pop()

        skip_this_packet_ = False # duplicate packets
        for iter_pack_ in orch_data.packets_queue:
            if iter_pack_ == packet_:
                skip_this_packet_ = True
        if skip_this_packet_ :
            continue

        # packet_ = RemHeaderTypes.decode(packet_)
        # RemHeaderTypes.print_packet("got raw pack:",packet_)
        packet_ = RemRouter.process_packet(packet_)
        # RemHeaderTypes.print_packet("procesed pack:",packet_)
        RemRouter.route_packet(packet_)

    RemDebugger and RemDebugger.update() # update at the end of the call to have fresh data
    RemServer and RemServer.update()
    rsleep = 400 + int(RemHardware.rand_float()*200) # about 0.5 secs between updates
    # log.info(f"{rsleep=} {RemHardware.device_id()} {RemHardware.time_milis()}")
    RemHardware.sleep_milis(rsleep)




def stop():
    log.debug(f"RemOrchestrator.stop")
    for ch_data in get_ch_servers():
        ch_data.server_logic.stop(ch_data)
    for ch_data in get_ch_clients():
        ch_data.client_logic.stop(ch_data)



def got_packet_type_1(packet_, server_data):
    # print(f" -X-  AMHERE RemOrchestrator got_packet_type_1 !!!!!!!!!!!!!!!!!!!")
    # RemHeaderTypes.print_packet("XXXXX got_packet_type_1 ... ",packet_)
    packet_ = RemHeaderTypes.decode_recv(packet_, server_data)

    RemHeaderTypes.add_HopsCounter(packet_)

    if check_make_client(packet_, server_data):
        RemHeaderTypes.print_packet("made client ",packet_)
        return

    if RemRouter.append_mesh_topo(packet_, server_data):
        # RemHeaderTypes.print_packet("mesh topo ",packet_)
        return

    if RemRouter.check_ping_pong(packet_, server_data):
        # RemHeaderTypes.print_packet("ping pong ",packet_)
        return

    orch_data.packets_queue.append(packet_)



def check_make_client(packet_, server_data):
    # link_server_client_type_1
    # print(f" -X-  AMHERE RemOrchestrator check_make_client  !!!!!!!!!!!!!!!!!!!!!!!!!")
    if server_data.server_client_can_make != True:
        return False

    if RemHeaderTypes.get_ContentsType(packet_) != RemHeaderTypes.ContentsType_RemMakeClient:
        return False

    message_ = RemHeaderTypes.get_message(packet_)

    # print(f"{message_=}")
    # print(f"{server_data=}")
    # print("WILL MAKE CLIENT !!!!!!!!!!!!!!!!")

    # MADE IN link_bidir_client_type_1
    mess_parts = message_.split(":")
    # print(f" ~~~~~~~~~~ {mess_parts=}")
    ip_ = mess_parts[0]
    port_ = int(mess_parts[1])
    protocol_ = mess_parts[2]
    serv_name_ = mess_parts[3]

    # print(f" XXXXXXXXXXXXXXXXXXXX ....... {server_data.client_logic=}, {ip_=}, {port_=}")
    init_client_type_1(server_data.client_logic, ip_, port_)
    # init_client_type_1(OSI4TcpClient, connect_to_ip, connect_port_tcp)



def link_bidir_client_type_1(server_data, client_data):
    # print(f" -X-  AMHERE RemOrchestrator link_bidir_client_type_1 ")
    if client_data.protocol != server_data.protocol:
        raise Exception(f"Client and server protocols need to match ! {server_data=} {client_data=} ")

    # print(f" --*-* {server_data=}\n")
    # print(f" --*-* {client_data=}\n")

    task_ = EasyDict()
    task_.type = "send_server_make_client"
    task_.match_protocol = True
    task_.protocol = client_data.protocol
    task_.server_name = server_data.name
    task_.server_ip = server_data.server_ip
    task_.server_port = server_data.server_port
    # task_.root_link = server_data.server_port

    message_ = f"{task_.server_ip}:{task_.server_port}:"
    message_ += f"{task_.protocol}:{task_.server_name}:"
    task_.packet = RemHeaderTypes.new_packet_message(message_)
    RemHeaderTypes.set_ForwardingType(task_.packet, RemHeaderTypes.ForwardingType_ToNeighbor)
    RemHeaderTypes.set_PacketPriority(task_.packet, RemHeaderTypes.PacketPriority_High)
    RemHeaderTypes.set_ContentsType(task_.packet, RemHeaderTypes.ContentsType_RemMakeClient)

    orch_data.logic_queue.append(task_)


def send_logic_task(task_):
    for ch_data in get_ch_clients():
        if task_.match_protocol and ch_data.protocol != task_.protocol:
            continue
        ch_data.client_logic.send_raw(ch_data, task_.packet)





def link_bidir_server_type_1(server_data, client_logic):
    # check_make_client
    global RemOrchestrator

    server_data.server_client_can_make = True
    server_data.client_logic = client_logic

    # client_data.name = f"{server_data.name}>{client_data.name}"


def uuid_small():
    device_id = RemHardware.device_id()
    rand_num = 1000 + int(RemHardware.rand_float()*10000)
    uuid = f"{device_id}_{rand_num}"
    return uuid

# def get_channel_id():
#     ret_id = orch_data.channels_counter
#     orch_data.channels_counter += 1
#     return ret_id

def init_server_type_1(server_logic, server_ip, server_port):
    server_data = EasyDict()
    server_logic.set_data(server_data)
    server_data.server_client_can_make = False
    server_data.server_ip = server_ip
    server_data.server_port = server_port
    server_data.server_logic = server_logic
    server_data.packets_queue = orch_data.packets_queue
    server_data.logic_queue = orch_data.logic_queue
    server_data.socket_obj = None
    server_data.RemOrchestrator = RemOrchestrator
    server_data.status = "off"
    server_data.type = "server"
    server_data.uuid = uuid_small()
    # server_data.channel_id = get_channel_id()
    server_data.device_id = RemHardware.device_id()
    server_data.towards_root = False
    server_data.is_here = True
    server_data.root_distance = -1

    # orch_data.server_data_list.append(server_data)
    orch_data.channels_list[server_data.uuid] = server_data
    return server_data



def init_client_type_1(client_logic, server_ip, server_port):
    client_data = EasyDict()
    client_logic.set_data(client_data)
    client_data.server_ip = server_ip
    client_data.server_port = server_port
    client_data.client_logic = client_logic
    client_data.packets_queue = orch_data.packets_queue
    client_data.logic_queue = orch_data.logic_queue
    client_data.socket_obj = None
    client_data.RemOrchestrator = RemOrchestrator
    client_data.status = "off"
    client_data.type = "client"
    client_data.uuid = uuid_small()
    # client_data.channel_id = get_channel_id()
    client_data.device_id = RemHardware.device_id()
    client_data.towards_root = False
    client_data.is_here = True
    client_data.root_distance = -1

    orch_data.channels_list[client_data.uuid] = client_data
    return client_data




def add_channel(channel_):
    pass

def clean_channels():
    pass




