#!/usr/bin/env python3

import os
import socket
import traceback



import _thread




# https://codezup.com/socket-server-with-multiple-clients-model-multithreading-python/
# https://stackoverflow.com/questions/6380057/python-binding-socket-address-already-in-use






RemOrchestrator = None
RemHeaderTypes = None
EasyDict = None
log = None
def set_orchestrator(set_value_):
    global RemOrchestrator
    global RemHeaderTypes
    global EasyDict
    global log
    RemOrchestrator = set_value_
    EasyDict = RemOrchestrator.EasyDict
    RemHeaderTypes = RemOrchestrator.RemHeaderTypes
    log = set_value_.log





ThreadCount = 0

def set_data(server_data):
    server_data.name = "TCP SERVER"
    server_data.protocol = "ip_tcp"

def start_auto(server_data):
    print(f" -X-  AMHERE OSI4TcpServerThread start_auto {os.getpid()=}")
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")

    server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    _thread.start_new_thread(server_listener, (server_data, ))

    server_data.status = "on"

    return server_data




def stop(server_data):
    print(f" -X-  AMHERE OSI4TcpServerThread stop {os.getpid()=}")
    if server_data.status != "on" :
        return
    server_data.socket_obj.close()


def stop_client(server_data, conn_obj):
    global ThreadCount
    print(f" -!!!!!!!!!!-  AMHERE OSI4TcpServerThread stop_client {os.getpid()=}")
    ThreadCount-=1
    conn_obj.close()





def clieant_listener(server_data, conn_obj):
    global ThreadCount
    print(f" -X-  AMHERE OSI4TcpServerThread clieant_listener {os.getpid()=}")
    # print(f"{conn_obj=}")
    # conn_obj.settimeout(60)

    while(True):
        packet = conn_obj.recv(4096)

        if len(packet) == 0:
            # print(f"{conn_obj=}")
            break

        packet_size_ = RemHeaderTypes.get_size(packet)
        data_size = len(packet)

        if packet_size_ != data_size :
            log.trace(f"Combined packets detected in TCP client {packet_size_=} {data_size=} :")

            tries=20
            splited_packets=[]

            while tries > 0:
                tries -= 1

                top_pack_ = packet[:packet_size_]
                packet = packet[packet_size_:]

                if top_pack_ in splited_packets:
                    log.trace(f"    skip {RemHeaderTypes.str_packet(top_pack_)}")
                else:
                    log.trace(f"    prep {RemHeaderTypes.str_packet(top_pack_)}")
                    splited_packets.append(top_pack_)

                packet_size_ = RemHeaderTypes.get_size(packet)
                data_size = len(packet)

                if data_size == 0 :
                    break
                if packet_size_ == data_size :
                    break

            for packet in splited_packets:
                RemOrchestrator.got_packet_type_1(packet, server_data)

        else :
            RemOrchestrator.got_packet_type_1(packet, server_data)



    stop_client(server_data, conn_obj)



def server_listener(server_data):
    global ThreadCount
    print(f" -X-  AMHERE OSI4TcpServerThread server_listener {os.getpid()=}")

    server_data.socket_obj.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # REUSE IF ALREADY EXISTING !!!!!

    server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))
    server_data.socket_obj.listen()


    try:
        while(True):
            conn_obj, client_address = server_data.socket_obj.accept()

            # TODO use this conn_obj to instantly create the client for that node !!!!

            _thread.start_new_thread(clieant_listener, (server_data, conn_obj, ))
            ThreadCount += 1
            print(f"{ThreadCount=} os.getpid:{os.getpid()}")

    # except KeyboardInterrupt:
    #     print(f" *** OSI4TcpServer.py  KeyboardInterrupt")
    # except:
    #     traceback.print_exc()

    finally:
        traceback.print_exc()
        print(f" *** OSI4TcpServer.py server_listener finally")
        stop(server_data)

