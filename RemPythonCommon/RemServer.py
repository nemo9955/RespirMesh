#!/usr/bin/env python3


RemOrchestrator = None
RemRouter = None
log = None
RemConnectionController = None
RemHardware = None
RemChannel = None
RemHeaderTypes = None
RemDebugger = None
utils = None
EasyDict = None

def set_orchestrator(remOrchestrator_):

    global RemOrchestrator
    global RemRouter
    global log
    global RemConnectionController
    global RemHardware
    global RemChannel
    global RemHeaderTypes
    global RemDebugger
    global utils
    global EasyDict

    RemOrchestrator = remOrchestrator_.RemOrchestrator
    RemRouter = remOrchestrator_.RemRouter
    log = remOrchestrator_.log
    RemConnectionController = remOrchestrator_.RemConnectionController
    RemHardware = remOrchestrator_.RemHardware
    RemChannel = remOrchestrator_.RemChannel
    RemHeaderTypes = remOrchestrator_.RemHeaderTypes
    RemDebugger = remOrchestrator_.RemDebugger
    utils = remOrchestrator_.utils
    EasyDict = remOrchestrator_.utils.EasyDict


def begin():
    pass


def update_continous(delta):
    pass
def update_discrete(counter):
    pass

