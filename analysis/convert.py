#!/usr/bin/env python3
# coding: utf-8

'''
read and parse raw (ADC) data, convert to a root/pkl file

select good (complete) events, don't do any cut on events here

usage: ./convert.py input.txt [-o output.pkl] 
'''

import os
import sys
import pandas as pd
import uproot
from copy import deepcopy
from datetime import datetime

''' units '''
us = 1
ms = 1000*us
s = 1000*ms

nBoards = 3
nBoardChannels = 64
nChannels = nBoards*nBoardChannels

class Board:
    def __init__(self):
        self.bId = -1
        self.TS = 0
        self.nChannels = 0
        self.LG = {}
        self.HG = {}

    def __eq__(self, b):
        self.bId = b.bId
        self.TS = b.TS
        self.nChannels = b.nChannels
        self.LG = b.LG
        self.HG = b.HG

    def addChannel(self, ch, l, h):
        self.LG[ch] = l
        self.HG[ch] = h
        self.nChannels += 1

    def clear(self):
        self.bId = -1
        self.TS = 0
        self.nChannels = 0
        self.LG.clear()
        self.HG.clear()

class convert:
    ''' shared by all instance '''
    outFormats = {'root', 'pkl'}
    columns = []

    def __init__(self, fin, fout, fmt='root'):
        ''' variables '''
        self.inFile = ''
        self.outFile = ''
        self.outFormat = 'root'  # default root output
        self.nEvents = 0
        self.nGoods = 0
        self.split = 0   # split the raw file into smaller ones

        self.event = {}
        self.data = {}

        if '' == fin:
            print(f'FATAL\tno data file specified')
            exit(2)

        if not os.path.isfile(fin):
            print(f'FATAL\tfile doesn\'t exist: {fin}')
            exit(4)

        self.inFile = fin
        print(f'INFO\twill process {fin}')

        if fmt:
            if fmt not in self.outFormats:
                print(f'ERROR\tunknown output file format: {fmt}')
                print(f'INFO\tavailable data format: {outFormats}')
                exit(4)
            self.outFormat = fmt
        print(f'INFO\toutput file format: {self.outFormat}')

        self.outFile = fout
        if '' == fout:
            # outName = fin.replace('.txt', f'.{self.outFormat}')
            self.outFile = fin.replace('_list.txt', '')
        print(f'INFO\toutput file: {self.outFile}')

    def init(self):
        ''' init output dataframe '''
        for ch in range(0, nChannels):
            for gain in ['LG', 'HG']:
                self.columns.append(f'Ch_{ch}_{gain}') 
        self.columns.append('TS')   # timestamps
        for col in self.columns:
            self.data[col] = []

    def addBoard(self, trgId, board):
        if board.nChannels: 
            if board.nChannels != nBoardChannels:   
                ''' bad board record '''
                ''' do nothing, discard it'''
                print(f'WARNING--bad board record in event {trgId} board {board.bId}, {board.nChannels}/{nBoardChannels} recorded')
            else: # a good board record
                if trgId not in self.event:
                    self.event[trgId] = []
                    self.nEvents += 1

                self.event[trgId].append(deepcopy(board))

                if len(self.event[trgId]) == nBoards:
                    self.nGoods += 1

    def fillEvent(self):
        for trgId in self.event:
            if len(self.event[trgId]) == nBoards:
                ts = []
                for bd in self.event[trgId]:
                    ts.append(bd.TS)
                    for ch in bd.LG.keys():
                        self.data[f'Ch_{ch}_LG'].append(bd.LG[ch]) 
                        self.data[f'Ch_{ch}_HG'].append(bd.HG[ch]) 
                self.data['TS'].append(sum(ts)/len(ts))

        self.event = {}

    def convert(self):
        # timeDiff = 20*ms*self.nBoards
        with open(self.inFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(6):
                next(fin)
            ''' get the start time '''
            line = next(fin)
            startTime = ' '.join(line.split()[4:-1])
            startTime = datetime.strptime(startTime, "%a %b %d %H:%M:%S %Y")
            startTime = startTime.timestamp()

            for l in range(2):
                next(fin)

            board = Board()
            trgId = 0
            ts = 0
            bd = 0
            ch = 0
            LG = 0
            HG = 0
            bdRecords = 0

            for line in fin:
                line = line.strip()
                values = line.split()
                if 6 == len(values):
                    self.addBoard(trgId, board)
                    board.clear()
                    if self.nGoods and (self.nGoods % 500000) == 0:
                        self.split += 1
                        self.fillEvent()
                        self.write()


                    ts = float(values[0]) / us + startTime  # convert to s
                    trgId = int(values[1])
                    bd = int(values[2])
                    ch = int(values[3])
                    LG = int(values[4])
                    HG = int(values[5])
                    board.bId = bd
                    board.TS = ts
                elif 4 == len(values):
                    bd = int(values[0])
                    ch = int(values[1])
                    LG = int(values[2])
                    HG = int(values[3])
                else:
                    print(f'ERROR:\tInvalide values in event {trgId} board {bd} ch {ch}')
                    print(values)
                    continue

                ch += nBoardChannels*bd
                board.addChannel(ch, LG, HG)

            ''' last board record '''
            self.addBoard(trgId, board)
            if self.split:
                self.split += 1
            self.fillEvent()
            self.write()

    def write(self):
        print(f'INFO\t{self.nGoods}/{self.nEvents} good events processed')
        fname = self.outFile
        if self.split:
            fname = f'{fname}_{self.split}'
        if 'root' == self.outFormat:
            with uproot.recreate(f'{fname}.root') as fout:                            
                fout['events'] = self.data
        elif 'pkl' == self.outFormat:
            df = pd.DataFrame(self.data)
            df.to_pickle(f'{fname}.pkl')
        else:
            print(f'ERROR\tunknown output file format: {self.outFormat}')
            exit(4)
        for col in self.columns:
            self.data[col] = []

def usage():
    print(sys.argv[0] + ' [-hofb] dataFile')
    print('\t-h: print this help message')
    print('\t-o: output file name')
    print('\t-f: output file format: root, pkl [default root]')
    print('\t-b: number of boards [default 1]')

if __name__ == '__main__':
    # read in command line arguments
    inName = ''
    outName = ''
    outFormat = ''
    i=1
    while i<len(sys.argv):
        if '-h' == sys.argv[i]:
            usage()
            exit(0)
        elif '-o' == sys.argv[i]:
            outName = sys.argv[i+1]
            i+=1
        elif '-f' == sys.argv[i]:
            outFormat = sys.argv[i+1]
            i+=1
        elif '-b' == sys.argv[i]:
            nBoards = int(sys.argv[i+1])
            nChannels = nBoards * nBoardChannels
            i+=1
        else:
            inName = sys.argv[i]
        i+=1

    f = convert(inName, outName, outFormat)
    f.init()
    f.convert()
