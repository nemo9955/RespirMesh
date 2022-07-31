#!/usr/bin/env python3


import os
import socket
import multiprocessing
import traceback

# RemOrchestrator = None
# EasyDict = None

# def set_orchestrator(remOrchestrator_):
#     RemOrchestrator = remOrchestrator_
#     EasyDict = RemOrchestrator.utils.EasyDict

def start_auto(server_data):
    server_data.name = "UDP SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")

    server_data.socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))

    server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=True)
    server_data.subprocess.start()

    return server_data



def stop(server_data):
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()
    pass

    # server_data.socket_obj.close()
    # # print(f"********************** {os.getpid()=} {server_data.subprocess=}")
    # if server_data.subprocess and server_data.subprocess.is_alive() :
    #     # server_data.subprocess.terminate()
    #     server_data.subprocess.kill()


def server_listener(server_data):
    # Listen for incoming datagrams
    try:
        while(True):
            # bytesAddressPair = server_data.socket_obj.recvfrom(4096)
            [data, address] = server_data.socket_obj.recvfrom(4096)
            # data = bytesAddressPair[0]
            # address = bytesAddressPair[1]

            server_data.packets_queue.append(data)
            # print(f"process udp recv {data=}")

            # Sending a reply to client
            # UDPServerSocket.sendto(bytesToSend, address)
    except KeyboardInterrupt:
        print(f" *** OSI4UdpServer.py KeyboardInterrupt")
    except:
        traceback.print_exc()
    finally:
        print(f" *** OSI4UdpServer.py finally")
        stop(server_data)




