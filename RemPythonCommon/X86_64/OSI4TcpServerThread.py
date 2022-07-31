#!/usr/bin/env python3

import os
import socket
import traceback



import _thread




# https://codezup.com/socket-server-with-multiple-clients-model-multithreading-python/
# https://stackoverflow.com/questions/6380057/python-binding-socket-address-already-in-use


ThreadCount = 0


def start_auto(server_data):
    print(f" -X-  AMHERE OSI4TcpServerThread start_auto {os.getpid()=}")
    server_data.name = "TCP THREAD SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")


    server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


    _thread.start_new_thread(server_listener, (server_data, ))


    return server_data





def stop(server_data):
    print(f" -X-  AMHERE OSI4TcpServerThread stop {os.getpid()=}")
    server_data.socket_obj.close()


def stop_client(server_data, conn_obj):
    global ThreadCount
    print(f" -!!!!!!!!!!-  AMHERE OSI4TcpServerThread stop_client {os.getpid()=}")
    ThreadCount-=1
    conn_obj.close()


def make_client_connection(server_data, conn_obj):
    client_data = server_data.RemOrchestrator.EasyDict()
    client_data = server_data.RemOrchestrator.init_client_type_2(server_data.client_logic, server_data, conn_obj)
    client_data.name = "OSI4TcpServerThread client"
    client_data.socket_obj = conn_obj

    message_ = f" !!!!!!!!!!!!! FROM {server_data.name} device_id:{server_data.RemOrchestrator.RemHardware.device_id()}".encode()
    packet_ = server_data.RemOrchestrator.RemHeaderTypes.new_packet_message(message_)
    client_data.client_logic.send_raw(client_data, packet_)


    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!
    # TODO test client can send back to server !!!!!!!!!!!!!!!!!!!!!!!!!



def clieant_listener(server_data, conn_obj):
    global ThreadCount
    print(f" -X-  AMHERE OSI4TcpServerThread clieant_listener {os.getpid()=}")
    # print(f"{conn_obj=}")
    # conn_obj.settimeout(60)
    if server_data.server_client_can_make:
        make_client_connection(server_data, conn_obj)

    while(True):
        data = conn_obj.recv(4096)

        if len(data) == 0:
            # print(f"{conn_obj=}")
            break

        server_data.packets_queue.append(data)

        # print(f"... server got {data=}")
        # if "_PING_" in str(data):
        #     print(f"{conn_obj=}")
        #     conn_obj.send(str(data).replace("_PING_", "_PONG_").encode())

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

            _thread.start_new_thread(clieant_listener, (server_data, conn_obj, ))
            ThreadCount += 1
            print(f"{ThreadCount=} {os.getpid()=}")

    # except KeyboardInterrupt:
    #     print(f" *** OSI4TcpServer.py  KeyboardInterrupt")
    # except:
    #     traceback.print_exc()

    finally:
        traceback.print_exc()
        print(f" *** OSI4TcpServer.py server_listener finally")
        stop(server_data)

