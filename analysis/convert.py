#!/usr/bin/env python3
# coding: utf-8

'''
read and parse raw (ADC) data, convert to a pkl file

usage: ./convert.py input.txt [-o output.pkl] 
'''

import os
import sys
import pandas as pd
import uproot

class convert:
    nChannels = 64
    inFile = ''
    outFile = ''
    outFormats = {'root', 'pkl'}
    outFormat = 'root'  # default root output
    nEvent = 0
    nGood = 0

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
            if fmt not in outFormat:
                print(f'ERROR\tunknown output file format: {fmt}')
                print(f'INFO\tavailable data format: {outFormats}')
                exit(4)
            self.outFormat = fmt
        print(f'INFO\toutput file format: {self.outFormat}')

        self.outFile = fout
        if '' == fout:
            outName = fin.replace('.txt', f'.{self.outFormat}')
            outName = outName.replace('_list', '')
            self.outFile = outName
        print(f'INFO\toutput file: {self.outFile}')

    def init(self):
        ''' init output dataframe '''
        for ch in range(0, self.nChannels):
            for gain in ['LG', 'HG']:
                self.columns.append(f'Ch_{ch}_{gain}') 
        self.columns.append('TimeStamps') 
        for col in self.columns:
            self.data[col] = []

    def fillEvent(self):
        for col in self.columns:
            self.data[col].append(self.dataBuf[col])

    def convert(self):
        with open(self.inFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(9):
                next(fin)

            nch = 0 # number of good channels in an event

            time = 0
            event = 0
            ch = 0
            bd = 0
            LG = 0
            HG = 0

            for line in fin:
                line = line.strip()
                values = line.split()
                if 6 == len(values):
                    self.nEvent += 1
                    ''' process last event '''
                    if 64 == nch:
                        self.fillEvent()
                        self.nGood += 1

                    nch = 0

                    self.dataBuf['TimeStamps'] = (float(values[0]))
                    event = int(values[1])
                    bd = int(values[2])
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
            if 64 == nch:
                self.fillEvent()
                self.nGood += 1

    def write(self):
        print(f'INFO:\t{self.nGood}/{self.nEvent} good events processed')
        if 'root' == self.outFormat:
            with uproot.recreate(self.outFile) as fout:                            
                fout['events'] = self.data
        elif 'pkl' == self.outFormat:
            df.to_pickle(self.outFile)
        else:
            print(f'ERROR\tunknown output file format: {self.outFormat}')
            exit(4)

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
        else:
            inName = sys.argv[i]
        i+=1

    f = convert(inName, outName, outFormat)
    f.init()
    f.convert()
    f.write()
