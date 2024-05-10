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
import json
import ROOT

''' units '''
us = 1
ms = 1000*us
s = 1000*ms

nBoards = 3
nChannels = 64*nBoards

class convert:
    def __init__(self, fin, fout):
        if '' == fin:
            print(f'FATAL\tno data file specified')
            exit(2)

        if not os.path.isfile(fin):
            print(f'FATAL\tfile doesn\'t exist: {fin}')
            exit(4)

        self.inFile = fin
        print(f'INFO\twill process {fin}')

        self.outFile = fout
        if '' == fout:
            self.outFile = fin.replace('_list.txt', '_hist.root')
        print(f'INFO\toutput file: {self.outFile}')

        self.pedFile = fin.replace('_list.txt', '_ped.json')

        self.h1 = {}
        self.func = {}
        xmax = {'LG': 500, 'HG': 1000}
        for gain in ['LG', 'HG']:
            self.func[gain] = ROOT.TF1(gain, "gaus", 0, xmax[gain])
            for ch in range(0, nChannels):
                hname = f'Ch_{ch}_{gain}'
                self.h1[hname] = ROOT.TH1F(hname, hname, 250, 0, xmax[gain])

        ROOT.gROOT.SetBatch(1)

    def convert(self):
        with open(self.inFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(9):
                next(fin)

            ch = 0
            bd = 0
            LG = 0
            HG = 0

            for line in fin:
                line = line.strip()
                values = line.split()
                if 6 == len(values):
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

                ch += 64*bd
                self.h1[f'Ch_{ch}_LG'].Fill(LG) 
                self.h1[f'Ch_{ch}_HG'].Fill(HG) 

    def fit(self):
        ped = {}
        for gain in ['LG', 'HG']:
            ped[gain] = {}
            f = self.func[gain]
            for ch in range(0, nChannels):
                name = f'Ch_{ch}_{gain}'
                h = self.h1[name]
                f.SetParameters(h.GetMaximum(), h.GetMean(), h.GetRMS())
                h.Fit(f)
                ped[gain][ch] = [f.GetParameter(1), f.GetParameter(2)]
        with open(self.pedFile, 'w') as f:
            f.write(json.dumps(ped))

    def write(self):
        fout = ROOT.TFile.Open(self.outFile, "recreate")
        fout.cd()
        for h in self.h1.keys():
            self.h1[h].Write()
        fout.Close()

def usage():
    print(sys.argv[0] + ' [-hofb] dataFile')
    print('\t-h: print this help message')
    print('\t-o: output file name')
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
        elif '-b' == sys.argv[i]:
            nBoards = int(sys.argv[i+1])
            nChannels = 64*nBoards
            i+=1
        else:
            inName = sys.argv[i]
        i+=1

    f = convert(inName, outName)
    f.convert()
    f.fit()
    f.write()
