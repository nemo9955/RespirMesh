#!/usr/bin/env python3






RemOrchestrator = None
RemHeaderTypes = None

def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    global RemHeaderTypes
    RemOrchestrator = remOrchestrator_
    RemHeaderTypes = remOrchestrator_.RemHeaderTypes


def send_packet(packet_):
    pass



def route_packet(packet_):
    # print(f" -X-  AMHERE RemRouter route_packet {RemOrchestrator.RemHardware.time_milis()}")
    # process_packet needs to be called before to ensure packet is updated for current node

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_Here:
        return

    for client_data in RemOrchestrator.orch_data.client_data_list:
        # TODO check if linked to ROOT !!!!!!!!!!!
        client_data.client_logic.send_raw(client_data, packet_)



def update():
    pass


def process_packet(packet_):
    # RemHeaderTypes.print_packet("before process_packet",packet_)

    # ForwardingType needs to change as it passes from node to node

    if RemOrchestrator.orch_data.is_root == True :
        if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
        if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
            RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)

    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighborToRoot:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToNeighbor:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)
    if RemHeaderTypes.get_ForwardingType(packet_) == RemHeaderTypes.ForwardingType_ToParent:
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_Here)

    # RemHeaderTypes.print_packet("after process_packet",packet_)
    return packet_


def handle_mesh_topo(packet_):
    pass

def send_mesh_topo():
    pass
    # TODO implement send topo
    # for client_data in RemOrchestrator.orch_data.client_data_list:
    #     message_ = f"_MESH_TOPO_:{RemOrchestrator.RemHardware.device_id()}:"
    #     packet_ = RemHeaderTypes.new_packet_message(message_)
    #     RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighborToRoot)
        # client_data.client_logic.send_raw(client_data, packet_)


def send_ping():
    # print(f" -X-  AMHERE RemRouter send_ping    {RemOrchestrator.RemHardware.time_milis()} ")
    for client_data in RemOrchestrator.orch_data.client_data_list:
        message_ = f"_PING_ FROM {client_data.name} device_id:{RemOrchestrator.RemHardware.device_id()}"
        packet_ = RemHeaderTypes.new_packet_message(message_)
        # RemHeaderTypes.print_packet("from send_ping", packet_)
        RemHeaderTypes.set_ForwardingType(packet_, RemHeaderTypes.ForwardingType_ToNeighbor)
        client_data.client_logic.send_raw(client_data, packet_)
        # RemOrchestrator.RemHardware.sleep_milis(1000)


def handle_ping(packet_):
    pass

def stop():
    pass

def begin():
    pass


