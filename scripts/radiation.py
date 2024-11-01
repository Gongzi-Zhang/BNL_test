import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import scipy
from scipy.optimize import curve_fit
import mplhep as hep
from datetime import datetime, timezone, timedelta
from matplotlib.pyplot import cm

fdir = os.path.expanduser('~/BNL_test/figures/misc/')
hep.style.use("CMS")
plt.figure(figsize=(16,12))

# benchmark IV curve
path = os.path.expanduser('~/labview/irradiated/')
files = ['3015A8_IV20sDelay_UTC_20240620__22_03','3015A9_IV20sDelay_UTC_20240620__23_01','3015A10_IV_UTC_20240621__22_13','3015A11_IV_UTC_20240621__22_31','3015A12_IV_UTC_20240621__23_00','3015A13_IV20sDelay_UTC_20240621__00_34']
OV = [-0.5, 0.5, 1.0, 1.5, 2.0, 2.5]
fluence = [8, 9, 10, 11, 12, 13]

DI = [[],[],[],[],[],[]]
for file in sorted(os.listdir(path)):
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

plt.ylabel(r'Dark Current ($\mu$A)')
plt.xlabel('Over Voltage (V)')
plt.yscale('log')
plt.legend()
plt.savefig(f'{fdir}/benchmark_IV.png')

# CALI radiation level
plt.clf()
plt.yscale('log')
average = []
color = cm.rainbow(np.linspace(0, 1, 6)) 
dark_current = [0, 0.02280694, 0.4995346, 2.339789, 5.472342, 10.01141]    # dark current on 2024/9/30
for i in range(6):
    plt.errorbar(fluence, DI[i], fmt='-o', label=f'OV = {OV[i]} V', c=color[i])

    if OV[i] >= 1:
        plt.axhline(dark_current[i], linestyle='--', c=color[i])
        cali_fluence = np.interp(np.log(dark_current[i]), np.log(DI[i]), fluence)
        plt.axvline(cali_fluence, c=color[i])
# plt.axvline(9.52, c='r', linestyle='dashed')
plt.legend()
plt.title('Peak radiation level recorded on 09/30')
plt.ylabel(r'Dark Current ($\mu$A)')
plt.xlabel(r'log(Fluence) (log($N_{p^+}$/$cm^2$))')
plt.savefig(f'{fdir}/radiation.png')

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
