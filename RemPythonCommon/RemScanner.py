#!/usr/bin/env python3



RemOrchestrator = None
RemRouter = None
log = None
RemHardware = None
RemScanner = None
RemHeaderTypes = None
RemDebugger = None
utils = None
EasyDict = None

def set_orchestrator(set_value_):
    global RemOrchestrator
    global RemRouter
    global log
    global RemHardware
    global RemScanner
    global RemHeaderTypes
    global RemDebugger
    global utils
    global EasyDict

    RemOrchestrator = set_value_.RemOrchestrator
    RemRouter = set_value_.RemRouter
    log = set_value_.log
    RemHardware = set_value_.RemHardware
    RemScanner = set_value_.RemScanner
    RemHeaderTypes = set_value_.RemHeaderTypes
    RemDebugger = set_value_.RemDebugger
    utils = set_value_.utils
    EasyDict = set_value_.utils.EasyDict

def begin():
    for scanner_uuid, scanner_data in RemOrchestrator.orch_data.scanners_list.items():
        scanner_data.scanner_logic.begin(scanner_data)

def update_continous(delta):
    for scanner_uuid, scanner_data in RemOrchestrator.orch_data.scanners_list.items():
        scanner_data.scanner_logic.update_continous(scanner_data, delta)

def update_discrete(counter):
    for scanner_uuid, scanner_data in RemOrchestrator.orch_data.scanners_list.items():
        scanner_data.scanner_logic.update_discrete(scanner_data, counter)




