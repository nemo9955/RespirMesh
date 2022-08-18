#!/usr/bin/env python3

RemOrchestrator = None
RemRouter = None
log = None
RemHardware = None
RemScanner = None
RemHeaderTypes = None
RemDebugger = None
RemServer = None

utils = None
EasyDict = None

orch_data = None


def set_orchestrator(set_value_):
    global RemOrchestrator
    RemOrchestrator = set_value_

def set_server(set_value_):
    global RemServer
    RemServer = set_value_

def set_debugger(set_value_):
    global RemDebugger
    RemDebugger = set_value_

def set_hardware(set_value_):
    global RemHardware
    RemHardware = set_value_

def set_router(set_value_):
    global RemRouter
    RemRouter = set_value_

def set_logger(set_value_):
    global log
    log = set_value_
    # log.set_orchestrator(RemOrchestrator)

def set_header(set_value_):
    global RemHeaderTypes
    RemHeaderTypes = set_value_

def set_scanner(set_value_):
    global RemScanner
    RemScanner = set_value_

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
    # RemHeaderTypes.set_orchestrator(RemOrchestrator)
    RemScanner and RemScanner.set_orchestrator(RemOrchestrator)
    RemDebugger and RemDebugger.set_orchestrator(RemOrchestrator)
    RemServer and RemServer.set_orchestrator(RemOrchestrator)

def init():
    global RemHardware
    global RemRouter
    global RemHeaderTypes
    global RemScanner
    global RemDebugger
    global RemServer


    global orch_data
    orch_data = EasyDict()


    orch_data.conn_data = EasyDict()
    orch_data.channels_list = EasyDict()
    orch_data.scanners_list = EasyDict()
    orch_data.packets_queue = None
    orch_data.logic_queue = None
    orch_data.is_root = False
    orch_data.update_counter = 0
    # orch_data.channels_counter = 1

    orch_data.startup_time = -1
    orch_data.update_discrete_last_time = -1
    orch_data.update_continous_last_time = -1

    orch_data.update_discrete_interval_fast = 300
    orch_data.update_discrete_interval_norm = 1000
    orch_data.update_discrete_interval_slow = 2000

    set_other_orchs()




##     ## ########  ########     ###    ######## ########
##     ## ##     ## ##     ##   ## ##      ##    ##
##     ## ##     ## ##     ##  ##   ##     ##    ##
##     ## ########  ##     ## ##     ##    ##    ######
##     ## ##        ##     ## #########    ##    ##
##     ## ##        ##     ## ##     ##    ##    ##
 #######  ##        ########  ##     ##    ##    ########



def update():
    if orch_data.startup_time <= 0 :
        begin()

    now_time = RemHardware.time_milis()

    delta_cont = now_time - orch_data.update_continous_last_time
    orch_data.update_continous_last_time = now_time
    update_continous(delta_cont)

    if orch_data.update_counter < 60:
        discr_interval = orch_data.update_discrete_interval_fast
    elif orch_data.update_counter < 300:
        discr_interval = orch_data.update_discrete_interval_norm
    else:
        discr_interval = orch_data.update_discrete_interval_slow

    if now_time - orch_data.update_discrete_last_time > discr_interval:
        orch_data.update_discrete_last_time = now_time
        update_discrete(orch_data.update_counter)
        orch_data.update_counter += 1

    # give a chance to pachets to deduplicate
    # RemHardware.sleep_milis(50)



def update_discrete(counter):
    # function called every time interval
    # used to simplify internal mesh checks
    # print(f"{counter=}")

    update_logic()

    orch_data.get_ch_closer_to_root_uuids=[]
    for ch_data in RemOrchestrator.get_ch_closer_to_root():
        orch_data.get_ch_closer_to_root_uuids.append(ch_data.uuid)

    orch_data.get_ch_farther_from_root_uuids=[]
    for ch_data in RemOrchestrator.get_ch_farther_from_root():
        orch_data.get_ch_farther_from_root_uuids.append(ch_data.uuid)


    # send_ping needs to be called first
    if counter > 60 and counter % 65 == 1:
        RemRouter.send_ping(return_to_root = True)
    elif counter % 35 == 1:
        RemRouter.send_ping()

    if orch_data.is_root == True and counter % 45 == 5:
        RemRouter.send_mesh_topo()

    # if counter % 30 == 5:
    # for ch_uuid, ch_data in orch_data.channels_list.items():
    #         print(f"i am {RemHardware.device_id()} : {ch_data.name=}")

    RemDebugger and RemDebugger.update_discrete(counter) # update at the end of the call to have fresh data
    RemServer and RemServer.update_discrete(counter)
    RemScanner and RemScanner.update_discrete(counter)



def update_continous(delta):
    # function called every update frame
    # used for precise actions


    while len(orch_data.packets_queue) > 0 :
        packet_wrapper_ = orch_data.packets_queue.pop()

        skip_this_packet_ = False # duplicate packets
        for iter_pack_ in orch_data.packets_queue:
            if len(iter_pack_)>1 and len(packet_wrapper_)>1 :
                if iter_pack_[0] == packet_wrapper_[0]:
                    skip_this_packet_ = True
            if iter_pack_ == packet_wrapper_:
                skip_this_packet_ = True
        if skip_this_packet_ :
            continue

        RemRouter.process_packet(packet_wrapper_)

    RemDebugger and RemDebugger.update_continous(delta) # update at the end of the call to have fresh data
    RemServer and RemServer.update_continous(delta)
    RemScanner and RemScanner.update_continous(delta)


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



def begin():
    orch_data.startup_time = RemHardware.time_milis()
    RemDebugger and RemDebugger.begin()
    RemServer and RemServer.begin()
    RemScanner and RemScanner.begin()
    update_logic()

    rsleep = int(RemHardware.rand_float()*500) # about 0.5 secs between updates
    RemHardware.sleep_milis(rsleep)



def stop():
    log.debug(f"RemOrchestrator.stop")
    for ch_data in get_ch_servers():
        ch_data.server_logic.stop(ch_data)
    for ch_data in get_ch_clients():
        ch_data.client_logic.stop(ch_data)



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



def got_packet_type_1(packet_, server_data):
    # print(f" -X-  AMHERE RemOrchestrator got_packet_type_1 !!!!!!!!!!!!!!!!!!!")
    # RemHeaderTypes.print_packet("XXXXX got_packet_type_1 ... ",packet_)
    packet_ = RemHeaderTypes.decode_recv(packet_, server_data)

    orch_data.packets_queue.append((packet_, server_data))



def process_make_client_type_1(packet_, server_data):
    # link_server_client_type_1
    # print(f" -X-  AMHERE RemOrchestrator process_make_client_type_1  !!!!!!!!!!!!!!!!!!!!!!!!!")

    if RemHeaderTypes.get_ContentsType(packet_) != RemHeaderTypes.ContentsType_RemMakeClient1:
        raise Exception(f"process_make_client_type_1 needs ContentsType_RemMakeClient1: {packet_=} {server_data=} ")

    if server_data.server_client_can_make != True:
        return

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
    RemHeaderTypes.set_ContentsType(task_.packet, RemHeaderTypes.ContentsType_RemMakeClient1)

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


def uuid_small(extra="_"):
    device_id = RemHardware.device_id()
    rand_num = 1000 + int(RemHardware.rand_float()*10000)
    uuid = f"{device_id}{extra}{rand_num}"
    return uuid

def init_server_type_1(server_logic, server_ip, server_port):
    server_data = EasyDict()
    server_logic.set_data(server_data)
    server_logic.set_orchestrator(RemOrchestrator)
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
    server_data.uuid = uuid_small("_srv_")
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
    client_logic.set_orchestrator(RemOrchestrator)
    client_data.server_ip = server_ip
    client_data.server_port = server_port
    client_data.client_logic = client_logic
    client_data.packets_queue = orch_data.packets_queue
    client_data.logic_queue = orch_data.logic_queue
    client_data.socket_obj = None
    client_data.RemOrchestrator = RemOrchestrator
    client_data.status = "off"
    client_data.type = "client"
    client_data.uuid = uuid_small("_cli_")
    client_data.device_id = RemHardware.device_id()
    client_data.towards_root = False
    client_data.is_here = True
    client_data.root_distance = -1

    orch_data.channels_list[client_data.uuid] = client_data
    return client_data

def init_scanner_type_1(scanner_logic):
    scanner_data = EasyDict()
    scanner_logic.set_data(scanner_data)
    scanner_logic.set_orchestrator(RemOrchestrator)
    scanner_data.packets_queue = orch_data.packets_queue
    scanner_data.scanner_logic = scanner_logic
    scanner_data.logic_queue = orch_data.logic_queue
    scanner_data.RemOrchestrator = RemOrchestrator
    scanner_data.status = "off"
    scanner_data.type = "scanner"
    scanner_data.uuid = uuid_small("_scn_")
    scanner_data.device_id = RemHardware.device_id()

    orch_data.scanners_list[scanner_data.uuid] = scanner_data
    return scanner_data



