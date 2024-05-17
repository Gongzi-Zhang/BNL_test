'''
global settings
'''

import os
from utilities import *

if 'CALIROOT' not in os.environ:
    logger.fatal('CALIROOT not set. Please source setup.sh')
    exit(4)

CALIROOT=os.environ['CALIROOT']

# constants
'''
channel:    global channel count: 0-192
quadrant:   global quadrant count: 0-44
layer:      global layer count: 0-11

quadrant:   local board count: 0-3
sipm:       local board SiPM count: 0-6
'''
run = -1
nCAENs = 3
nCAENChannels = 64
nChannels = nCAENs*nCAENChannels

gains = ["LG", "HG"]

nLayers = 11
nLayerBoards = 4
nHexBoardChannels = 7
nSqaBoardChannels = 4
nHexLayers = 4
nHexBoards = 4*nHexLayers
nHexChannels = 7*nHexBoards
nEightLayerChannels = 4*nLayerBoards*nHexBoardChannels + 4*nLayerBoards*nSqaBoardChannels
nNineLayerChannels = nEightLayerChannels + 3*4
nTenLayerChannels = nEightLayerChannels + 2*3*4
maxChannel = 192 - 1

# units
mm = 1
cm = 10*mm

gapX = 0*mm
gapY = 2.54*mm	# 0.1 in
pcbX = 131.92*mm	# 177.64 - 45.72
pcbY = 97.99*mm	# 130.91 - 32.92
layerZ = 27.1526*mm

def setRun(r):
    run = r
    if (run > 5000):
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

class Pos:
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
        return Pos(-self.x, -self.y)
    def __add__(self, a):
        return Pos(self.x + a.x, self.y + a.y)
    def __sub__(self, a):
        return Pos(self.x - a.x, self.y - a.y)

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
    Pos(-gapX/2, gapY/2 + pcbY),
    Pos( gapX/2, gapY/2),
    Pos( gapX/2, -(gapY/2 + pcbY)),
    Pos(-gapX/2, -gapY/2),
]
                                        
hexBoardSipmPos = [
    Pos(50.01*mm, 80.90*mm),
    Pos(77.64*mm, 64.95*mm),
    Pos(22.37*mm, 64.95*mm),
    Pos(50.01*mm, 48.99*mm),
    Pos(77.64*mm, 33.04*mm),
    Pos(22.37*mm, 33.04*mm),
    Pos(50.01*mm, 17.08*mm),
]
sqaBoardSipmPos = [
    Pos(73.9*mm, 72.89*mm),
    Pos(26.1*mm, 72.89*mm),
    Pos(73.9*mm, 25.09*mm),
    Pos(26.1*mm, 25.09*mm),
]
otherHexBoardSipmPos = [
    Pos(50.01*mm, 80.90*mm),
    Pos(22.37*mm, 64.95*mm),
    Pos(77.64*mm, 64.95*mm),
    Pos(50.01*mm, 48.99*mm),
    Pos(77.64*mm, 33.04*mm),
    Pos(22.37*mm, 33.04*mm),
    Pos(50.01*mm, 17.08*mm),
]
otherSqaBoardSipmPos = [
    Pos(26.1*mm, 72.89*mm),
    Pos(73.9*mm, 72.89*mm),
    Pos(26.1*mm, 25.09*mm),
    Pos(73.9*mm, 25.09*mm),
]

def getSipm(ch):
    if (ch < 0 or ch > maxChannel):
        return False

    restCh = 0
    board = -1
    sipm = -1
    if ch < nHexChannels:
        board = ch // nHexBoardChannels
        sipm = ch % nHexBoardChannels
    elif ch < nEightLayerChannels:
        restCh = ch - nHexChannels
        board = nHexBoards + restCh // nSqaBoardChannels
        sipm = restCh % nSqaBoardChannels
    elif ch < nNineLayerChannels:
        restCh = ch - nEightLayerChannels
        board = 8*nLayerBoards + 1 + restCh // nSqaBoardChannels
        sipm = restCh % nSqaBoardChannels
    else:
        # no 10th layer right now
        restCh = ch - nNineLayerChannels
        board = 10*nLayerBoards + restCh // nSqaBoardChannels
        sipm = restCh % nSqaBoardChannels

    layer = board // nLayerBoards
    quadrant = board % nLayerBoards

    return SiPM(layer, quadrant, sipm)

def getSipmXY(ch):
    sp = getSipm(ch)
    if not sp:
        return False

    bl = boardLabel[sp.layer][sp.quadrant]
    if 25 == bl:   # left side hexagonal
        pos = otherHexBoardSipmPos[sp.sipm]
    elif bl in (28, 37):	# right side hexagonal, flip the index
        pos = otherHexBoardSipmPos[nHexBoardChannels - 1 - sp.sipm]
    elif bl in (15, 21, 46):	# left side square
        pos = otherSqaBoardSipmPos[sp.sipm]
    elif bl in (18, 35, 38):	# right side square, flip the index
        pos = otherSqaBoardSipmPos[nSqaBoardChannels - 1 - sp.sipm]
    elif sp.layer < nHexLayers: # general hex tile
        pos = hexBoardSipmPos[sp.sipm]
    else: 
        # general square tile
        pos = sqaBoardSipmPos[sp.sipm]

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

def printSipmPos():
    for ch in range(0, maxChannel+1):
        pos = getSipmPos(ch)
        print(f'{ch:3d}\t{pos.x:.2f}\t{pos.y:.2f}')
