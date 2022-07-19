#!/usr/bin/env python3

import os
import socket
import multiprocessing

# RemOrchestrator = None
# EasyDict = None

# def set_orchestrator(remOrchestrator_):
#     RemOrchestrator = remOrchestrator_
#     EasyDict = RemOrchestrator.utils.EasyDict




def start_auto(server_data):
    server_data.name = "TCP SERVER"

    server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))
    server_data.socket_obj.listen(1)

    server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=True)
    server_data.subprocess.start()

    return server_data




def stop(server_data):
    server_data.socket_obj.close()
    # print(f"********************** {os.getpid()=} {server_data.subprocess=}")
    if server_data.subprocess and server_data.subprocess.is_alive() :
        # server_data.subprocess.terminate()
        server_data.subprocess.kill()



def server_listener(server_data):
    # Listen for incoming datagrams
    try:
        while(True):
            connection, client_address = server_data.socket_obj.accept()
            # bytesAddressPair = connection.recvfrom(4096)
            [data, address] = connection.recvfrom(4096)
            # data = bytesAddressPair[0]
            # address = bytesAddressPair[1]

            server_data.packets_queue.append(data)
            # print(f"process tcp recv {data=}")
    except KeyboardInterrupt:
        print(f" *** OSI4TcpServer.py KeyboardInterrupt")
    finally:
        print(f" *** OSI4TcpServer.py finally")
        stop(server_data)


# def server_listener_tcp(TCPServerSocket, packets_queue):
#     while True:
#         # Wait for a connection
#         # print('waiting for a TCP connection')
#         connection, client_address = TCPServerSocket.accept()
#         print('connection from', client_address)
#         # Receive the data in small chunks and retransmit it
#         while True:
#             data = connection.recv(4096)
#             # print('received {!r}'.format(data))
#             if data:
#                 # print('sending data back to the client')
#                 # connection.sendall(data)
#                 packets_queue.append(data)
#                 print(f"process tcp recv {data=}")
#             else:
#                 # print('no data from', client_address)
#                 break
