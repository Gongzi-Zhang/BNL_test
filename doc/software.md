# operation
# DRS4
```
cd ~/local
./drs4eb/software/bin/drsosc &
```
## calibration
config -> -0.05 to +0.95 V -> Execute Voltage Calibration
## problem shooting
* No DRS found, running in demo mode
    > reboot DRS4

# Janus
```
cd ~/local/Janus_5202_3.6.0_20240514_linux/bin
python3 JanusPy.pyw &
```

# GlobalProtect
* Download globalprotect installer for linux on UCR VPN webpage
> globalprotect connect -p campusvpn.ucr.edu


# installation
# drsosc
## drs4ebp -- discarded
* fetch drs4eb source code from [git repository](https://bitbucket.org/ritt/drs4eb.git) 
```
git clone https://bitbucket.org/ritt/drs4eb.git .
cd drs4eb
sudo cp 41-drs.rules /etc/udev/rules.d/
cd software/
mkdir build
cd build
cmake ..
make install
```

```
sudo apt install drs4eb
```


## drs-5.0.6 doesn't work on Ubuntu-22.04
drs-5.0.6 requires libwx_gtk2u_adv-3.0, wxWidgets

## access usb device
* To access the usb device, one needed to be add to the `plugdev` group:
```
sudo usermod -a -G plugdev $USER
```
And a rule file under /etc/udev/rules.d/
```
# I don't know which one actually works
sudo cat 'ATTRS{idVendor}=="04b4", ATTRS{idProduct}=="1175", SUBSYSTEMS=="usb", ACTION=="add", MODE="0666", GROUP="plugdev"' >> /etc/udev/rules.d/41-drs.rules
sudo cat 'ATTRA{idVendor}=="04b4", ATTRS{idProduct}=="1175", SUBSYSTEM=="usb", ACTION=="add", MODE="0660", GROUP="plugdev"' >> /etc/udev/rules.d/41-drs.rules
sudo udevadm control --reload
# maybe need to reboot
```

# Janus
* Janus_5202_3.6.0 doesn't work on Ubuntu 22.04
* Janus 3.2.4 works on Ubuntu 22.04
```
tar Janus_3.2.4_Linux_20220822.tar.gz
cd Janus_3.2.4_Linux_20220822
sudo bash Janus_Install.sh
sudo udevadm control --reload
sudo udevadm trigger

```

#labview
## installation
### windows
* https://systems.engr.ucr.edu/software/labview


# Singularity for eic-shell
```
sudo apt-get install -y build-essential libssl-dev uuid-dev libgpgme-dev squashfs-tools libseccomp-dev wget pkg-config git cryptsetup-bin

# installing Go
wget https://go.dev/dl/go1.23.1.linux-amd64.tar.gz  # used the newest verion: https://go.dev/dl/
export PATH=/usr/local/go/bin:$PATH

# Singularity
wget https://github.com/singularityware/singularity/releases/download/v3.5.3/singularity-3.5.3.tar.gz
tar -xzvf singularity-3.5.3.tar.gz

cd singularity
./mconfig 
cd builddir
make
sudo make install
```

# cvmfs
```
wget https://ecsft.cern.ch/dist/cvmfs/cvmfs-release/cvmfs-release-latest_all.deb
sudo dpkg -i cvmfs-release-latest_all.deb
rm -f cvmfs-release-latest_all.deb
sudo apt-get update
sudo apt-get install cvmfs
```
## configuration
create */etc/cvmfs/local.default* with the following content:
CVMFS_REPOSITORIES=sw-nightlies.hsf.org,singularity.opensciencegrid.org
CVMFS_CLIENT_PROFILE=single

## setup
```
cvmfs_config setup
cvmfs_config probe
```

## debug
If the probe fails, try to restart *autofs*
```
sudo systemctl restart autofs
```

# eic-shell: https://github.com/eic/eic-shell

# Virtual machine: KVM
* default network is not active
```
sudo virsh net-list --all
sudo virsh net-start default
```
Looks like the above command will result in 'no internet connection' in the VM,
run the following command to fix it:
```
sudo virsh net-destroy default
sudo virsh net-start default
sudo systemctl restart libvirtd
```


# Keithley
* If there is empty txt output, it means the script is working, but the device
is not connected. Try to re-add the hardware in QEMU.
