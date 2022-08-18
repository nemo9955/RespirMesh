#!/usr/bin/env python3

# Dummy implementation to prepare locally for WiFi scanner and changer on ESP devices
# Will use a local file that will act as the transmission medium for the Data Link OSI level 2
# Each node will have a 2D coordinate assigned in a grid that will be used to estimate the distance
# between the nodes and the ROOT that is simulated also on that grid


import time
import traceback
import json
import math
import random
import os


import OSI4TcpClient
import OSI4UdpClient
import OSI4TcpServerThread as OSI4TcpServer
import OSI4UdpServerThread as OSI4UdpServer


RemOrchestrator = None
EasyDict = None
log = None
def set_orchestrator(set_value_):
    global RemOrchestrator
    global EasyDict
    global log
    RemOrchestrator = set_value_
    EasyDict = RemOrchestrator.EasyDict
    log = set_value_.log

def set_data(scanner_data):
    scanner_data.name = "Fake WiFi"
    scanner_data.protocol = "802.11"

def begin(scanner_data):
    # scanner_data.____ =random.randint(20, 40)

    with open(scanner_data.node_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    node_data = mesh_data.nodes[scanner_data.device_id]

    scanner_data.when_to_update = node_data.n - 8
    print(f"{scanner_data.when_to_update=}")

    scanner_data.scan_counter = 0

def stop(scanner_data):
    pass

def update_continous(scanner_data, delta):
    pass


def update_discrete(scanner_data, counter):
    if counter % 30 == scanner_data.when_to_update :

        if not os.path.exists(scanner_data.mesh_path):
            return

        scanner_data.scan_counter+=1
        mesh_data, mesh_node_data, node_data = get_mesh_data(scanner_data)
        # print(f"mesh_data={json.dumps(mesh_data, indent=4, sort_keys=True)}")

        wiggle = min(0.5,(scanner_data.scan_counter/10))
        scanner_data.randomize_distances(mesh_node_data, node_data, wiggle)
        scan_closest_root_link(scanner_data, mesh_data, mesh_node_data, node_data)

        with open(scanner_data.node_path, "w") as file_:
            json.dump(mesh_node_data, file_, indent=4, sort_keys=True)



def connect_to_node(scanner_data, mesh_data, mesh_node_data, node_data, parent_node):
    node_data.connected_to_id = parent_node.k
    node_data.connected_to_root = parent_node.connected_to_root

    scanner_data.connected_to_id = node_data.connected_to_id
    scanner_data.connected_to_root = node_data.connected_to_root

    print(f"linking {node_data.k=} to {parent_node.k=}")
    # print(f"node_data={json.dumps(node_data, indent=4, sort_keys=True)}")
    # print(f"parent_node={json.dumps(parent_node, indent=4, sort_keys=True)}")

    # parent_node.server_ip
    # parent_node.server_port

    server_ip = parent_node.server_ip
    server_port_tcp = parent_node.server_port + 1
    server_port_udp = parent_node.server_port + 2

    # randomly we try to create a client with tcp or udp or both
    chance = random.random()
    chance = 0.5

    if chance > 0.3 :
        client_data_tcp = RemOrchestrator.init_client_type_1(OSI4TcpClient, server_ip, server_port_tcp)
        RemOrchestrator.link_bidir_server_type_1(scanner_data.server_data_tcp, OSI4TcpClient)
        RemOrchestrator.link_bidir_client_type_1(scanner_data.server_data_tcp, client_data_tcp)

    if chance < 0.6 :
        client_data_udp = RemOrchestrator.init_client_type_1(OSI4UdpClient, server_ip, server_port_udp)
        RemOrchestrator.link_bidir_server_type_1(scanner_data.server_data_udp, OSI4UdpClient)
        RemOrchestrator.link_bidir_client_type_1(scanner_data.server_data_udp, client_data_udp)




def get_mesh_data(scanner_data):
    with open(scanner_data.mesh_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    mesh_node_data=None
    for node_path in mesh_data.node_paths:
        if not os.path.exists(node_path):
            continue
        with open(node_path, "r") as file_:
            mesh_node_data_ = json.load(file_, object_pairs_hook=EasyDict)
            if node_path == scanner_data.node_path:
                mesh_node_data = mesh_node_data_

            # mesh_data.update(mesh_node_data_)
            mesh_data.nodes.update(mesh_node_data_.nodes)
            # mesh_data.nodes[mesh_node_data_.device_id] = mesh_node_data_.nodes[mesh_node_data_.device_id]

    node_data = mesh_node_data.nodes[scanner_data.device_id]
    return mesh_data, mesh_node_data, node_data



def scan_closest_root_link(scanner_data, mesh_data, mesh_node_data, node_data):
    # with open(scanner_data.node_path, "r") as file_:
    #     mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    # mesh_data, mesh_node_data, node_data = get_mesh_data(scanner_data)

    if "dists" not in node_data:
        return

    if node_data.connected_to_root :
        return

    for i in range(len(node_data.dists)):
        checker = min(1,(scanner_data.scan_counter/10))
        if i > len(node_data.dists) * checker:
            break

        try_node = mesh_data.nodes[node_data.dists[i][1]]
        # print(f"try_node={json.dumps(try_node, indent=4, sort_keys=True)}")
        # if "dists" not in try_node:
        #     continue
        print(f"{node_data.k=} ---1---> {try_node.k=}")
        if try_node.connected_to_root == False:
            continue
        connect_to_node(scanner_data, mesh_data, mesh_node_data, node_data, try_node)
        break

    # with open(scanner_data.node_path, "w") as file_:
    #     json.dump(mesh_node_data, file_, indent=4, sort_keys=True)
