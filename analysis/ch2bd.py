#!/usr/bin/env python3

import sys

maxCh = 192

if len(sys.argv) < 2:
    print("FATAL\tno channel number provided")
    print(f'usage\t{sys.argv[0]} #Ch')
    exit(4)

ch = int(sys.argv[1])
bd = 0
if ch < 0 or ch > maxCh:
    print(f"ERROR\tinvalid channel number. Allowed range: [0-{maxCh}]")
    exit(2)

nHexLayers = 4
nHexBoards = nHexLayers * 4
nHexChannels = nHexBoards*7
if ch < nHexChannels:
    bd = ch/7
    ch = ch%7
else:
    ch -= nHexChannels
    bd = nHexBoards + ch/4
    ch = ch%4

print(f"layer: {int(bd/4)}")
print(f"bd: {int(bd)}")
print(f"ch: {int(ch)}")
