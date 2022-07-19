#!/usr/bin/env python3






RemOrchestrator = None

def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    RemOrchestrator = remOrchestrator_


def send_packet(packet_):
    pass

def route_packet(packet_):
    for client_data in RemOrchestrator.orch_data.client_data_list:
        # TODO check if linked to ROOT !!!!!!!!!!!
        client_data.logic.send_raw(client_data, packet_)

def process_packet(packet_):
    pass

def update():
    pass

def handle_mesh_topo(packet_):
    pass

def send_mesh_topo():
    pass




def send_ping():
    # print(f" -X-  AMHERE RemRouter send_ping ")
    for client_data in RemOrchestrator.orch_data.client_data_list:
        packet_ = RemOrchestrator.RemHeaderTypes.new_packet()
        message_ = f"FROM {client_data.name} device_id:{RemOrchestrator.RemHardware.device_id()}".encode()
        packet_.extend(message_)
        # print(f"sending ping {packet_=}")
        RemOrchestrator.RemHeaderTypes.set_size(len(packet_), packet_)
        client_data.logic.send_raw(client_data, packet_)


def handle_ping(packet_):
    pass

def stop():
    pass

def begin():
    pass


