#!/usr/bin/env python3

import os
import socket
import multiprocessing
import traceback


# https://docs.python.org/3/library/socketserver.html#socketserver.TCPServer
# https://codesource.io/creating-python-socket-server-with-multiple-clients/

# RemOrchestrator = None
# EasyDict = None

# def set_orchestrator(remOrchestrator_):
#     RemOrchestrator = remOrchestrator_
#     EasyDict = RemOrchestrator.utils.EasyDict




def start_auto(server_data):
    print(f" -X-  AMHERE OSI4TcpServer start_auto {os.getpid()=}")
    server_data.name = "TCP SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")
    server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))
    # server_data.socket_obj.listen(1)
    server_data.socket_obj.listen()

    server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=False)
    # server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=True)
    server_data.subprocess.start()


    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()

    return server_data




def stop(server_data):
    print(f" -X-  AMHERE OSI4TcpServer stop {os.getpid()=}")
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    server_data.socket_obj.close()

    # server_data.socket_obj.close()
    # # print(f"********************** {os.getpid()=} {server_data.subprocess=}")
    # if server_data.subprocess and server_data.subprocess.is_alive() :
    #     # server_data.subprocess.terminate()
    #     server_data.subprocess.kill()




def clieant_listener(server_data, conn_obj):
    print(f" -X-  AMHERE OSI4TcpServer clieant_listener {os.getpid()=}")
    # try:
    while(True):
        # [data, address] = conn_obj.recv_from(4096)
        data = conn_obj.recv(4096)

        if len(data) == 0:
            raise Exception(f"Invalid data received {data=}")


        server_data.packets_queue.append(data)
    conn_obj.close()
        # print(f"process tcp recv {data=}")
    # except KeyboardInterrupt:
    #     print(f" *** OSI4TcpServer.py clieant_listener KeyboardInterrupt")
    #     conn_obj.close()
    # except:
    #     traceback.print_exc()
    # finally:
    #     traceback.print_exc()
    #     print(f" *** OSI4TcpServer.py clieant_listener finally")
    #     conn_obj.close()



def server_listener(server_data):
    print(f" -X-  AMHERE OSI4TcpServer server_listener {os.getpid()=}")
    print(f" {multiprocessing.parent_process=}  ")
    # Listen for incoming datagrams
    try:
        while(True):
            conn_obj, client_address = server_data.socket_obj.accept()

            client_proc = multiprocessing.Process(target=clieant_listener, args=[server_data, conn_obj],daemon=True)
            client_proc.start()
            conn_obj.close() # imediatly close on parent process to leave it only on child

    # except KeyboardInterrupt:
    #     print(f" *** OSI4TcpServer.py  KeyboardInterrupt")
    # except:
    #     traceback.print_exc()

    finally:
        traceback.print_exc()
        print(f" *** OSI4TcpServer.py server_listener finally")
        stop(server_data)



