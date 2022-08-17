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
import RemScanner
import utils
import RemDebugger


import OSI4TcpClient
import OSI4UdpClient
import OSI4TcpServerThread as OSI4TcpServer
import OSI4UdpServerThread as OSI4UdpServer

import OSI2FileScanner

import random
import math
from  utils import EasyDict
import json




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




def generate_mesh(mesh_path, ip, port, server_ip, server_port):
    mesh_data = EasyDict()

    # grid_size_x = 50
    # grid_size_y = 10
    # points_count = 5

    grid_size_x = 140
    grid_size_y = 35
    points_count = 10

    mesh_data.max_dist = (grid_size_x ** 2 - grid_size_y ** 2) ** 0.5
    mesh_data.min_dist = ((grid_size_x * grid_size_y) / points_count) * 0.05
    mesh_data.avg_dist = mesh_data.max_dist / 2

    coords = []
    coords.append((1, 1, "root", 0, "R"))
    check_coords = []
    check_coords.append((1, 1))

    print(f"{mesh_data.max_dist=}")
    print(f"{mesh_data.min_dist=}")


    for i in range(points_count):
        x=None
        y=None
        valid = True
        for iii in range(10000):
            x = random.randint(0, grid_size_x-1)
            y = random.randint(0, grid_size_y-1)
            valid = True
            for other_cord in check_coords:
                dst = math.dist(other_cord, (x,y))
                dyn_min = max((mesh_data.min_dist - (iii/500)) , 1)
                if iii > 100 and dst < dyn_min :
                    valid = False
                elif dst < mesh_data.min_dist:
                    valid = False
            if valid:
                break
        if not valid :
            print("Adding invalid:",i,x, y)
        coords.append((x, y, f"{i+10}", i+10, str(i+10)))
        check_coords.append((x, y))

    mesh_data.start = coords[1][2]
    mesh_data.stop = coords[-1][2]

    matr=list()
    for i in range(grid_size_y):
        x_fill = [" "]*grid_size_x
        matr.append(x_fill)

    mesh_data.coords = EasyDict()
    # mesh_data.matr = matr
    for x,y,k,n,v in coords:
        # print(x,y,n)
        matr[y][x] = v
        mesh_data.coords[k] = EasyDict()
        mesh_data.coords[k].connected_to_root = False
        mesh_data.coords[k].y = y
        mesh_data.coords[k].n = n
        mesh_data.coords[k].k = k
        mesh_data.coords[k].v = v
        mesh_data.coords[k].coord = [x,y]
        mesh_data.coords[k].server_ip = ip
        mesh_data.coords[k].server_port = port + n*100 + 0
        mesh_data.coords[k].client_ip = ip
        mesh_data.coords[k].client_port = port + n*100 + 10

    mesh_data.coords.root.server_ip = server_ip
    mesh_data.coords.root.server_port = server_port
    mesh_data.coords.root.client_ip = None
    mesh_data.coords.root.client_port = None
    mesh_data.coords.root.connected_to_root = True

    # for n1, pt1 in mesh_data.coords.items():
    #     pt1.dists = EasyDict()
    #     for n2, pt2 in mesh_data.coords.items():
    #         pt1.dists[n2] = math.dist(pt1.coord, pt2.coord)

    mesh_data.a=list()
    for col in matr:
        line=""
        for ch in col:
            line+=ch
        mesh_data.a.append(line)


    with open(mesh_path, 'w') as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)




def update_json_data(mesh_data):
    node_copy = mesh_data.coords[device_id].copy()

    with open(mesh_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)

    mesh_data.coords[device_id] = node_copy

    with open(mesh_path, 'w') as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)

    node_data = mesh_data.coords[device_id]

    return mesh_data, node_data


def start(mesh_data, device_id):
    node_data = mesh_data.coords[device_id]

    connect_to_ip = node_data.client_ip
    connect_to_port = node_data.client_port
    my_server_ip = node_data.server_ip
    my_server_port = node_data.server_port

    # mesh_data, node_data = update_json_data(mesh_data) #########################

    if device_id == 0 :
        device_id = 1000 + int(random.random() * 1000)

    # device_id = f"msh_{device_id}"

    # log.info(f"{connect_to_ip=}")
    # log.info(f"{my_server_ip=}")
    # log.info(f"{connect_to_port=}")
    # log.info(f"{my_server_port=}")
    # log.info(f"{device_id=}")

    RemHardware.set_device_id(device_id)

    RemOrchestrator.set_hardware(RemHardware)
    RemOrchestrator.set_router(RemRouter)
    RemOrchestrator.set_logger(log)
    RemOrchestrator.set_scanner(RemScanner)
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
    chance = random.random()
    chance = 0.5


    # server_data_tcp = RemOrchestrator.init_server_type_1(OSI4TcpServer, my_server_ip, server_port_tcp)
    # if chance > 0.3 :
    #     client_data_tcp = RemOrchestrator.init_client_type_1(OSI4TcpClient, connect_to_ip, connect_port_tcp)
    #     RemOrchestrator.link_bidir_server_type_1(server_data_tcp, OSI4TcpClient)
    #     RemOrchestrator.link_bidir_client_type_1(server_data_tcp, client_data_tcp)

    # server_data_udp = RemOrchestrator.init_server_type_1(OSI4UdpServer, my_server_ip, server_port_udp)
    # if chance < 0.6 :
    #     client_data_udp = RemOrchestrator.init_client_type_1(OSI4UdpClient, connect_to_ip, connect_port_udp)
    #     RemOrchestrator.link_bidir_server_type_1(server_data_udp, OSI4UdpClient)
    #     RemOrchestrator.link_bidir_client_type_1(server_data_udp, client_data_udp)


    scanner_fake_data = RemOrchestrator.init_scanner_type_1(OSI2FileScanner)
    scanner_fake_data.mesh_path = mesh_path

    limiter=10000
    # RemOrchestrator.begin()
    while limiter > 0:
        # limiter-=1
        RemOrchestrator.update()



if sys.argv[1] == "generate_mesh":
    generate_mesh(sys.argv[2],sys.argv[3],int(sys.argv[4]),sys.argv[5],int(sys.argv[6]))
    sys.exit()


if sys.argv[1] == "run_node":
    mesh_path = sys.argv[2]
    device_id = sys.argv[3]

    # print(f"{mesh_path=}")
    # print(f"{device_id=}")
    mesh_data = None
    with open(mesh_path, "r") as file_:
        mesh_data = json.load(file_,object_pairs_hook=EasyDict)

    try:
        start(mesh_data, device_id)
    except KeyboardInterrupt:
        print("Interrupt closing things")
        close_all()
    except:
        traceback.print_exc()
        close_all()
    finally:
        traceback.print_exc()
        print("Finally closing things")
        close_all()
