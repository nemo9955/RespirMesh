#!/usr/bin/env python3

import socket

# RemOrchestrator = None

# def set_orchestrator(remOrchestrator_):
#     RemOrchestrator = remOrchestrator_

def start_auto(client_data):
    client_data.name = "UDP CLIENT"

    # Create a TCP/IP socket
    client_data.socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    # Connect the socket to the port where the server is listening
    client_data.socket_obj.connect((client_data.server_ip, client_data.server_port))
    return client_data


def send_raw(client_data, packet_):
    # print(f" -X-  AMHERE OSI4UdpClient 20 {packet_=}")
    # bytes_sent = client_data.socket_obj.sendall(packet_)
    bytes_sent = client_data.socket_obj.send(packet_)
    # bytes_sent = client_data.socket_obj.sendto(packet_,(client_data.server_ip, client_data.server_port))
    return bytes_sent



def stop(client_data):
    client_data.socket_obj.close()
    # print(f"********************** {os.getpid()=} {client_data.subprocess=}")




# if __name__ == "__main__":
#     import os, sys
#     rem_base_path = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
#     sys.path.append(f"{rem_base_path}/RemPythonCommon")
#     from utils import EasyDict

#     client_data_= EasyDict()
#     client_data_.server_ip = "localhost"
#     client_data_.server_port = 19000 + 2

#     start_auto(client_data_)
#     send_raw(client_data_, b"TEMP UDP MESSAGE 1111111 !!!!!!!!!!!!!!")
#     stop(client_data_)
