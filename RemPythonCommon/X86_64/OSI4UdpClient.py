#!/usr/bin/env python3

import socket
import traceback


RemOrchestrator = None
EasyDict = None
log = None
def set_orchestrator(set_value_):
    global RemOrchestrator
    global EasyDict
    global log
    RemOrchestrator = set_value_
    EasyDict = RemOrchestrator.EasyDict
    log = set_value_.log



def set_data(client_data):
    client_data.name = "UDP CLIENT"
    client_data.protocol = "ip_udp"

def start_auto(client_data):
    # print(f" -X-  AMHERE OSI4UdpClient start_auto {client_data.name} {client_data.server_ip} {client_data.server_port} ")

    # Create a TCP/IP socket
    client_data.socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    client_data.status = "on"

    # Connect the socket to the port where the server is listening
    client_data.socket_obj.connect((client_data.server_ip, client_data.server_port))

    return client_data




def send_raw(client_data, packet_):
    # print(f" -X-  AMHERE OSI4UdpClient send_raw {packet_=}")

    if client_data.status != "on" :
        return

    bytes_sent = -1
    try:
        packet_ = client_data.RemOrchestrator.RemHeaderTypes.encode_send(packet_,client_data)
        # bytes_sent = client_data.socket_obj.sendall(packet_)
        # bytes_sent = client_data.socket_obj.sendto(packet_,(client_data.server_ip, client_data.server_port))
        bytes_sent = client_data.socket_obj.send(packet_)
    except:
        # client_data.socket_obj.close()
        # raise Exception(f"OSI4UdpClient ERROR {packet_=}")
        print(f"{packet_=}")
        client_data.RemOrchestrator.RemHeaderTypes.print_packet(f"send_raw~udp~  ",packet_)
        traceback.print_exc()
        stop(client_data)
    # print(f"{bytes_sent=}")
    return bytes_sent





def stop(client_data):
    if client_data.status != "on":
        return
    # client_data.socket_obj.close()
    # print(f"********************** {os.getpid()=} {client_data.subprocess=}")




if __name__ == "__main__":
    import os, sys
    rem_base_path = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
    sys.path.append(f"{rem_base_path}/RemPythonCommon")
    from utils import EasyDict

    client_data_= EasyDict()
    client_data_.server_ip = "localhost"
    client_data_.server_port = 20000 + 2

    start_auto(client_data_)
    send_raw(client_data_, b"TEMP UDP MESSAGE 1111111 !!!!!!!!!!!!!!")
    stop(client_data_)
