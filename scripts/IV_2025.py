import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import scipy
import mplhep as hep
from datetime import datetime, timezone, timedelta
from zoneinfo import ZoneInfo
from matplotlib.pyplot import cm

breakV = 38.74096661764706
CALIROOT = os.getenv("CALIROOT")
directory = os.path.expanduser(f'{CALIROOT}/IV/')
Voltage = []
Current = []
date = []
OV = [-0.5, 0.5, 1.0, 1.5, 2.0, 2.5] 
II = [[],[],[],[],[],[]]
for file in sorted(os.listdir(directory)):
    file = f'{directory}/{file}'
    if os.path.isdir(file) or os.stat(file).st_size == 0:
        continue

    timestamp = file.split('UTC_')[1].split('.txt')[0]
    dt = datetime.strptime(timestamp, "%Y%m%d__%H_%M")
    dt = dt.replace(tzinfo=timezone.utc)
    # convert to New York time
    dt = dt.astimezone(ZoneInfo("America/New_York"))
    if (dt.year != 2025):
        continue
    date.append(dt)

    with open(file) as f:
        lines = f.read().split(',')
    voltage = np.array(lines[::2], dtype=float)
    current = 1e6*np.array(lines[1::2], dtype=float)
    Voltage.append(voltage)
    Current.append(current)

    for i in range(6):
        II[i].append(current[np.argmin(abs(Voltage[0]-breakV-OV[i]))])


hep.style.use("CMS")
plt.figure(figsize=(16,12))
plt.subplots_adjust(left=0.08, right=0.98, bottom=0.09, top=0.91)
# plt.title(f'Dark Current Monitor for Irradiated S14160 3015 SiPM')
plt.xlabel('Date', fontsize=35)
plt.ylabel(r'Dark Current [$\mu$A]', fontsize=35)
plt.xticks(fontsize=25)
plt.yticks(fontsize=25)

for i in range(6):
    index = np.argmin(abs(Voltage[-1]-breakV-OV[i]))

    plt.errorbar(date, np.array(Current)[:,index], label=f'OV = {np.average(np.array(Voltage)[:,index])-breakV:.1f} V', fmt='--o' if np.average(np.array(Voltage)[:,index])-breakV > 0 else '-^')

beamOffDates = {
        datetime(2025, 12,  8, 10,  0): "AuAu Ends", 
        # datetime(2024, 10,  7, 0,  0): "AuAu Starts", 
        # datetime(2024, 10, 21, 8, 30): "AuAu Ends",
}
for dt, label in beamOffDates.items():
    plt.axvline(x=dt, color='red', linestyle='--')
    plt.text(dt, 24.0, label, color="red", ha="right", va="center", fontsize=29, rotation=90)

plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m/%d'))
# plt.yscale('log')
# plt.xticks(rotation=45)
plt.legend(loc='upper left', fontsize=30)
# plt.show()
fdir = os.path.expanduser(f'{CALIROOT}/figures/misc/')
plt.savefig(f'{fdir}/IV.pdf')

# benchmark IV curve
plt.clf()
path = os.path.expanduser(f'{CALIROOT}/IV/Davis_test/')
files = ['Hex_3015A8_UTC_20240724__23_44.txt','Hex_3015A9_UTC_20240725__00_04.txt','Hex_3015A10_UTC_20240725__16_55.txt','Hex_3015A11_UTC_20240725__17_35.txt','Hex_3015A12_UTC_20240724__18_12.txt','Hex_3015A13_UTC_20240725__20_03.txt']
OV = [-0.5, 0.5, 1.0, 1.5, 2.0, 2.5]
fluence = [8, 9, 10, 11, 12, 13]

DI = [[],[],[],[],[],[]]
for file in files:
    with open(path+file) as f:
        lines = f.read().split(',')

        voltage = lines[::2]
        current = lines[1::2]
        voltage = np.array(list(map(float, voltage[1:])))
        current = 1e6*np.array(list(map(float, current[1:])))

        current_1 = scipy.ndimage.gaussian_filter(current, 1)
        dIdV = (np.log(current_1[1:])-np.log(current_1[:-1]))/(voltage[1:]-voltage[:-1])
        breakV = voltage[np.argmax(dIdV)]/2+voltage[np.argmax(dIdV)+1]/2

        plt.errorbar(voltage-breakV, current, fmt='-o', label=file.split('_')[1])

        for i in range(6):
            DI[i].append(current[np.argmin(abs(voltage-breakV-OV[i]))])

plt.ylabel(r'Dark Current [$\mu$A]')
plt.xlabel('Over Voltage (V)')
plt.yscale('log')
plt.legend()
plt.savefig(f'{fdir}/benchmark_IV.pdf')

# CALI radiation level
plt.clf()
plt.yscale('log')
average = []
color = cm.rainbow(np.linspace(0, 1, 6)) 
selected_date = datetime(2025, 12, 8, 10, 0, tzinfo=ZoneInfo("America/New_York"))
for i in range(6):
    plt.errorbar(fluence, DI[i], fmt='-o', label=f'OV = {OV[i]} V', c=color[i])

    if OV[i] >= 1:
        dark_current = np.mean(np.array(II[i])[selected_date - np.array(date) <= timedelta(days=1)])
        plt.axhline(dark_current, linestyle='--', c=color[i])
        cali_fluence = np.interp(np.log(dark_current), np.log(DI[i]), fluence)
        plt.axvline(cali_fluence, c=color[i])
# plt.axvline(9.52, c='r', linestyle='dashed')
plt.legend()
# plt.title('Peak radiation level recorded on 09/30')
plt.ylabel(r'Dark Current [$\mu$A]')
plt.xlabel(r'log(Fluence) [log($N_{p^+}$/$cm^2$)]')
plt.savefig(f'{fdir}/radiation.pdf')

# plt.figure(figsize=(20,10))
# plt.title(f'Dark Current Monitor for\n Insitu S14160 3015 SiPM \n Estimated Fluence: 10^{np.mean(average):.1f}'r' $N_{p+}$')
# plt.xlabel('Time (UTC)')
# plt.ylabel(r'Current ($\mu$A)')
# 
# for i in range(6):
#     index = np.argmin(abs(Voltage[-1]-breakV-OV[i]))
#     plt.errorbar(date,np.array(Current)[:,index],label=f'OV = {np.average(np.array(Voltage)[:,index])-breakV:.2f} V',fmt='--o' if np.average(np.array(Voltage)[:,index])-breakV > 0 else '-^')
# 
# plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m-%d'))
# #plt.yscale('log')
# plt.xticks(rotation=45)
# plt.legend(ncol=2)
# plt.savefig(f'{fdir}/radiation.png')
