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
    # RemHardware.set_orchestrator(RemOrchestrator)

def set_router(remRouter_):
    global RemRouter
    RemRouter = remRouter_
    RemRouter.set_orchestrator(RemOrchestrator)

def set_scanner(remConnectionController_):
    global RemConnectionController
    RemConnectionController = remConnectionController_
    RemConnectionController.set_orchestrator(RemOrchestrator)

def set_logger(remLogger_):
    global log
    log = remLogger_
    # log.set_orchestrator(RemOrchestrator)

def set_header(remHeaderTypes_):
    global RemHeaderTypes
    RemHeaderTypes = remHeaderTypes_
    # RemHeaderTypes.set_orchestrator(RemOrchestrator)

def set_channel(remChannel_):
    global RemChannel
    RemChannel = remChannel_
    RemChannel.set_orchestrator(RemOrchestrator)

def set_utils(utils_):
    global utils
    global EasyDict
    utils = utils_
    EasyDict = utils.EasyDict

def set_packets_queue(packets_queue_):
    orch_data.packets_queue = packets_queue_



def set_root():
    orch_data.is_root = True


def init():
    global orch_data
    orch_data = EasyDict()

    orch_data.server_data_list = []
    orch_data.client_data_list = []
    orch_data.packets_queue = None
    orch_data.is_root = False
    orch_data.update_counter = 0


def begin():
    for server_data in orch_data.server_data_list:
        server_data.logic.start_auto(server_data)
    for client_data in orch_data.client_data_list:
        client_data.logic.start_auto(client_data)



def update():
    orch_data.update_counter += 1

    while len(orch_data.packets_queue) > 0 :
        packet_ = orch_data.packets_queue.pop()
        if orch_data.is_root == False :
            RemRouter.route_packet(packet_)
        # print(f" ~~~ {packet_=}")
        RemHeaderTypes.print_packet(packet_)


    if orch_data.is_root == False :
        if orch_data.update_counter % 5 == 1:
            RemRouter.send_ping()
            # RemRouter.send_mesh_topo()

    rsleep = 100 + int(RemHardware.rand_float()*200)
    # log.info(f"{rsleep=} {RemHardware.device_id()} {RemHardware.time_milis()}")
    RemHardware.sleep_milis(rsleep)




def stop():
    log.debug(f"RemOrchestrator.stop")
    for server_data in orch_data.server_data_list:
        server_data.logic.stop(server_data)
    for client_data in orch_data.client_data_list:
        client_data.logic.stop(client_data)



def init_server_type_1(server_logic, server_ip, server_port):
    server_data = EasyDict()
    server_data.name = "UNKNOWN"
    server_data.server_ip = server_ip
    server_data.server_port = server_port
    server_data.logic = server_logic
    # server_data.logic.set_orchestrator(RemOrchestrator)
    server_data.packets_queue=orch_data.packets_queue
    server_data.socket_obj = None

    orch_data.server_data_list.append(server_data)
    return server_data

def init_client_type_1(client_logic, server_ip, server_port):
    client_data = EasyDict()
    client_data.name = "UNKNOWN"
    client_data.server_ip = server_ip
    client_data.server_port = server_port
    client_data.logic = client_logic
    # client_data.logic.set_orchestrator(RemOrchestrator)
    client_data.packets_queue=orch_data.packets_queue
    client_data.socket_obj = None

    orch_data.client_data_list.append(client_data)
    return client_data




def add_channel(channel_):
    pass

def clean_channels():
    pass




