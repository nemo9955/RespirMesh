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
    # RemOrchestrator.stop()
    sys.exit(0)


# signal.signal(signal.SIGINT, close_all)
# signal.signal(signal.SIGTERM, close_all) # This seems to be enough



def randomize_distances(mesh_node_data, node_data, wiggle):
    dists = list()
    for n2, pt2 in mesh_node_data.coords.items():
        if n2 == node_data.k:
            continue
        distance = math.dist(node_data.coord, pt2)
        distance *= random.uniform(1-wiggle,1+wiggle)
        dists.append((distance, n2))
    dists.sort(reverse=False)
    node_data.dists = dists



def generate_mesh(mesh_path, ip, port, server_ip, server_port):
    mesh_data = EasyDict()

    # grid_size_x = 50
    # grid_size_y = 10
    # points_count = 3

    grid_size_x = 130
    grid_size_y = 35
    points_count = 20

    mesh_data.max_dist = (grid_size_x ** 2 - grid_size_y ** 2) ** 0.5
    mesh_data.min_dist = ((grid_size_x * grid_size_y) / points_count) * 0.05
    mesh_data.avg_dist = mesh_data.max_dist / 2

    rx = int(grid_size_x / 4)
    ry = int(grid_size_y / 4)

    coords = []
    coords.append((rx, ry, "root", 0, "R"))
    check_coords = []
    check_coords.append((rx,ry ))

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
    mesh_data.nodes = EasyDict()
    mesh_data.node_paths = list()
    mesh_data.node_paths.append(mesh_path)
    for x,y,k,n,v in coords:
        # print(x,y,n)
        matr[y][x] = v
        mesh_data.coords[k] = [x,y]
        mesh_data.nodes[k] = EasyDict()
        mesh_data.nodes[k].connected_to_root = False
        mesh_data.nodes[k].n = n
        mesh_data.nodes[k].k = k
        mesh_data.nodes[k].v = v
        mesh_data.nodes[k].coord = [x,y]
        mesh_data.nodes[k].server_ip = ip
        mesh_data.nodes[k].server_port = port + n*10 + 0
        if k != "root":
            node_path = mesh_path.replace(".",f".{k}.")
            mesh_data.node_paths.append(node_path)

    for _, node_data in mesh_data.nodes.items():
        randomize_distances(mesh_data, node_data, 0)

    mesh_data.nodes.root.server_ip = server_ip
    mesh_data.nodes.root.server_port = server_port
    mesh_data.nodes.root.connected_to_root = True

    mesh_data.a=list()
    for col in matr:
        line=""
        for ch in col:
            line+=ch
        mesh_data.a.append(line)


    print(f"{mesh_path=}")
    print(f"{mesh_path=}")
    print(f"{mesh_path=}")
    print(f"{mesh_path=}")
    print(f"{mesh_path=}")
    with open(mesh_path, 'w') as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)


def start(mesh_data, device_id):
    node_data = mesh_data.nodes[device_id]

    if device_id == 0 :
        device_id = 1000 + int(random.random() * 1000)

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

    my_server_ip = node_data.server_ip
    my_server_port = node_data.server_port
    server_port_tcp = my_server_port + 1
    server_port_udp = my_server_port + 2


    # print(f"{my_server_ip=}")
    # print(f"{server_port_tcp=}")
    # print(f"{server_port_udp=}")

    packets_queue = list()
    logic_queue = list()

    RemOrchestrator.set_packets_queue(packets_queue)
    RemOrchestrator.set_logic_queue(logic_queue)


    server_data_tcp = RemOrchestrator.init_server_type_1(OSI4TcpServer, my_server_ip, server_port_tcp)
    server_data_udp = RemOrchestrator.init_server_type_1(OSI4UdpServer, my_server_ip, server_port_udp)

    scanner_fake_data = RemOrchestrator.init_scanner_type_1(OSI2FileScanner)
    scanner_fake_data.mesh_path = mesh_path
    scanner_fake_data.node_path = node_path
    scanner_fake_data.randomize_distances = randomize_distances
    scanner_fake_data.server_data_tcp = server_data_tcp
    scanner_fake_data.server_data_udp = server_data_udp

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
    node_path = mesh_path.replace(".",f".{device_id}.")

    # print(f"{mesh_path=}")
    # print(f"{device_id=}")
    mesh_data = None
    # while not os.path.exists(mesh_path):
    #     time.sleep(1)
    with open(mesh_path, "r") as file_:
        mesh_data = json.load(file_,object_pairs_hook=EasyDict)


    node_data = mesh_data.nodes[device_id]
    mesh_data.nodes = EasyDict()
    mesh_data.nodes[device_id] = node_data
    mesh_data.device_id = device_id

    with open(node_path, 'w') as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)

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

