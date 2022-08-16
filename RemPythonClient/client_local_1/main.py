#!/usr/bin/env python3

# https://stackoverflow.com/questions/2546276/python-process-wont-call-atexit


import traceback
import sys
import os
from random import random


import multiprocessing

import signal
import atexit

rem_base_path = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
# print(f"{rem_base_path=}")
sys.path.append(f"{rem_base_path}/RemPythonCommon/X86_64")
sys.path.append(f"{rem_base_path}/RemPythonCommon")

import RemHardwareMock as RemHardware
import RemLogger as log
import RemOrchestrator
import RemRouter
import RemHeaderTypes
import RemChannel
import utils
import RemDebugger

import OSI4TcpClient
import OSI4UdpClient
# import OSI4TcpServerMproc as OSI4TcpServer
# import OSI4TcpServerAsync as OSI4TcpServer
# import OSI4TcpServerHelper as OSI4TcpServer
import OSI4TcpServerThread as OSI4TcpServer


# import OSI4UdpServerMproc as OSI4UdpServer
import OSI4UdpServerThread as OSI4UdpServer




did_close=False
def close_all(num=None, frame=None):
    global did_close
    print(f"{did_close=} !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
    if did_close : return
    did_close=True
    print(f"client close_all() {os.getpid()=}  {device_id=} ")
    RemOrchestrator.stop()
    sys.exit(0)


# signal.signal(signal.SIGINT, close_all)
# signal.signal(signal.SIGTERM, close_all) # This seems to be enough


# print(f"{sys.argv=}")

if len(sys.argv) != 6:
    print("First 2 arguments specify host and port of the server")
    print("3th arg, the ID of the device, random if 0")
    print("Next sets of 2 arguments specify host and port of client")
    sys.exit(1)

connect_to_ip = sys.argv[1]
connect_to_port = int(sys.argv[2])
device_id = sys.argv[3]
my_server_ip = sys.argv[4]
my_server_port = int(sys.argv[5])


if device_id == 0 :
    device_id = 1000 + int(random() * 1000)


log.info(f"{connect_to_ip=}")
log.info(f"{my_server_ip=}")
log.info(f"{connect_to_port=}")
log.info(f"{my_server_port=}")
log.info(f"{device_id=}")

RemHardware.set_device_id(device_id)

# log.info(f"{RemHardware.device_id()=}")
# log.info(f"{RemHardware.time_milis()=}")
# RemHardware.sleep_milis(50)
# log.info(f"{RemHardware.time_milis()=}")



RemOrchestrator.set_hardware(RemHardware)
RemOrchestrator.set_router(RemRouter)
RemOrchestrator.set_logger(log)
# RemOrchestrator.set_scanner(____)
RemOrchestrator.set_channel(RemChannel)
RemOrchestrator.set_header(RemHeaderTypes)
RemOrchestrator.set_utils(utils)
RemOrchestrator.set_orchestrator(RemOrchestrator)
RemOrchestrator.set_debugger(RemDebugger)




RemOrchestrator.init() # needs to be done after linking modules


connect_port_tcp = connect_to_port + 1
server_port_tcp = my_server_port + 1

connect_port_udp = connect_to_port + 2
server_port_udp = my_server_port + 2

packets_queue = list()
logic_queue = list()


RemOrchestrator.set_packets_queue(packets_queue)
RemOrchestrator.set_logic_queue(logic_queue)

# randomly we try to create a client with tcp or udp or both
chance = random()
# chance = 0.5


server_data_tcp = RemOrchestrator.init_server_type_1(OSI4TcpServer, my_server_ip, server_port_tcp)
if chance > 0.3 :
    client_data_tcp = RemOrchestrator.init_client_type_1(OSI4TcpClient, connect_to_ip, connect_port_tcp)
    RemOrchestrator.link_bidir_server_type_1(server_data_tcp, OSI4TcpClient)
    RemOrchestrator.link_bidir_client_type_1(server_data_tcp, client_data_tcp)

server_data_udp = RemOrchestrator.init_server_type_1(OSI4UdpServer, my_server_ip, server_port_udp)
if chance < 0.6 :
    client_data_udp = RemOrchestrator.init_client_type_1(OSI4UdpClient, connect_to_ip, connect_port_udp)
    RemOrchestrator.link_bidir_server_type_1(server_data_udp, OSI4UdpClient)
    RemOrchestrator.link_bidir_client_type_1(server_data_udp, client_data_udp)

print(f" 000 {os.getpid()=}  {device_id=}")

def main():
    limiter=10000
    # RemOrchestrator.begin()
    while limiter > 0:
        # limiter-=1
        RemOrchestrator.update()



try:
    main()
except KeyboardInterrupt:
    print("Interrupt closing things")
    close_all()
except:
    traceback.print_exc()
finally:
    traceback.print_exc()
    print("Finally closing things")
    close_all()
