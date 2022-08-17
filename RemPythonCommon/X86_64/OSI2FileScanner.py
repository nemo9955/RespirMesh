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


RemOrchestrator = None
EasyDict = None
def set_orchestrator(set_value_):
    global RemOrchestrator
    global EasyDict
    RemOrchestrator = set_value_
    EasyDict = RemOrchestrator.EasyDict

def set_data(scanner_data):
    scanner_data.name = "Fake WiFi"
    scanner_data.protocol = "802.11"

def begin(scanner_data):
    # scanner_data.____ =random.randint(20, 40)

    with open(scanner_data.mesh_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    node_data = mesh_data.coords[scanner_data.device_id]

    scanner_data.when_to_update = node_data.n
    # print(f"{scanner_data.when_to_update=}")

def stop(scanner_data):
    pass

def update_continous(scanner_data, delta):
    pass


def update_discrete(scanner_data, counter):
    if counter % 50 == scanner_data.when_to_update :
        randomize_distances(scanner_data)
        scan_closest_root_link(scanner_data)


def scan_closest_root_link(scanner_data):
    with open(scanner_data.mesh_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    node_data = mesh_data.coords[scanner_data.device_id]

    if "dists" not in node_data:
        return


    for i in range(len(node_data.dists)):
        if i > len(node_data.dists) * 0.3:
            break
        try_node = mesh_data.coords[node_data.dists[i][1]]
        if try_node.connected_to_root == False:
            continue
        node_data.connected_to_id = try_node.k
        node_data.connected_to_root = try_node.connected_to_root
        print(f"linking {node_data.k=} to {try_node.k=}")
        break

    with open(scanner_data.mesh_path, "w") as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)

def randomize_distances(scanner_data):
    with open(scanner_data.mesh_path, "r") as file_:
        mesh_data = json.load(file_, object_pairs_hook=EasyDict)
    node_data = mesh_data.coords[scanner_data.device_id]

    dists = list()
    for n2, pt2 in mesh_data.coords.items():
        if n2 == scanner_data.device_id:
            continue
        distance = math.dist(node_data.coord, pt2.coord)
        wiggle = 0.3
        # distance *= random.uniform(1-wiggle,1+wiggle)
        dists.append((distance, n2))
    dists.sort(reverse=True)
    node_data.dists = dists

    with open(scanner_data.mesh_path, "w") as file_:
        json.dump(mesh_data, file_, indent=4, sort_keys=True)