#!/usr/bin/env python3
# coding: utf-8

'''
extract the TS info for analysis

usage: ./extract.py input.txt 
'''

import os
import sys
from cali import *

''' units '''
us = 1
ms = 1000*us
s = 1000*ms

class extract:
    nEvents = 0
    nGoods = 0

    def __init__(self, fin):
        if '' == fin:
            print(f'FATAL\tno data file specified')
            exit(2)

        if not os.path.isfile(fin):
            print(f'FATAL\tfile doesn\'t exist: {fin}')
            exit(4)

        self.inFile = fin

    def extract(self):
        with open(self.inFile, 'r') as fin:
            ''' skip the first 9 lines '''
            for l in range(9):
                next(fin)

            trgId = 0
            ch = 0
            bd = 0

            for line in fin:
                values = line.split()
                if 6 != len(values):
                    continue
                ts = float(values[0])
                trgId = int(values[1])
                bd = int(values[2])
                print(f'{ts}\t{trgId}\t{bd}')

def usage():
    print(sys.argv[0] + ' [-h] dataFile')
    print('\t-h: print this help message')

if __name__ == '__main__':
    # read in command line arguments
    inName = ''
    i=1
    while i<len(sys.argv):
        if '-h' == sys.argv[i]:
            usage()
            exit(0)
        else:
            inName = sys.argv[i]
        i+=1

    f = extract(inName)
    f.extract()
