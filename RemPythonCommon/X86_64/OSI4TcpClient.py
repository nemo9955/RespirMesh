#!/usr/bin/env python3

import socket
import time
import traceback
from contextlib import closing

# https://stackoverflow.com/questions/19196105/how-to-check-if-a-network-port-is-open


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


# def wait_port_ok(client_data):
#     with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as sock:
#         result = sock.connect_ex((client_data.server_ip, client_data.server_port))

#         limit=60
#         while limit > 0:
#             limit-=1
#             if result == 0:
#                 return
#             time.sleep_milis(500)
#         raise Exception(f"Port not open {client_data}")

def set_data(client_data):
    client_data.name = "TCP CLIENT"
    client_data.protocol = "ip_tcp"

def start_auto(client_data):
    # print(f" -X-  AMHERE OSI4TcpClient start_auto {client_data.name} {client_data.server_ip} {client_data.server_port} ")

    # Create a TCP/IP socket
    client_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    client_data.socket_obj.connect((client_data.server_ip, client_data.server_port))

    client_data.status = "on"

    return client_data



def send_raw(client_data, packet_):
    # print(f" -X-  AMHERE OSI4TcpClient send_raw {client_data.name} {packet_=}")

    if client_data.status != "on" :
        return

    bytes_sent = -1
    try:
        packet_ = client_data.RemOrchestrator.RemHeaderTypes.encode_send(packet_,client_data)
        # bytes_sent = client_data.socket_obj.sendall(packet_)
        bytes_sent = client_data.socket_obj.send(packet_)
    except:
        traceback.print_exc()
        client_data.socket_obj.close()
        raise Exception(f"OSI4TcpClient ERROR.")
    # print(f"{bytes_sent=}")
    return bytes_sent






def stop(client_data):
    if client_data.status != "on":
        return
    client_data.socket_obj.shutdown(socket.SHUT_RDWR)
    client_data.socket_obj.close()
    # if client_data.socket_obj:
    #     client_data.socket_obj.kill()
    # print(f"********************** {os.getpid()=} {client_data.subprocess=}")


if __name__ == "__main__":
    import os, sys
    rem_base_path = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
    sys.path.append(f"{rem_base_path}/RemPythonCommon")
    from utils import EasyDict

    client_data_= EasyDict()
    client_data_.server_ip = "localhost"
    client_data_.server_port = 20000 + 1

    start_auto(client_data_)
    pack_ = bytearray([10, 99, 10, 10, 10])
    pack_.extend(b"TEMP TCP MESSAGE 1111111 !!!!!!!!!!!!!!")
    send_raw(client_data_, pack_)
    stop(client_data_)

