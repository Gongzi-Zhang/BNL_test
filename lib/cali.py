'''
global settings
'''

import os
from utilities import *

if 'CALIROOT' not in os.environ:
    logger.fatal('CALIROOT not set. Please source setup.sh')
    exit(4)

CALIROOT=os.environ['CALIROOT']
backupDir = '/media/arratialab/CALI/BNL_test/'

# constants
'''
CAEN:       CAEN unit count: 0-2
channel:    channel count: 0-192
layer:      sampling layers: 0-12
board:	    PCB count: 0-50
quadrant:   quadrant count in a layer: 0-3
sipm:       SiPM count in a PCB: 0-6
'''
run = -1
nCAENs = 3
nCAENChannels = 64
nChannels = nCAENs*nCAENChannels
layerMax = 20
nHexLayers = 4
nSqaLayers = 9
nLayerBoards = 4
nHexBoards = nHexLayers*nLayerBoards
nSqaBoards = nSqaLayers*nLayerBoards
nHexBoardChannels = 7
nSqaBoardChannels = 4
nHexChannels = nHexBoards*nHexBoardChannels
nSqaChannels = nSqaBoards*nSqaBoardChannels

nEightLayerBoards = 8*nLayerBoards
nNineLayerBoards  = 9*nLayerBoards
channelMax = 300

# units
mm = 1
cm = 10*mm

gapX = 0*mm
gapY = 2.54*mm	# 0.1 in
pcbX = 131.92*mm	# 177.64 - 45.72
pcbY = 97.99*mm	# 130.91 - 32.92
layerZ = 27.1526*mm

gains = ["LG", "HG"]

def setRun(r):
    run = r
    global nCAENs
    global nChannels
    global nHexLayers
    global nHexBoards
    global nHexChannels
    global nSqaLayers
    global nSqaBoards
    global nSqaChannels
    if run < 3:
	    nCAENs = 1
	    nChannels = 56
	    nHexLayers = 2
	    nHexBoards = 8
	    nHexChannels = 56
	    nSqaLayers = 0
	    nSqaBoards = 0
	    nSqaChannels = 0
    elif run < 4:
	    nCAENs = 3
	    nChannels = 176
	    nHexLayers = 4
	    nHexBoards = 16
	    nHexChannels = 112
	    nSqaLayers = 4
	    nSqaBoards = 16
	    nSqaChannels = 64
    elif (run > 5000):
        nCAENs = 4
        nChannels = nCAENs*nCAENChannels

class SiPM:
    layer = -1
    quadrant = -1
    sipm = -1
    def __init__(self, layer, quadrant, sipm):
        self.layer = layer
        self.quadrant = quadrant
        self.sipm = sipm

class sipmXY:
    x = 0
    y = 0

    def __init__(self, x, y):
        self.x = x
        self.y = y
    
    def __is__(self, a):
        return self.x == a.x and self.y == a.y
    def __eq__(self, a):
        return self.x == a.x and self.y == a.y
    def __neg__(self):
        return sipmXY(-self.x, -self.y)
    def __add__(self, a):
        return sipmXY(self.x + a.x, self.y + a.y)
    def __sub__(self, a):
        return sipmXY(self.x - a.x, self.y - a.y)

boardLabel = [
# top right, top left, bottom left, bottom right
    [ 1,  2,  3,  4],	# hexagon
    [ 5,  6,  7,  8],	# hexagon
    [10,  9, 11, 12],	# hexagon
    [26, 13, 25, 28],	# hexagon
    [30, 41, 14, 37],	# transition: top 2 square, bottom 2 hexagon
    [38, 54, 21, 18],	# square
    [44, 22, 58, 47],	# square
    [50, 57, 33, 31],	# square
    [ 0, 15, 46, 35],	# square
    [ 0, 20, 45, 36],   # square
    [42, 49, 23, 16],   # square
    [17, 52, 19, 51],   # square
    [55, 43, 32, 39],   # square
]

pcbAnchor = [
    sipmXY(-gapX/2, gapY/2 + pcbY),
    sipmXY( gapX/2, gapY/2),
    sipmXY( gapX/2, -(gapY/2 + pcbY)),
    sipmXY(-gapX/2, -gapY/2),
]
                                        
hexBoardSipmsipmXY = [
    sipmXY(50.01*mm, 80.90*mm),
    sipmXY(77.64*mm, 64.95*mm),
    sipmXY(22.37*mm, 64.95*mm),
    sipmXY(50.01*mm, 48.99*mm),
    sipmXY(77.64*mm, 33.04*mm),
    sipmXY(22.37*mm, 33.04*mm),
    sipmXY(50.01*mm, 17.08*mm),
]
sqaBoardSipmsipmXY = [
    sipmXY(73.9*mm, 72.89*mm),
    sipmXY(26.1*mm, 72.89*mm),
    sipmXY(73.9*mm, 25.09*mm),
    sipmXY(26.1*mm, 25.09*mm),
]
otherHexBoardSipmsipmXY = [
    sipmXY(50.01*mm, 80.90*mm),
    sipmXY(22.37*mm, 64.95*mm),
    sipmXY(77.64*mm, 64.95*mm),
    sipmXY(50.01*mm, 48.99*mm),
    sipmXY(77.64*mm, 33.04*mm),
    sipmXY(22.37*mm, 33.04*mm),
    sipmXY(50.01*mm, 17.08*mm),
]
otherSqaBoardSipmsipmXY = [
    sipmXY(26.1*mm, 72.89*mm),
    sipmXY(73.9*mm, 72.89*mm),
    sipmXY(26.1*mm, 25.09*mm),
    sipmXY(73.9*mm, 25.09*mm),
]

def getSipm(ch):
    if (ch < 0 or ch >= nChannels):
        return False

    restCh = 0
    board = -1
    sipm = -1
    if ch < nHexChannels:
        board = ch // nHexBoardChannels
        sipm = ch % nHexBoardChannels
    else:
        restCh = ch - nHexChannels
        board = nHexBoards + restCh // nSqaBoardChannels
        sipm = restCh % nSqaBoardChannels

        if (board >= nEightLayerBoards):
            board += 1;
        if (board >= nNineLayerBoards):
            board += 1;

        # special cases
        if (run < 265):
            if (board == 37):
                board = 40;

    layer = board // nLayerBoards
    quadrant = board % nLayerBoards

    return SiPM(layer, quadrant, sipm)

def getSipmXY(ch):
    sp = getSipm(ch)
    if not sp:
        return False

    bl = boardLabel[sp.layer][sp.quadrant]
    if 25 == bl:   # left side hexagonal
        pos = otherHexBoardSipmsipmXY[sp.sipm]
    elif bl in (28, 37):	# right side hexagonal, flip the index
        pos = otherHexBoardSipmsipmXY[nHexBoardChannels - 1 - sp.sipm]
    elif bl in (41, 21, 15, 46, 20, 45, 49, 23, 52, 19, 43, 32, # left side square
            30, 44, 47, 31, 36, 42, 39):    # right side square
        pos = otherSqaBoardSipmsipmXY[sp.sipm]
    elif bl in (18, 38, 35, 16, 17):	# right side square, flip the index
        pos = otherSqaBoardSipmsipmXY[nSqaBoardChannels - 1 - sp.sipm]
    elif sp.layer < nHexLayers: # general hex tile
        pos = hexBoardSipmsipmXY[sp.sipm]
    else: 
        pos = sqaBoardSipmsipmXY[sp.sipm] # general square tile

    if sp.quadrant in (0,3):
        pos = -pos

    pos = pos + pcbAnchor[sp.quadrant]
    return pos

def getSipmXYZ(ch):
    pos = getSipmXY(ch)
    if not pos:
        return False

    sp = getSipm(ch)
    z = 24.0526*mm + sp.layer*27.1526*mm
    return pos.x, pos.y, z

def printSipmXYZ():
    for ch in range(0, nChannels):
        pos = getSipmXY(ch)
        print(f'{ch:3d}\t{pos.x:.2f}\t{pos.y:.2f}')
