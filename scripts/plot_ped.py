#!/usr/bin/env python
# coding: utf-8

import sys
import json
import matplotlib.pyplot as plt
from matplotlib.pyplot import cm
import mplhep as hep
import numpy as np

import cali
from utilities import *
from calidb import *

if len(sys.argv) < 3:
    logger.fatal("No run specified")
    exit(1)

run1 = sys.argv[1] 
run2 = sys.argv[2] 
ped = {'mean': {}, 'rms': {}}

db = caliDB()
date = {}
for run in [run1, run2]:
    if (db.getRunType(run) != "ptrg"):
        logger.error(f'run {run} is not a MIP run.')
        exit(2)

    file = cali.getFile(f'Run{run}_ped.json')
    if (not file):
        logger.error("Can't find the ped json file for run: {run}")
        exit(3)

    date[run] = db.getRunStartTime(run).split()[0]

    ped['mean'][run] = []
    ped['rms'][run] = []

    with open(file, 'r') as fin:
        buf = json.load(fin)
        for ch in range(0, cali.nChannels):
            ped['mean'][run].append(buf['HG'][f'{ch}'][0])
            ped['rms'][run].append(buf['HG'][f'{ch}'][1])

colors = cm.rainbow(np.linspace(0, 1, 2))
plt.rcParams['figure.facecolor'] = 'white'
plt.rcParams['savefig.facecolor'] = 'white'
plt.rcParams['savefig.bbox'] = 'tight'
hep.style.use(hep.style.CMS)
hep.style.use("CMS")

plt.figure( figsize=(20, 10) )

x = np.array(range(0, cali.nChannels))

# ped mean plot
y1 = np.array(ped['mean'][run1])
y1err = np.array(ped['rms'][run1])
y2 = np.array(ped['mean'][run2])
y2err = np.array(ped['rms'][run2])

plt.errorbar(x, y1, yerr = y1err, fmt='o', linestyle='none', color=colors[0], label=f'Run   572 ({date[run1]})')
plt.errorbar(x, y2, yerr = y2err, fmt='o', linestyle='none', color=colors[1], label=f'Run 2578 ({date[run2]})')

plt.ylim(0, 780)
# plt.legend(fontsize=40, loc='upper left', prop={'family': 'monospace'})
plt.legend(fontsize=27, loc='upper left')

# plt.title(f"HG pedestal mean", fontsize=50)
plt.ylabel("HG pedestal mean [ADC]", fontsize=30)
plt.xlabel("Ch", fontsize=30)
plt.savefig(f'run{run1}_vs_{run2}_ped_mean.pdf')


# ped rms plot
plt.cla()

y1 = np.array(ped['rms'][run1])
y2 = np.array(ped['rms'][run2])

plt.scatter(x, y1, color=colors[0], label=f'Run   572 ({date[run1]})')
plt.scatter(x, y2, color=colors[1], label=f'Run 2578 ({date[run2]})')

plt.ylim(0, 280)
# plt.legend(fontsize=40, loc='upper left', prop={'family': 'monospace'})
plt.legend(fontsize=27, loc='upper left')


# plt.title(f"HG pedestal RMS", fontsize=50)
plt.ylabel("HG pedestal RMS [ADC]", fontsize=30)
plt.xlabel("Ch", fontsize=30)
plt.savefig(f'run{run1}_vs_{run2}_ped_RMS.pdf')

