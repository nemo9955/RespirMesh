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
    pass

def update_continous(delta):
    pass

def update_discrete(counter):
    pass

