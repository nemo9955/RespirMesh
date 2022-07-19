#!/usr/bin/env python3

import time
from random import random

DEVICE_ID=None

def set_device_id(dev_id_):
    global DEVICE_ID
    DEVICE_ID = dev_id_

def device_id():
    global DEVICE_ID
    return DEVICE_ID


def time_milis():
    return round(time.time() * 1000)

def sleep_milis(duration):
    time.sleep(duration / 1000)

def rand_float():
    return random()


