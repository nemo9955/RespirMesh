#!/usr/bin/env python3


# https://blog.ruanbekker.com/blog/2019/02/19/sharing-global-variables-in-python-using-multiprocessing/

# temp use for debugging
import time

import traceback
import signal
import socket
import sys
import os

import multiprocessing


rem_base_path = os.path.dirname(os.path.dirname(__file__))
# print(f"{rem_base_path=}")
sys.path.append(f"{rem_base_path}/RemPythonCommon/X86_64")
sys.path.append(f"{rem_base_path}/RemPythonCommon")


import RemHardwareMock as RemHardware
import RemLogger as log
import RemOrchestrator
import RemRouter
import RemHeaderTypes
# import RemScanner
import utils
import RemDebugger
import RemServer


import OSI4TcpClient
import OSI4UdpClient
# import OSI4TcpServerMproc as OSI4TcpServer
# import OSI4TcpServerAsync as OSI4TcpServer
# import OSI4TcpServerHelper as OSI4TcpServer
import OSI4TcpServerThread as OSI4TcpServer

# import OSI4UdpServerMproc as OSI4UdpServer
import OSI4UdpServerThread as OSI4UdpServer




print("\n\n\n")


did_close=False
def close_all(num=None, frame=None):
    global did_close
    print(f"{did_close=} !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    if did_close :return
    did_close=True
    print(f"server close_all() {os.getpid()=}  {device_id=} ")
    RemOrchestrator.stop()
    sys.exit(0)

# signal.signal(signal.SIGINT, close_all)
signal.signal(signal.SIGTERM, close_all) # This seems to be enough






if len(sys.argv) != 4:
    print("First 2 arguments specify host and port of the server")
    print("3th arg, the ID of the device, random if 0")
    sys.exit(1)

server_ip = sys.argv[1]
server_port = int(sys.argv[2]) # not used directly !!!!!!!!!!
device_id = sys.argv[3]


if device_id == 0 :
    device_id = int(random() * 1000)

server_port_tcp = server_port + 1
server_port_udp = server_port + 2

log.info(f"{server_ip=}")
log.info(f"{server_port=}")
log.info(f"{device_id=}")
log.info(f"{server_port_tcp=}")
log.info(f"{server_port_udp=}")


RemHardware.set_device_id(device_id)

RemOrchestrator.set_orchestrator(RemOrchestrator)
RemOrchestrator.set_hardware(RemHardware)
RemOrchestrator.set_router(RemRouter)
RemOrchestrator.set_logger(log)
RemOrchestrator.set_header(RemHeaderTypes)
RemOrchestrator.set_utils(utils)
RemOrchestrator.set_debugger(RemDebugger)
RemOrchestrator.set_server(RemServer)

RemOrchestrator.init() # needs to be done after linking modules
RemOrchestrator.set_root()


# manager = multiprocessing.Manager()
# packets_queue = manager.list()
packets_queue = list()
logic_queue = list()


RemOrchestrator.set_packets_queue(packets_queue)
RemOrchestrator.set_logic_queue(logic_queue)


server_data_tcp = RemOrchestrator.init_server_type_1(OSI4TcpServer, server_ip, server_port_tcp)
RemOrchestrator.link_bidir_server_type_1(server_data_tcp, OSI4TcpClient)


server_data_udp = RemOrchestrator.init_server_type_1(OSI4UdpServer, server_ip, server_port_udp)
RemOrchestrator.link_bidir_server_type_1(server_data_udp, OSI4UdpClient)


def main():
    limiter=3333333
    # RemOrchestrator.begin()
    while limiter > 0:
        # limiter-=1
        RemOrchestrator.update()



try:
    main()
except KeyboardInterrupt:
    print("Interrupt closing things")
    # close_all()
except:
    traceback.print_exc()
finally:
    traceback.print_exc()
    print("Finally closing things")
    close_all()






