#!/usr/bin/env python3

# plot dark-current vs radiation from UC Davis test, 
# with highest radiation from BNL test

import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.pyplot import cm
import scipy
from scipy.optimize import curve_fit
import mplhep as hep
from datetime import datetime, timezone, timedelta
import re

breakV = 38.74096661764706
CALIROOT = os.getenv("CALIROOT")
fdir = os.path.expanduser(f'{CALIROOT}/figures/misc/')
# directory = os.path.expanduser('~/labview/data')
OV = [-1.5, -1.0, -0.5, 0.5, 1.0, 2.0, 2.5, 3.0, 4.0]
# OV = [-0.5, 0.5, 1.0, 1.5, 2.0, 2.5] 

# benchmark IV curve

bnl_time = datetime(2024, 10, 21, 0, 0, tzinfo=timezone.utc)
davis_time = datetime(2024, 5, 13, 0, 0, tzinfo=timezone.utc)
timediff = bnl_time - davis_time
initial = 4.507095

# array([1.92998231e+159, 2.12078275e+002, 3.42416323e+000])
# plt.title('S14160 6015PS SiPM')

def exponential_decay(x, a, b, c):
    return a * np.exp(-b * x) + c

# read in IV txt files
start_time = datetime(2024, 10, 21, 0, 0, tzinfo=timezone.utc)
end_time = datetime(2024, 12, 6, 0, 0, tzinfo=timezone.utc)
Current = []
Voltage = []
HighestDarkCurrent = []
for i in range(9):
    HighestDarkCurrent.append([])
breakdown = []
Time = []

directory = os.path.expanduser(f'{CALIROOT}/work/labview/')
selected_date = datetime(2024, 10, 1, 0, 0, tzinfo=timezone.utc)
for f in sorted(os.listdir(directory)):
    timestamp = f.split('UTC_')[1].split('.txt')[0]
    yyyymmdd = list(timestamp.split('__')[0])
    month = yyyymmdd[4]+yyyymmdd[5]
    day = yyyymmdd[6]+yyyymmdd[7]
    hour = f.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[0]
    minute = f.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[1]
    time = datetime(2024, int(month), int(day), int(hour), int(minute), tzinfo=timezone.utc)
    timediff = time - selected_date

    if timedelta(days=-1) <= timediff and timediff <= timedelta(days=1):
        with open(directory + f, 'r') as file:
            lines = file.read().split(',')
        if lines[0] == '': continue
        voltage = lines[::2]
        current = lines[1::2]
        voltage = np.array(list(map(float, voltage[1:])))
        current = np.array(list(map(float, current[1:])))*1e6
        dIdV = (np.log(current[1:])-np.log(current[:-1]))/(voltage[1:]-voltage[:-1])
        # breakV = voltage[np.argmax(dIdV)]/2+voltage[np.argmax(dIdV)+1]/2
        for i in range(9):
            HighestDarkCurrent[i].append(current[np.argmin(abs(voltage-breakV-OV[i]))])

    if time > start_time:
        with open(directory + f, 'r') as file:
            lines = file.read().split(',')
        if lines[0] == '': continue
        Time.append(time)

        voltage = lines[::2]
        current = lines[1::2]

        voltage = np.array(list(map(float, voltage[1:])))
        current = np.array(list(map(float, current[1:])))*1e6
        dIdV = (np.log(current[1:])-np.log(current[:-1]))/(voltage[1:]-voltage[:-1])
        breakV = voltage[np.argmax(dIdV)]/2+voltage[np.argmax(dIdV)+1]/2

        Voltage.append(voltage)
        Current.append(current)
        breakdown.append(breakV)

stop_time = datetime(2025, 2, 1, 0, 0, tzinfo=timezone.utc)
ts = np.array([t.timestamp()/1e9 for t in Time])
time_range = np.linspace(start_time.timestamp()/1e9,stop_time.timestamp()/1e9,100)
time_diff = np.array([datetime.utcfromtimestamp(t*1e9).replace(tzinfo=timezone.utc) for t in time_range])
c = ['r', 'g', 'b']
for i in range(3):
    #if i != 1: continue
    # plt.errorbar(Time,np.array(Current)[:,49+i*5],fmt='-o',label=f'+{np.array(Voltage)[0][49+i*5]-np.mean(breakdown):.1f} OV',c=c[i])
    coeff, covar = curve_fit(exponential_decay,ts,np.array(Current)[:,49+i*5],maxfev = 800000)
    # plt.plot(time_diff,exponential_decay(time_range,*coeff),linestyle='--',c=c[i])

#plt.yscale('log')
# plt.ylabel(f'Dark Current [$\mu$A]')
# plt.xlabel('Time (UTC)')
# plt.legend()
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m-%d'))
# plt.clf()

stop_time = datetime(2025, 2, 1, 0, 0, tzinfo=timezone.utc)
ts = np.array([t.timestamp()/1e9 for t in Time])
time_range = np.linspace(start_time.timestamp()/1e9,stop_time.timestamp()/1e9,100)
time_diff = np.array([datetime.utcfromtimestamp(t*1e9).replace(tzinfo=timezone.utc) for t in time_range])
c = ['r','g','b']
for i in range(3):
    #if i != 1: continue
    # plt.errorbar(Time,np.array(Current)[0][49+i*5]/np.array(Current)[:,49+i*5],fmt='-o',label=f'+{np.array(Voltage)[0][49+i*5]-np.mean(breakdown):.1f} OV',c=c[i])

    coeff, covar = curve_fit(exponential_decay,ts,np.array(Current)[:,49+i*5],maxfev = 800000)
    # plt.errorbar(time_diff,np.array(Current)[0][49+i*5]/exponential_decay(time_range,*coeff),linestyle='--',c=c[i])
    if i == 0:
        acoeff1 = coeff
        acovar1 = covar
    if i == 1:
        acoeff = coeff
        acovar = covar
    if i == 2:
        acoeff2 = coeff
        acovar2 = covar
#plt.yscale('log')
# plt.ylabel(r'Room Temperature Annealing Ratio ($\frac{I_{initial}}{I_{expanded}}$)')
# plt.xlabel('Time (UTC)')
# plt.legend()
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m-%d'))
# plt.clf()

DI = [[], [], [], [], [], [], [], [], []]
flu = []
fluence = []
fig, ax = plt.subplots()
plt.sca(ax)
Voltage = []
Current = []
breakdown = []

directory = os.path.expanduser(f'{CALIROOT}/doc/Davis_test/')
files = ['Hex_Nonirradiated_UTC_20240724__00_19.txt']
for file in files:
    #if '3015B8.544_IV_UTC_20240514__20_43' not in file: continue
    f = open(directory+file)
    lines = f.read().split(',')
    # create arrays with data
    voltage = lines[::2]
    current = lines[1::2]
    voltage = np.array(list(map(float, voltage[1:])))
    current = 1e6*np.array(list(map(float, current[1:])))
    Voltage.append(voltage)
    Current.append(current)
    #id = file.split('_')[2]
    #plt.errorbar(voltage3,current3,fmt='--v',label=f'Flower Board: Non-irradiated ({id})')
    current_1 = scipy.ndimage.gaussian_filter(current,1)
    dIdV = (np.log(current[1:])-np.log(current[:-1]))/(voltage[1:]-voltage[:-1])
    breakV = voltage[np.argmax(dIdV)]/2+voltage[np.argmax(dIdV)+1]/2
    breakdown.append(breakV)
plt.errorbar(np.mean(Voltage,axis=0),np.mean(Current,axis=0),fmt='-o',label='Non-Irradiated')
plt.fill_between(np.mean(Voltage,axis=0),np.mean(Current,axis=0),np.mean(Current,axis=0),alpha=0.2)
for i in range(9):
    DI[i].append(np.mean(Current,axis=0)[np.argmin(abs(np.mean(Voltage,axis=0)-np.mean(breakdown)-OV[i]))])
flu.append(0)
files = ['Hex_3015A8_UTC_20240724__23_44.txt','Hex_3015A9_UTC_20240725__00_04.txt','Hex_3015A10_UTC_20240725__16_55.txt','Hex_3015A11_UTC_20240725__17_35.txt','Hex_3015A12_UTC_20240724__18_12.txt','Hex_3015A13_UTC_20240725__20_03.txt']
for file in files:
    if 'A' not in file: continue
    if 'Retest' in file: continue
    #if file.split('_')[2] == 'Retest': continue
    with open(directory+file) as f:
        lines = f.read().split(',')
        timestamp = file.split('UTC_')[1].split('.txt')[0]
        yyyymmdd = list(timestamp.split('__')[0])
        month = yyyymmdd[4]+yyyymmdd[5]
        day = yyyymmdd[6]+yyyymmdd[7]
        hour = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[0]
        minute = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[1]
        time = datetime(2024, int(month), int(day), int(hour), int(minute), tzinfo=timezone.utc)
        after0 = exponential_decay((time + timediff).timestamp()/1e9,*acoeff1)
        after1 = exponential_decay((time + timediff).timestamp()/1e9,*acoeff)
        after2 = exponential_decay((time + timediff).timestamp()/1e9,*acoeff2)
        after = (after0+after1+after2) / 3
        std = np.std(np.array([after0,after1,after2]))
        factor = initial / after
        factor0 = initial / (after - std)
        factor1 = initial / (after + std)
        voltage = lines[::2]
        current = lines[1::2]
        voltage = np.array(list(map(float, voltage[1:])))
        current = 1e6*np.array(list(map(float, current[1:]))) * factor
        current0 = current / factor * factor0
        current1 = current / factor * factor1
        #current_1 = scipy.ndimage.gaussian_filter(current,1)
        if '13' in file.split('_')[1] or '12' in file.split('_')[1]:
            voltage = voltage[:75]
            current = current[:75]
            current0 = current0[:75]
            current1 = current1[:75]
        dIdV = (np.log(current[1:])-np.log(current[:-1]))/(voltage[1:]-voltage[:-1])
        breakV = voltage[np.argmax(dIdV)]/2+voltage[np.argmax(dIdV)+1]/2
        FLUENCE = str(int(float(re.split('[A-Z]',file.split('_')[1])[1])))
        plt.errorbar(voltage[:-1],current[:-1],fmt='-o',label=rf'10$^{{{FLUENCE}}}$ N$_{{p^{{+}}}}$')
        plt.fill_between(voltage[:-1],current0[:-1],current1[:-1],alpha=0.2)
        pos = re.split('[0-9]',file.split('_')[1])[4]
        if pos == 'A':
            flu.append(float(re.split('[A-Z]',file.split('_')[1])[1]))
            for i in range(9):
                DI[i].append(current[np.argmin(abs(voltage-breakV-OV[i]))])
#plt.errorbar([4,4],[0.04032,3*0.04032],xerr=[0.2,0.2],linewidth=3)
#plt.text(2.2,0.015,'Expected Dark Current Range', c='black', fontsize=15)
#plt.xlim(35,45)
plt.ylim(1e-3,1e4)
#plt.title('S14160 3015PS SiPM')
plt.ylabel(r'Dark Current [$\mu$A]')
plt.xlabel('Voltage [V]')
plt.yscale('log')
#plt.axhline(1e3,label='Saturation',c='black',linestyle='dashed')
handles, labels = ax.get_legend_handles_labels()
ax.legend(handles[::-1], labels[::-1], loc='upper left')
plt.title('S14160 3015PS SiPM')
plt.show()
#plt.style.use('default')
hep.style.use("CMS")
fig, (ax1, ax2) = plt.subplots(1, 2, sharey=True, figsize=(16,12), facecolor='w',
                                  gridspec_kw={'width_ratios': (1, 5)})
fig.subplots_adjust(wspace=0.05)
#plt.title('S14160 3015PS SiPM')
color = cm.rainbow(np.linspace(0, 1, 9))
for i in range(9):
    ax1.errorbar(flu, DI[i], fmt='-o' if OV[i]>=0 else '--o', label=f'{OV[i]} V', c=color[i])
    ax2.errorbar(flu, DI[i], fmt='-o' if OV[i]>=0 else '--o', label=f'{OV[i]} V', c=color[i])
ax1.set_xlim(-0.5,0.5)
ax2.set_xlim(7.5, 13.5)
ax1.set_ylim(1e-3,1e4)
ax2.set_ylim(1e-3,1e4)
ax1.spines['right'].set_visible(False)
ax2.spines['left'].set_visible(False)
ax2.tick_params(axis='x', which='both', top=False)
#ax2.tick_params(axis='y', length=0)
#ax1.tick_params(axis='y', length=0)
handles, labels = ax2.get_legend_handles_labels()
ax1.legend(handles[::-1], labels[::-1], loc='upper left', fontsize=25)
#ax1.axhline(1e3,label='Saturation',c='black',linestyle='dashed')
#ax2.axhline(1e3,label='Saturation',c='black',linestyle='dashed')
ax1.set_xticks([0])
ax1.set_ylabel(r'Dark Current [$\mu$A]', fontsize=35)
plt.xlabel(r'log(Fluence) [log($N_{p^+}$/$cm^2$)]', fontsize=35)
plt.yscale('log')
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)
#plt.title('S14160 3015PS SiPM')
d = 2.
kwargs = dict(marker=[(-1, -d), (1, d)], markersize=12,
              linestyle="none", color='k', mec='k', mew=1, clip_on=False)
ax1.plot([1, 1], [0, 1], transform=ax1.transAxes, **kwargs)
ax2.plot([0, 0], [0, 1], transform=ax2.transAxes, **kwargs)
ax22 = ax2.secondary_xaxis('top', functions=(lambda x: x+0.176, lambda x: x+0.176))
ax22.tick_params(labelsize=25)
ax22.set_xlabel(r'log(Fluence) [log($N_{n}$/$cm^2$)]', fontsize=35)

for i in range(9):
    if OV[i] >= 1 and OV[i] < 4:
        dark_current = np.mean(HighestDarkCurrent[i])
        plt.axhline(dark_current, linestyle='--', c=color[i])
        cali_fluence = np.interp(np.log(dark_current), np.log(DI[i]), flu)
        plt.axvline(cali_fluence, c=color[i])

plt.savefig(f'{fdir}/radiation.pdf')
