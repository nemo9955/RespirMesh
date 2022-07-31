#!/usr/bin/env python3

RemOrchestrator = None
RemRouter = None
log = None
RemConnectionController = None
RemHardware = None
RemChannel = None
RemHeaderTypes = None

utils = None
EasyDict = None

orch_data = None


def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    RemOrchestrator = remOrchestrator_


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


def init():
    global RemHardware
    global RemRouter
    global RemConnectionController
    global RemHeaderTypes
    global RemChannel

    # RemHardware.set_orchestrator(RemOrchestrator)
    RemRouter.set_orchestrator(RemOrchestrator)
    # RemConnectionController.set_orchestrator(RemOrchestrator)
    # RemHeaderTypes.set_orchestrator(RemOrchestrator)
    RemChannel.set_orchestrator(RemOrchestrator)


    global orch_data
    orch_data = EasyDict()

    orch_data.server_data_list = []
    orch_data.client_data_list = []
    orch_data.packets_queue = None
    orch_data.logic_queue = None
    orch_data.is_root = False
    orch_data.update_counter = 0


def begin():
    update_logic()

def update_logic():
    for server_data in orch_data.server_data_list:
        if server_data.status == "off":
            server_data.server_logic.start_auto(server_data)
    for client_data in orch_data.client_data_list:
        if client_data.status == "off":
            client_data.client_logic.start_auto(client_data)

    # print(f"{orch_data.logic_queue=}")
    while len(orch_data.logic_queue) > 0 :
        task_ = orch_data.logic_queue.pop()
        # print(f"{task_=}")

        if task_.type == "send_server_make_client":
            send_logic_task(task_)
        else:
            log.error(f"Task type not handeled: {task_=}")


def update():
    orch_data.update_counter += 1

    update_logic()

    if orch_data.update_counter % 37 == 3:
        RemRouter.send_ping()

    if orch_data.update_counter % 23 == 1:
        RemRouter.send_mesh_topo()


    while len(orch_data.packets_queue) > 0 :
        packet_ = orch_data.packets_queue.pop()
        packet_ = RemHeaderTypes.decode(packet_)
        RemHeaderTypes.print_packet("got raw pack:",packet_)
        packet_ = RemRouter.process_packet(packet_)
        RemRouter.route_packet(packet_)


    # if orch_data.update_counter % 15 == 5:
    #     for server_data in orch_data.server_data_list:
    #         print(f"i am {RemHardware.device_id()} : {server_data.name=}")
    #     for client_data in orch_data.client_data_list:
    #         print(f"i am {RemHardware.device_id()} : {client_data.name=}")

    rsleep = 400 + int(RemHardware.rand_float()*200) # about 0.5 secs between updates
    # log.info(f"{rsleep=} {RemHardware.device_id()} {RemHardware.time_milis()}")
    RemHardware.sleep_milis(rsleep)




def stop():
    log.debug(f"RemOrchestrator.stop")
    for server_data in orch_data.server_data_list:
        server_data.server_logic.stop(server_data)
    for client_data in orch_data.client_data_list:
        client_data.client_logic.stop(client_data)


def got_packet_type_1(packet_, server_data):
    # print(f" -X-  AMHERE RemOrchestrator got_packet_type_1 !!!!!!!!!!!!!!!!!!!")
    # RemHeaderTypes.print_packet("XXXXX got_packet_type_1 ... ",packet_)
    packet_ = RemHeaderTypes.decode(packet_)

    if check_make_client(packet_, server_data):
        RemHeaderTypes.print_packet("made client",packet_)
        return

    orch_data.packets_queue.append(packet_)





def check_make_client(packet_, server_data):
    # link_server_client_type_1
    # print(f" -X-  AMHERE RemOrchestrator check_make_client  !!!!!!!!!!!!!!!!!!!!!!!!!")
    if server_data.server_client_can_make != True:
        return False

    message_ = RemHeaderTypes.get_message(packet_)
    if "_MAKE_CLIENT_" not in message_ :
        return False

    # print(f"{message_=}")
    # print(f"{server_data=}")
    # print("WILL MAKE CLIENT !!!!!!!!!!!!!!!!")

    # MADE IN link_bidir_client_type_1
    mess_parts = message_.split(":")
    # print(f" ~~~~~~~~~~ {mess_parts=}")
    type_ = mess_parts[0]
    ip_ = mess_parts[1]
    port_ = int(mess_parts[2])
    protocol_ = mess_parts[3]
    serv_name_ = mess_parts[4]

    # print(f" XXXXXXXXXXXXXXXXXXXX ....... {server_data.client_logic=}, {ip_=}, {port_=}")
    init_client_type_1(server_data.client_logic, ip_, port_)
    # init_client_type_1(OSI4TcpClient, connect_to_ip, connect_port_tcp)





def send_logic_task(task_):
    for client_data in orch_data.client_data_list:
        if task_.match_protocol and client_data.protocol != task_.protocol:
            continue
        client_data.client_logic.send_raw(client_data, task_.packet)




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

    message_ = f"_MAKE_CLIENT_:{task_.server_ip}:{task_.server_port}:"
    message_ += f"{task_.protocol}:{task_.server_name}:"
    task_.packet = RemHeaderTypes.new_packet_message(message_)
    RemHeaderTypes.set_ForwardingType(task_.packet , RemHeaderTypes.ForwardingType_ToNeighbor)

    orch_data.logic_queue.append(task_)



def link_bidir_server_type_1(server_data, client_logic):
    # check_make_client
    global RemOrchestrator

    server_data.server_client_can_make = True
    server_data.client_logic = client_logic

    # client_data.name = f"{server_data.name}>{client_data.name}"





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

    orch_data.server_data_list.append(server_data)
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

    orch_data.client_data_list.append(client_data)
    return client_data




def add_channel(channel_):
    pass

def clean_channels():
    pass




