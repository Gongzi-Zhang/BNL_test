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
from cali import *
from db_utilities import *

''' units '''
us = 1
ms = 1000*us
s = 1000*ms

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

    def __init__(self, run, fout, fmt='root'):
        self.run = run

        ''' variables '''
        self.listFile = f'{CALIROOT}/data/Run{run}_list.txt'
        if not os.path.isfile(self.listFile):
            logger.fatal(f'no list file found for run {run}')
            exit(4)

        logger.info(f'will process run {run}')

        self.outFormat = 'root'  # default root output
        if fmt:
            if fmt not in self.outFormats:
                logger.error(f'unknown output file format: {fmt}')
                logger.info(f'available data format: {outFormats}')
                exit(4)
            self.outFormat = fmt
        logger.info(f'output file format: {self.outFormat}')

        self.outFile = fout
        if '' == fout:
            self.outFile = f'{CALIROOT}/data/Run{run}'
        logger.info(f'output file: {self.outFile}')

        self.columns = []
        for ch in range(0, nChannels):
            for gain in ['LG', 'HG']:
                self.columns.append(f'Ch_{ch}_{gain}') 
        self.columns.append('TS')   # timestamps

        self.data = {}
        for col in self.columns:
            self.data[col] = []

        self.board = {}
        self.TS = {}
        for b in range(0, nBoards):
            self.board[b] = []
            self.TS[b] = []

        self.tdActual = {}
        self.startTime = 0
        self.nEvents = 0
        self.nGoods = 0
        self.nBads = 0
        self.split = 0

    #################################################
    def addBoard(self, board):
        if not board.nChannels: 
            return
        if board.nChannels != nBoardChannels:   
            ''' bad board record '''
            ''' do nothing, discard it'''
            logger.warning(f'bad board record in board {board.bId}, {board.nChannels}/{nBoardChannels} recorded; timestamps: {board.TS}')
            return

        bd = board.bId
        self.board[bd].append(deepcopy(board))
        self.TS[bd].append(board.TS) 

    #################################################
    def convert(self):
        # timeDiff = 20*ms*self.nBoards
        with open(self.listFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(6):
                next(fin)
            ''' get the start time '''
            line = next(fin)
            startTime = ' '.join(line.split()[4:-1])
            startTime = datetime.strptime(startTime, "%a %b %d %H:%M:%S %Y")
            self.startTime = startTime.timestamp()

            for l in range(2):
                next(fin)

            board = Board()
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
                    self.addBoard(board)
                    board.clear()
                    ''' split large files '''
                    if len(self.TS[0]) % 100000 == 0:
                        self.buildEvent()

                    ts = float(values[0]) 
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
                    logger.error(f'Invalid values in {values}')
                    continue

                ch += nBoardChannels*bd
                board.addChannel(ch, LG, HG)

            ''' last board record '''
            self.addBoard(board)
            self.buildEvent()
            if self.split:
                self.split += 1
            self.write()

    #################################################
    def getTimeDiff(self):
        tdCandidate = {
            1: [20*ms, 24*ms], 
            2: [40*ms, 44*ms]
        }
        ei = [0 for bd in range(0, nBoards)]
        while ei[0] < len(self.TS[0]):
            ts0 = self.TS[0][ei[0]]
            foundEvent = True
            for bd in range(1, nBoards):
                foundMatch = False
                while ei[bd] < len(self.TS[bd]):
                    ts = self.TS[bd][ei[bd]]
                    if ts > ts0:
                        break
                    diff = ts0 - ts
                    for td in tdCandidate[bd]:
                        if abs(diff - td) < td/20:
                            foundMatch = True
                            break
                    if foundMatch:
                        break
                    ei[bd] += 1

                if not foundMatch:
                    foundEvent = False
                    break

            if foundEvent:
                for bd in range(0, nBoards):
                    self.tdActual[bd] = self.TS[0][ei[0]] - self.TS[bd][ei[bd]] 
                    logger.info(f'trigger time difference between board 0 and {bd}: {self.tdActual[bd]}')
                break
            ei[0] += 1

    #################################################
    def buildEvent(self):
        if not all([len(self.TS[b]) for b in range(0, nBoards)]):
            return

        ''' find the first event and time difference between them '''
        if len(self.tdActual) != nBoards:
            self.getTimeDiff()
            if len(self.tdActual) != nBoards:
                logger.fatal('''Can't not find out the time difference between boards''')
                logger.info('here are the first 10 records')
                for i in range(0, 10):
                    row = {b: self.TS[b][i] for b in range(0, nBoards)}
                    logger.info(f'{row}')
                exit(4)

        ''' find all events '''
        for bd in range(1, nBoards):
            self.TS[bd] = [x + self.tdActual[bd] for x in self.TS[bd]]

        while all([len(self.TS[b]) for b in range(0, nBoards)]):
            ts0 = min([self.TS[b][0] for b in range(0, nBoards)])
            event = {}
            for bd in range(0, nBoards):
                ts = self.TS[bd][0]
                if ts - ts0 < 10*us:
                    event[bd] = ts
            self.nEvents += 1

            if len(event) == nBoards:
                self.nGoods += 1
                self.data['TS'].append(ts0/s + self.startTime)
                for bd in range(0, nBoards):
                    bdRecord = self.board[bd][0]
                    for ch in bdRecord.LG.keys():
                        self.data[f'Ch_{ch}_LG'].append(bdRecord.LG[ch]) 
                        self.data[f'Ch_{ch}_HG'].append(bdRecord.HG[ch]) 
                    self.TS[bd].pop(0)
                    self.board[bd].pop(0)
                if self.nGoods % 100000 == 0:
                    self.split += 1
                    self.write()
            else:
                self.nBads += 1
                row = {b: self.TS[b][0] for b in range(0, nBoards)}
                logger.warning(f'bad event {self.nBads}: {row}')
                for bd in event.keys():
                    self.TS[bd].pop(0)
                    self.board[bd].pop(0)

    #################################################
    def write(self):
        logger.info(f'find {self.nGoods}/{self.nEvents} good events')
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
            logger.error(f'unknown output file format: {self.outFormat}')
            exit(4)
        for col in self.columns:
            self.data[col] = []

def usage():
    print(sys.argv[0] + ' [-hof] dataFile')
    print('\t-h: print this help message')
    print('\t-o: output file name')
    print('\t-f: output file format: root, pkl [default root]')

if __name__ == '__main__':
    # read in command line arguments
    run = ''
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
        else:
            run = int(sys.argv[i])
        i+=1

    db = caliDB()
    runType = db.getRunType(run)
    if runType == 'ptrg':
        logger.warn(f'run {run} is a ptrg run, please use parse_ptrg.py to analyze it')
        exit(1)

    f = convert(run, outName, outFormat)
    f.convert()
