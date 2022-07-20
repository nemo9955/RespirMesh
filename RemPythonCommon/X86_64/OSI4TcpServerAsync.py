#!/usr/bin/env python3

import os
import socket
import multiprocessing
import traceback
import asyncio

# https://stackoverflow.com/questions/48506460/python-simple-socket-client-server-using-asyncio



def start_auto(server_data):
    server_data.name = "TCP ASYNC SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")

    asyncio.run(run_server(server_data))


    # server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))
    # # server_data.socket_obj.listen(1)
    # server_data.socket_obj.listen()

    # server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=False)
    # # server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=True)
    # server_data.subprocess.start()


    return server_data



def stop(server_data):
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    server_data.socket_obj.close()

async def handle_client(reader, writer,server_data):
    request = None
    print(f" -X-  AMHERE OSI4TcpServerAsync 38 ")
    while request != 'quit':
        request = (await reader.read(255)).decode('utf8')
        print(f"{request=}")
        server_data.packets_queue.append(request)
        # response = str(eval(request)) + '\n'
        # writer.write(response.encode('utf8'))
        # await writer.drain()
    writer.close()

async def run_server(server_data):
    server = await asyncio.start_server(lambda r, w: handle_client(r, w, session), server_data.server_ip, server_data.server_port)
    async with server:
        print(f" -X-  AMHERE OSI4TcpServerAsync 50 ")
        await server.serve_forever()


# def clieant_listener(server_data, conn_obj):
#     # try:
#     while(True):
#         # [data, address] = conn_obj.recv_from(4096)
#         data = conn_obj.recv(4096)

#         server_data.packets_queue.append(data)
#     conn_obj.close()


# def server_listener(server_data):
#     # Listen for incoming datagrams
#     try:
#         while(True):
#             conn_obj, client_address = server_data.socket_obj.accept()

#             client_proc = multiprocessing.Process(target=clieant_listener, args=[server_data, conn_obj],daemon=True)
#             client_proc.start()
#             conn_obj.close() # imediatly close on parent process to leave it only on child


#     finally:
#         traceback.print_exc()
#         print(f" *** OSI4TcpServer.py server_listener finally")
#         stop(server_data)



