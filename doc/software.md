# operation
# DRS4
~/local
./drs4eb/software/bin/drsosc &

# Janus
~/local/Janus_3.2.4_Linux_20230822/bin$ 
python3 JanusPy.pyw &



# installation
# drsosc
## drs4eb
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
**make sure you use the same account for downloading and activating labview**
otherwise it will fail activation.

download labview package: 
    * ni-labview-2024-community-24.1.1-zip
    * NILinux2024Q1Drivers.zip
```
unzip ni-labview-2024-community-24.1.1-zip NILinux2024Q1Drivers.zip
sudo dpkg --install ./ni-labview-2024-community-24.1.1.49154-0+f2-ubuntu2204_all.deb
sudo dpkg --install NILinux2024Q1Drivers/ni-ubuntu2204-drivers-stream.deb
sudo apt update
sudo apt grade
sudo apt install ni-labview-2024-community
sudo apt install ni-visa ni-488.2
```
