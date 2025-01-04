#!/usr/bin/env python3

# ped RMS vs dark current

import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import scipy
import mplhep as hep
from datetime import datetime, timezone, timedelta
from matplotlib.pyplot import cm
import json

import cali
from utilities import *
from calidb import *

db = caliDB()

channels = [25, 62, 100, 160]

breakV = 38.74096661764706
CALIROOT = os.getenv("CALIROOT")
directory = os.path.expanduser(f'{CALIROOT}/work/labview/')
# directory = os.path.expanduser('~/labview/data')
current_list = []
date_list = []
OV = [-1.5, -1.0, -0.5, 0.5, 1.0, 2.0, 2.5, 3.0, 4.0]
for i in range(9):
    current_list.append([])
dates = set()

for file in sorted(os.listdir(directory)):
    file = f'{directory}/{file}'
    if os.stat(file).st_size == 0:
        continue
    with open(file) as f:
        lines = f.read().split(',')
    voltage = np.array(lines[::2], dtype=float)
    current = 1e6*np.array(lines[1::2], dtype=float)

    for i in range(9):
        current_list[i].append(current[np.argmin(abs(voltage-breakV-OV[i]))])

    timestamp = file.split('UTC_')[1].split('.txt')[0]
    yyyymmdd = list(timestamp.split('__')[0])
    month = yyyymmdd[4]+yyyymmdd[5]
    day = yyyymmdd[6]+yyyymmdd[7]
    hour = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[0]
    minute = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[1]   
    # date_list.append(datetime(2024, int(month), int(day), int(hour), int(minute), tzinfo=timezone.utc)) 
    date_list.append(datetime(2024, int(month), int(day), 0, 0, tzinfo=timezone.utc)) 
    dates.add(f'2024-{month}-{day}')

dark_current = []
ped = {'mean': {}, 'RMS': {}}
for ch in channels:
    ped['mean'][ch] = []
    ped['RMS'][ch] = []

for date in sorted(dates):
    ped_runs = db.query(f'Type = "ptrg" AND Flag = "good" AND DATE(StartTime) = "{date}"')['Run']    # use the first pedestal run on that day
    if not ped_runs:
        continue

    ped_run = ped_runs[0]
    if (ped_run == 1291):
        continue

    pedFile = cali.getFile(f'Run{ped_run}_ped.json')
    with open(pedFile, 'r') as fin:
       buf = json.load(fin)
       for ch in channels:
           ped['mean'][ch].append(buf['HG'][f'{ch}'][0])
           ped['RMS'][ch].append(buf['HG'][f'{ch}'][1])

    year, month, day = date.split('-')
    current_date = datetime(int(year), int(month), int(day), 0, 0, tzinfo=timezone.utc)
    dc = np.mean(np.array(current_list[6])[current_date - np.array(date_list) == timedelta(days=0)])
    dark_current.append(dc) 


hep.style.use("CMS")
plt.figure(figsize=(16,12))
# plt.title(f'Dark Current Monitor for Irradiated S14160 3015 SiPM')

fdir = os.path.expanduser(f'{CALIROOT}/figures/misc/')
colors = cm.rainbow(np.linspace(0, 1, 4))

plt.clf()
plt.xlabel(r'Dark Current [$\mu$A]')
plt.ylabel(f'Pedestal Mean [ADC]')
i=0
for ch in channels:
    plt.scatter(dark_current, ped['mean'][ch], color=colors[i], label = f'Ch {ch}')
    i+=1
plt.legend(loc = 'upper left')
plt.savefig(f'{fdir}/ped_mean_vs_DC.pdf')

plt.clf()
plt.xlabel(r'$\sqrt{Dark\ Current}\ [\sqrt{\mu A}$]')
plt.ylabel(f'Pedestal RMS [ADC]')

i=0
for ch in channels:
    plt.scatter(np.sqrt(dark_current), ped['RMS'][ch], color=colors[i], label = f'Ch {ch}')
    i+=1
plt.legend(loc = 'upper left')
plt.savefig(f'{fdir}/ped_RMS_vs_DC.pdf')

