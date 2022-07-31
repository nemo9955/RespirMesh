#!/usr/bin/env python3

import os
import socket
import traceback



import _thread




# https://codezup.com/socket-server-with-multiple-clients-model-multithreading-python/
# https://stackoverflow.com/questions/6380057/python-binding-socket-address-already-in-use


ThreadCount = 0

def set_data(server_data):
    server_data.name = "TCP THREAD SERVER"
    server_data.protocol = "ip_tcp"

def start_auto(server_data):
    print(f" -X-  AMHERE OSI4TcpServerThread start_auto {os.getpid()=}")
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")


    server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_data.status = "on"

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





def clieant_listener(server_data, conn_obj):
    global ThreadCount
    print(f" -X-  AMHERE OSI4TcpServerThread clieant_listener {os.getpid()=}")
    # print(f"{conn_obj=}")
    # conn_obj.settimeout(60)

    while(True):
        packet_ = conn_obj.recv(4096)

        if len(packet_) == 0:
            # print(f"{conn_obj=}")
            break

        packet_size_ = server_data.RemOrchestrator.RemHeaderTypes.get_size(packet_)
        data_size = len(packet_)
        if packet_size_ != data_size :
            tries=10
            server_data.RemOrchestrator.RemHeaderTypes.print_packet_compact("[WARNING], combined packets: ",packet_)
            while tries > 0: # WHILE !!!!!!!!!!!!!!!!!!!!!!!!!!!!
                tries -= 1

                top_pack_ = packet_[:packet_size_]
                packet_ = packet_[packet_size_:]

                # server_data.RemOrchestrator.RemHeaderTypes.print_packet_compact("\ntop_pack_ .........  : ",top_pack_)
                # server_data.RemOrchestrator.RemHeaderTypes.print_packet_compact("packet_ .........  : ",packet_)

                if top_pack_ == packet_ :
                    # means there is only one packet left
                    # we break before sending top_pack_, so the actual send runs
                    break

                server_data.RemOrchestrator.got_packet_type_1(top_pack_, server_data)

                packet_size_ = server_data.RemOrchestrator.RemHeaderTypes.get_size(packet_)
                data_size = len(packet_)

                # print(f"{packet_size_=}")
                # print(f"{data_size=}")
                # print(f"{packet_=}")

                if data_size == 0 :
                    break
                if packet_size_ == data_size :
                    break
            # print(f"FIXED !!!!!!! {packet_=}")


        server_data.RemOrchestrator.got_packet_type_1(packet_, server_data)

        # server_data.packets_queue.append(packet_)

        # print(f"... server got {packet_=}")
        # if "_PING_" in str(packet_):
        #     print(f"{conn_obj=}")
        #     conn_obj.send(str(packet_).replace("_PING_", "_PONG_").encode())

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
            print(f"{ThreadCount=} os.getpid:{os.getpid()}")

    # except KeyboardInterrupt:
    #     print(f" *** OSI4TcpServer.py  KeyboardInterrupt")
    # except:
    #     traceback.print_exc()

    finally:
        traceback.print_exc()
        print(f" *** OSI4TcpServer.py server_listener finally")
        stop(server_data)

