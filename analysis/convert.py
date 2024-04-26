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

''' units '''
us = 1
ms = 1000*us
s = 1000*ms


class convert:
    nBoards = 3
    nChannels = 64*nBoards
    inFile = ''
    outFile = ''
    outFormats = {'root', 'pkl'}
    outFormat = 'root'  # default root output
    nEvents = 0
    nGoods = 0
    split = 0   # split the raw file into smaller ones

    data = {}

    dataBuf = {}    # tmp value for one event
    columns = []

    def __init__(self, fin, fout, fmt='root'):
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
        for ch in range(0, self.nChannels):
            for gain in ['LG', 'HG']:
                self.columns.append(f'Ch_{ch}_{gain}') 
        self.columns.append('TS')   # timestamps
        for col in self.columns:
            self.data[col] = []

    def setNboards(self, n):
        self.nBoards = n
        self.nChannels = 64*n

    def fillEvent(self):
        for col in self.columns:
            self.data[col].append(self.dataBuf[col])

    def convert(self):
        timeDiff = 20*ms*self.nBoards
        with open(self.inFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(9):
                next(fin)

            nch = 0 # number of good channels in an event
            times = []
            boards = []

            event = 0
            ch = 0
            bd = 0
            LG = 0
            HG = 0

            for line in fin:
                line = line.strip()
                values = line.split()
                if 6 == len(values):
                    ts = float(values[0])
                    bd = int(values[2])
                    if bd in boards or (times and abs(ts - times[0]) > timeDiff):
                        self.nEvents += 1
                        ''' a new event '''
                        if self.nChannels == nch:
                            ''' good event '''
                            self.dataBuf['TS'] = sum(times) / len(times)    
                            self.dataBuf['TS'] /= s     # output TS unit: s
                            self.fillEvent()
                            self.nGoods += 1

                            if (self.nGoods % 500000) == 0:
                                self.split += 1
                                self.write()
                        else:
                            ''' bad event '''
                            ''' do nothing, discard it'''
                            print(f'WARNING--bad event in event {event}, {nch}/{self.nChannels} recorded')

                        nch = 0
                        times = []
                        boards = []


                    times.append(ts)    # input TS unit: us
                    boards.append(bd)
                    event = int(values[1])
                    ch = int(values[3])
                    LG = int(values[4])
                    HG = int(values[5])
                elif 4 == len(values):
                    bd = int(values[0])
                    ch = int(values[1])
                    LG = int(values[2])
                    HG = int(values[3])
                else:
                    print(f'ERROR:\tInvalide values in event {event}')
                    print(values)
                    continue
                nch += 1

                ch += 64*bd
                self.dataBuf[f'Ch_{ch}_LG'] = LG
                self.dataBuf[f'Ch_{ch}_HG'] = HG

            ''' the last event '''
            if self.nChannels == nch:
                self.fillEvent()
                self.nEvents += 1
                self.nGoods += 1

            if self.split:
                self.split += 1

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
    nBoards = 0
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
            i+=1
        else:
            inName = sys.argv[i]
        i+=1

    f = convert(inName, outName, outFormat)
    f.init()
    if nBoards:
        f.setNboards(nBoards)
    f.convert()
    f.write()
