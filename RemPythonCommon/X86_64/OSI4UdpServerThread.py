#!/usr/bin/env python3


import os
import socket
import traceback


import _thread



def start_auto(server_data):
    print(f" -X-  AMHERE start_auto stop {os.getpid()=}")
    server_data.name = "UDP SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")

    server_data.socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    _thread.start_new_thread(server_listener, (server_data, ))

    return server_data



def stop(server_data):
    print(f" -X-  AMHERE OSI4UdpServerThread stop {os.getpid()=}")
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()
    pass

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


            server_data.packets_queue.append(data)
            # print(f"process udp recv {data=}")

            # Sending a reply to client
            # UDPServerSocket.sendto(bytesToSend, address)
    # except KeyboardInterrupt:
    #     print(f" *** OSI4UdpServerThread.py KeyboardInterrupt")
    # except:
    #     traceback.print_exc()
    finally:
        print(f" *** OSI4UdpServerThread.py finally")
        stop(server_data)




