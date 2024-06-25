'''
global settings for calorimeter
'''
from utilities import *

nCAENs = 0
nChannels = 0
nCAENChannels = []
preChannels = []

def setnCAENChannels(ch):
    global nCAENs
    global nChannels
    global nCAENChannels
    global preChannels

    nCAENs = len(ch)
    if not nCAENs:
        logger.error("empty content in setting number of channels per CAEN")
        return

    nChannels = ch[0]
    nCAENChannels = []
    nCAENChannels.append(ch[0])
    preChannels = []
    preChannels.append(0)
    for i in range(1, nCAENs):
        nCAENChannels.append(ch[i])
        preChannels.append(preChannels[i-1] + ch[i])
        nChannels += ch[i]
