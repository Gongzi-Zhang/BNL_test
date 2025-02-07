#!/usr/bin/env python
# coding: utf-8

# plot MIP over channels for a mip run

import sys
import json
import matplotlib.pyplot as plt
import mplhep as hep
import numpy as np

import cali
from utilities import *
from calidb import *

if len(sys.argv) < 2:
    logger.fatal("No run specified")
    exit(1)

run = sys.argv[1] 
db = caliDB()
if (db.getRunType(run) != "mip"):
    logger.error(f'run {run} is not a MIP run.')
    exit(2)

mipFile = cali.getFile(f'Run{run}_MIP.json')
if (not mipFile):
    logger.error("Can't find the mip json file for run: {run}")
    exit(3)

pedRun = db.getRunPedRun(run)
logger.info(f'run {run} uses ped run: {pedRun}')
pedFile = cali.getFile(f'Run{pedRun}_ped.json')
if (not pedFile):
    logger.error("Can't find the ped json file for ped run: {pedRun}")
    exit(3)

mip = []
mipOverPed = []
with open(mipFile, 'r') as fin:
    buf = json.load(fin)
    for ch in range(0, cali.nChannels):
        mip.append(buf['HG'][f'{ch}'])

with open(pedFile, 'r') as fin:
    buf = json.load(fin)
    for ch in range(0, cali.nChannels):
        mipOverPed.append(mip[ch]/buf['HG'][f'{ch}'][1])

plt.rcParams['figure.facecolor'] = 'white'
plt.rcParams['savefig.facecolor'] = 'white'
plt.rcParams['savefig.bbox'] = 'tight'
hep.style.use(hep.style.CMS)
hep.style.use("CMS")

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(36, 12))

configurations = {
    'Hex tile, 3mm SiPM': {'ranges': [(7, 34), (42, 55), (124, 127)], 'color': 'tab:red'},
    'Hex tile, 1.3mm SiPM': {'ranges': [(56,83), (91, 97)], 'color': 'tab:blue'},
    'Square tile, 3mm SiPM': {'ranges': [(112, 123),(128, 191)], 'color': 'tab:green'},
    'Hex tile, 3mm SiPM, Unpainted': {'ranges': [(0, 6), (35, 41), (84, 90), (98, 111)], 'color': 'y'}
}

x = np.array(range(0, cali.nChannels))
y1 = np.array(mip)
y2 = np.array(mipOverPed)
mask = y1 > 0
x = x[mask]
print(x)
y1 = y1[mask]
y2 = y2[mask]

labels_added = set()
for label, config in configurations.items():
    color = config['color']
    count = 0
    total = 0
    for start, end in config['ranges']:
        indices = np.where((x >= start) & (x <= end))
        count += np.count_nonzero(indices)
        total += np.sum(y1[indices])

        if label not in labels_added:
            ax2.scatter(x[indices], y2[indices], color=color, label=label)
            labels_added.add(label)
        else:
            ax2.scatter(x[indices], y2[indices], color=color)

        ax1.scatter(x[indices], y1[indices], color=color)

    ax1.axhline(y=total/count, color=color)

ax2.legend(fontsize=40, loc='upper right', prop={'family': 'monospace'})

# plt.suptitle(f"Run {run}")
ax1.set_xlabel(f"Ch")
ax1.set_ylabel("HG MIP [ADC]")
ax1.set_ylim(0, 5500)
ax2.set_xlabel("Ch")
ax2.set_ylabel("HG MIP/Ped RMS")
ax2.set_ylim(0, 35)
# plt.subplots_adjust(hspace=0)
plt.savefig(f'{cali.CALIROOT}/figures/{run}/MIP.pdf')

