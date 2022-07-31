#!/usr/bin/env python3

import os
import socket
import multiprocessing
import traceback
import socketserver



# https://docs.python.org/3/library/socketserver.html#socketserver-tcpserver-example



def start_auto(server_data):
    print(f" -X-  AMHERE OSI4TcpServerHelper start_auto {os.getpid()=}")
    server_data.name = "TCP HELPER SERVER"
    print(f" >>> {server_data.name} {server_data.server_ip} {server_data.server_port}")
    # server_data.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # server_data.socket_obj.bind((server_data.server_ip, server_data.server_port))
    # server_data.socket_obj.listen(1)
    # server_data.socket_obj.listen()

    server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=False)
    # server_data.subprocess = multiprocessing.Process(target=server_listener, args=[server_data],daemon=True)
    server_data.subprocess.start()


    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()

    return server_data





def stop(server_data):
    print(f" -X-  AMHERE OSI4TcpServerHelper stop {os.getpid()=}")
    # server_data.socket_obj.shutdown(socket.SHUT_WR)
    # server_data.socket_obj.close()



class MyTCPHandler(socketserver.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    allow_reuse_address = True


    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(4096).strip()
        self.server.server_data.packets_queue.append(self.data)
        # print("{} wrote:".format(self.client_address[0]))
        # print(self.data)
        # # just send back the same data, but upper-cased
        # self.request.sendall(self.data.upper())



def server_listener(server_data):
    print(f" -X-  AMHERE OSI4TcpServerHelper server_listener {os.getpid()=}")

    # Create the server, binding to localhost on port 9999
    with socketserver.TCPServer((server_data.server_ip, server_data.server_port), MyTCPHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.server_data = server_data
        server.allow_reuse_address = True # allows to close existing socket
        server.serve_forever()

    # # Listen for incoming datagrams
    # try:
    #     while(True):
    #         conn_obj, client_address = server_data.socket_obj.accept()

    #         client_proc = multiprocessing.Process(target=clieant_listener, args=[server_data, conn_obj],daemon=True)
    #         client_proc.start()
    #         conn_obj.close() # imediatly close on parent process to leave it only on child

    # except:
    #     traceback.print_exc()
    # finally:
    #     traceback.print_exc()
    #     print(f" *** OSI4TcpServer.py server_listener finally")
    #     stop(server_data)



