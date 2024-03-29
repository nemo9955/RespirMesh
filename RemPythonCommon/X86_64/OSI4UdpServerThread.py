#!/usr/bin/env python3


import os
import socket
import traceback


import _thread



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



def set_data(server_data):
    server_data.name = "UDP SERVER"
    server_data.protocol = "ip_udp"

def start_auto(server_data):
    print(f" -X-  AMHERE start_auto stop {os.getpid()=}")
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")

    server_data.socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    server_data.status = "on"

    _thread.start_new_thread(server_listener, (server_data, ))

    return server_data



def stop(server_data):
    print(f" -X-  AMHERE OSI4UdpServerThread stop {os.getpid()=}")
    if server_data.status != "on" :
        return
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()

    # server_data.socket_obj.close()
    # # print(f"********************** {os.getpid()=} {server_data.subprocess=}")
    # if server_data.subprocess and server_data.subprocess.is_alive() :
    #     # server_data.subprocess.terminate()
    #     server_data.subprocess.kill()


def server_listener(server_data):
    print(f" -X-  AMHERE OSI4UdpServerThread server_listener {os.getpid()=}")


    server_data.socket_obj.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # REUSE IF ALREADY EXISTING !!!!!
    server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))

    # Listen for incoming datagrams
    try:
        while(True):
            # bytesAddressPair = server_data.socket_obj.recvfrom(4096)
            [data, address] = server_data.socket_obj.recvfrom(4096)
            # print(f"udp recv ............ {address=}")
            # data = bytesAddressPair[0]
            # address = bytesAddressPair[1]


            if len(data) == 0:
                # raise Exception(f"Invalid data received {data=}")
                continue



            server_data.RemOrchestrator.got_packet_type_1(data, server_data)
            # server_data.packets_queue.append(data)
            # print(f"process udp recv {data=}")

            # Sending a reply to client
            # UDPServerSocket.sendto(bytesToSend, address)
    # except KeyboardInterrupt:
    #     print(f" *** OSI4UdpServerThread.py KeyboardInterrupt")
    # except:
    #     traceback.print_exc()
    finally:
        print(f" *** OSI4UdpServerThread.py finally")
        # traceback.print_exc()
        stop(server_data)





