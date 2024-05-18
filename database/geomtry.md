# geometry
* 20240515
    * change P36Ch0-3 to P37Ch0-3, and move them from board 42 to 20.

* 20240429
    * align the prototype
    * old: 
	Layer 0: 69 cm
   	Layer 11: 71 cm
    * new:
	~47 cm (measure it again)

* 20240418
    * First 9 layers connected + top right of Layer 11 (the one after the showermax 
      trigger tiles)
    * Layer 5 is a transition layer, with 2 4-channel PCBs on top and 
      2 7-channel PCBs on bottom, only 4 channels (0-3) of the 7-channel
      PCBs are connected
    * Only 3 PCBs in layer 9

* 20240417
    * First 8 layers connected 
    * P3CH3 and P3CH5 are broken
    * Layer 5 is a transition layer, with 2 4-channel PCBs on top and 
      2 7-channel PCBs on bottom, only 4 channels (0-3) of the 7-channel
      PCBs are connected

* 20240322
    * First 2 layers connected
    * P3CH3 and P3CH5 are broken


# PCB map (counterclockwise, starts from top right)
Layer 1:  1,  2,  3,  4
Layer 2:  5,  6,  7,  8
Layer 3:  10, 9,  11, 12
Layer 4:  26, 13, 25, 28
Layer 5:  30, 41, 14, 37
Layer 6:  38, 54, 21, 18
Layer 7:  44, 22, 58, 47
Layer 8:  50, 57, 33, 31
Layer 9:      15, 46, 35
Layer 10:
Layer 11: 42

## Channels connection
* Bd 14: this is a 7-channel PCB on the left, but only SiPM 1-4 are connected
* bd 37: A 7-channel PCB on the right, connect the top 4 SiPMs (top down)
* Bd 15, 21, 46: 4-channel PCB on the left, No SiPM labelling, connect channel 0-3 top down
* Bd 18, 35, 38: 4-channel PCB on the right, channel 0-3 top down
