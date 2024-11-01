import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import mplhep as hep
from datetime import datetime, timezone, timedelta

# hep.set_style(hep.style.CMS)
breakV = 38.74096661764706
directory = os.path.expanduser('~/labview/data')
Voltage = []
Current = []
date = []
OV = [-0.5, 0.5, 1.0, 1.5, 2.0, 2.5] 
for file in sorted(os.listdir(directory)):
    file = f'{directory}/{file}'
    if os.stat(file).st_size == 0:
        continue
    with open(file) as f:
        lines = f.read().split(',')
    voltage = np.array(lines[::2], dtype=float)
    current = 1e6*np.array(lines[1::2], dtype=float)
    Voltage.append(voltage)
    Current.append(current)

    timestamp = file.split('UTC_')[1].split('.txt')[0]
    yyyymmdd = list(timestamp.split('__')[0])
    month = yyyymmdd[4]+yyyymmdd[5]
    day = yyyymmdd[6]+yyyymmdd[7]
    hour = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[0]
    minute = file.split('UTC_')[1].split('.txt')[0].split('__')[1].split('_')[1]   
    date.append(datetime(2024, int(month), int(day), int(hour), int(minute), tzinfo=timezone.utc) + timedelta(hours=-4))    # use NY time

hep.style.use("CMS")
plt.figure(figsize=(16,12))
plt.title(f'Dark Current Monitor for Irradiated S14160 3015 SiPM')
# plt.xlabel('Time (UTC)')
plt.ylabel(r'Current ($\mu$A)')

#timestamps = [datetime.strptime(ts, '%m/%d/%Y %I:%M:%S %p') for ts in date]

for i in range(5):
    # find the highest dark current: 2316
    current = np.array(Current)[:, i*7+35]
    # idx = np.argmax(current)
    print(current[2316], date[2316])
    plt.errorbar(date, np.array(Current)[:,i*7+35], label=f'OV = {np.average(np.array(Voltage)[:,i*7+35])-breakV:.1f} V', fmt='--o' if np.average(np.array(Voltage)[:,i*7+35])-breakV > 0 else '-^')

plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%m-%d'))
# plt.yscale('log')
plt.xticks(rotation=45)
plt.legend()
# plt.show()
fdir = os.path.expanduser('~/BNL_test/figures/misc/')
plt.savefig(f'{fdir}/IV.png')
