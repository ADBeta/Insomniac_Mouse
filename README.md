# Insomniac Mouse MkII
USB HID Mouse Device using a 10¢ Microcontroller.  
Moves randomly to keep PCs awake; make games harder to play; annoy co-workers 
and friends; or to give you a digital fishbowl to watch.  
Each board has jumpers to change the Mouse Behaviour, a lanyard loop,
and is designed with minimum cost and complexity.


Using [ch32v003fun](https://github.com/cnlohr/ch32v003fun) and 
[rv003usb](https://github.com/cnlohr/rv003usb) to create a Software Defined
HID Mouse - it then uses `Bresenham's Line Algorithm` to plot lines for the
mouse to move.

<div style="display: flex; align-items: center; gap: 10px;">
  <img src="/Images/Schematic.png" alt="Schematic" width="800"><br>
  <img src="/Images/PCB_2D.png" alt="PCB_2D" width="800"><br>
  <img src="/Images/PCB_Front.png" alt="PCB_Front" width="400">
  <img src="/Images/PCB_Back.png" alt="PCB_Back" width="400">
</div>


## Jumpers

|    Mode    | J1  | J2  | J3  |        Description        |                             Usecase                       |
|------------|-----|-----|-----|---------------------------|-----------------------------------------------------------|
|   Normal   |  0  |  0  |  0  |    ±125 Units Movement    |           Default balance of speed and distance           |
|   Hi-Res   |  1  |  0  |  0  |    ±250 Units Movement    |            Longer movement for Hi DPI Displays            |
|   Jitter   |  0  |  1  |  0  |     ±20 Units Movement    |    More chaotic movement. Good for messing with games     |
|   Stepped  |  1  |  1  |  0  | ±2 Unit Movement (Slower) |     Slow mode for controllability while plugged in        |
|   Unused   |  0  |  0  |  1  |                           |                                                           |
|   Unused   |  1  |  0  |  1  |                           |                                                           |
|   Unused   |  0  |  1  |  1  |                           |                                                           |
|   Unused   |  1  |  1  |  1  |                           |                                                           |


## Uses
### Keeping PCs awake
Even if you disable Screensaver/Sleep mode, some PCs go to sleep anyway - 
this forces the machine to stay awake regardless.  
This is especially useful on Industial, Unmanaged and headless machines.

### Testing USB Ports
Due to the very low power consumption, small size, lanyard loop, USB 1.0
connection spec, and simple HID data, this is a perfect cheap USB Tester.

### Activity Monitor Spoofing
Because this moves the mouse like any other HID Mouse, it keeps users as
`"Online"` on Teams, Slack etc programs which monitor user activity.  
**WARNING This is not recommended, and any touble you get into for doing this
is your own fault.**

### Pranks / Game Challenges
Guaranteed to annoy your friends and make games **much** harder to play.  

### Creating Artwork
Hold down the mouse in an art package of your choice to create 
some.... interesting art.  
<div style="display: flex; align-items: center; gap: 10px;">
  <img src="/Images/4-col.png" alt="art 4 colour" width="800"><br>
  <img src="/Images/12-col.png" alt="art 12 colour" width="800"><br>
  <img src="/Images/confetti.png" alt="art confetti" width="800"><br>
</div>


## PCB Specification
The overall thickness of the PCBs **MUST** be between 2.0mm and 2.4mm.  
`MkII` Offers more adaptability to get to this final thickness easily.
* Order `Insomniac_USB_Gerb` as a 2.0mm PCB
* Order `Insomniac_USB_Gerb` as a 1.6mm PCB and glue on a 0.4mm - 0.8mm Shim
* Order `Insomniac_USB_Gerb` and `Insomniac_Shim_Gerb` as 1.2mm or 1.0mm PCBs and glue together


## Partslist
```
U1    CH32V003J4M6       SOIC-8
U2    AP2112K-3.3        SOT-23-5
C1    100nF MLCC         0603
C2    100nF MLCC         0603
R1    1.5K Ohm           0603
```


## CHANGELOG
* V1.0.1 - Fixed a compatability issue with Windows
* V1.1.2 - Refactor functions and vairable names
* V1.2.0 - Added Diagonal Movement for USB HID Data, improves smoothness
* V1.3.2 - Changed Jumper Detection Method & added `JITTER` Mode
* V1.4.0 - Refined the code and added `CALM` Mode.
* V1.5.0 - Added USB Serial Number based on UUID of the MCU
* V2.0.0 - MkII PCB and minor bug fixes & refactor


## TODO


----
MIT License
Copyright (c) 2025-2026 ADBeta
