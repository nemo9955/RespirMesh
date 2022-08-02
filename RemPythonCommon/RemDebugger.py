#!/usr/bin/env python3

import os
import sys
import json
from copy import deepcopy

RemOrchestrator = None
RemRouter = None
log = None
RemConnectionController = None
RemHardware = None
RemChannel = None
RemHeaderTypes = None
RemDebugger = None
RemServer = None
utils = None
EasyDict = None
orch_data = None

def set_orchestrator(remOrchestrator_):
    global RemOrchestrator
    global RemRouter
    global log
    global RemConnectionController
    global RemHardware
    global RemChannel
    global RemHeaderTypes
    global RemDebugger
    global RemServer
    global utils
    global EasyDict
    global orch_data

    RemOrchestrator = remOrchestrator_.RemOrchestrator
    orch_data = RemOrchestrator.orch_data
    RemRouter = remOrchestrator_.RemRouter
    log = remOrchestrator_.log
    RemConnectionController = remOrchestrator_.RemConnectionController
    RemHardware = remOrchestrator_.RemHardware
    RemChannel = remOrchestrator_.RemChannel
    RemHeaderTypes = remOrchestrator_.RemHeaderTypes
    RemDebugger = remOrchestrator_.RemDebugger
    RemServer = remOrchestrator_.RemServer
    utils = remOrchestrator_.utils
    EasyDict = utils.EasyDict

DATA_DIR = None

def begin():
    global DATA_DIR
    repo_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
    DATA_DIR = os.path.join(repo_path, "rem_diagrams")
    # print(f"\n\n{DATA_DIR=} !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")

    if not os.path.exists(DATA_DIR):
        os.mkdir(DATA_DIR)

    if RemOrchestrator.orch_data.is_root != True :
        return

    for file_name in os.listdir(DATA_DIR):
        debug_file = os.path.join(DATA_DIR, file_name)
        if os.path.isfile(debug_file):
            print('Deleting file:', debug_file)
            os.remove(debug_file)




def write_file_json(jdata):
    global DATA_DIR
    device_id = RemHardware.device_id()
    filename = f"{device_id}.json"
    file_path = os.path.join(DATA_DIR,filename)
    with open(file_path, 'w') as file_:
        json.dump(jdata, file_, indent=4, sort_keys=True)

def write_file_uml(jdata):
    global DATA_DIR
    device_id = RemHardware.device_id()
    filename = f"{device_id}.puml"
    uml_path = os.path.join(DATA_DIR,filename)

    uml_str = get_str_uml(jdata)

    with open(uml_path, 'w') as file_:
        file_.write(uml_str)


def pop_unwanted(tmp_):
    tmp_.pop("RemOrchestrator",None)
    tmp_.pop("client_logic",None)
    tmp_.pop("server_logic",None)
    tmp_.pop("socket_obj",None)
    tmp_.pop("packets_queue",None)
    tmp_.pop("logic_queue",None)
    tmp_.pop("conn_data",None)
    tmp_.pop("channels_list",None)
    tmp_.pop("update_counter",None)

def get_data_pack():
    # jdata = EasyDict()
    jdata = EasyDict()
    jdata.conn_data = EasyDict(orch_data.conn_data)
    jdata.channels_list = EasyDict()

    for key_, val1_ in orch_data.channels_list.items():
        # print(f"{val1_=}")
        # val_ = deepcopy(val1_)
        val_ = EasyDict()
        val_.update(val1_)
        pop_unwanted(val_)
        jdata.channels_list[val_.uuid] = val_


    for key_, val1_ in jdata.conn_data.channel_data.items():
        # val_ = EasyDict() ; val_.update(val1_)
        val_ = deepcopy(val1_)
        pop_unwanted(val_)
        # print(f"!!!!!!!!!!!!!!! {val_=}")
        jdata.channels_list[val_.uuid].update(val_)

    for key_, val1_ in jdata.channels_list.items():
        val_ = EasyDict() ; val_.update(val1_)
        # val_ = deepcopy(val1_)
        # print(f":::::::::::::::::::::::::::: {val_=}")
        pop_unwanted(val_)
        jdata.channels_list[val_.uuid].update(val_)

    return jdata


def get_str_uml(jdata):
    global orch_data
    device_id = f"{RemHardware.device_id()}"
    uml = ""

    # uml += f""
    # uml += f""
    # uml += f"skinparam actorStyle Hollow"

    juml = EasyDict()
    juml.packages=EasyDict()

    for key_, val_ in jdata.conn_data.channel_data.items():
        juml.packages.setdefault(val_.device_id,[])
        juml.packages[val_.device_id].append(val_.uuid)


    for key_, val_ in juml.packages.items():
        chans = ""

        if key_ == RemHardware.device_id():
            od_copy = EasyDict(orch_data)
            pop_unwanted(od_copy)
            od_str = json.dumps(od_copy,indent=4, sort_keys=True).replace("\n","\n    ")
            chans += f"\n    json {key_} {od_str} \n"

        for  val_ch_ in val_:
            chjson = jdata.channels_list[val_ch_]
            str_json = json.dumps(chjson,indent=4, sort_keys=True).replace("\n","\n    ")
            chans += f"\n    json {val_ch_} {str_json} \n"
        uml += f"\npackage {key_} {{\n {chans} }} \n"



    uml += f"\n"
    for src_, dests_ in jdata.conn_data.channel_links.items():
        for dest_ in dests_ :
            uml += f"{src_} --> {dest_} \n"

    # chans=""
    # for server_data in orch_data.server_data_list:
    #     # uml += f"({server_data.uuid}) \n"
    #     chans += f"    usecase {server_data.uuid} \n"
    # for client_data in orch_data.client_data_list:
    #     # uml += f"({client_data.uuid}) \n"
    #     chans += f"    usecase {client_data.uuid} \n"
    # uml += f"package {device_id} {{\n {chans} }} \n"


    # for server_data in orch_data.server_data_list:
    #     uml += f"{device_id} --> {server_data.uuid} \n"
    # for client_data in orch_data.client_data_list:
    #     uml += f"{device_id} --> {client_data.uuid} \n"


    # uml += f""
    # uml += f""

    uml = f"@startuml {device_id}\n\n{uml}\n\n@enduml"
    return uml

def update():
    if "channel_data" not in orch_data.conn_data:
        return

    jdata = get_data_pack()

    write_file_json(jdata)
    write_file_uml(jdata)

