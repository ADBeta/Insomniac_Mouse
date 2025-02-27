# Insomniac Mouse MkI
USB HID Mouse Device, which moves randomly to keep PCs awake -
make games harder to play, annoy co-workers and friends, or to waste time 
watching it dance!

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
```
|    Mode    | J1  | J2  | J3  |        Description        |
|   Normal   |  0  |  0  |  0  |    ±125 Units Movement    |
|   Hi-Res   |  0  |  0  |  1  |    ±250 Units Movement    |
|   Jitter   |  0  |  1  |  0  |    ±20 Units Movement     |
|   Unused   |  0  |  1  |  1  |                           |
|   Unused   |  1  |  0  |  0  |                           |
|   Unused   |  1  |  0  |  1  |                           |
|   Unused   |  1  |  1  |  0  |                           |
|   Unused   |  1  |  1  |  1  |                           |
```

## Uses
Creating artwork  
<div style="display: flex; align-items: center; gap: 10px;">
  <img src="/Images/4-col.png" alt="art 4 colour" width="800"><br>
  <img src="/Images/12-col.png" alt="art 12 colour" width="800"><br>
  <img src="/Images/confetti.png" alt="art confetti" width="800"><br>
</div>

## PCB Specification
A 2.0mm PCB is preffered, but due to high cost, 1.6mm PCBs will work fine,
especially with an added shim of plastic on the rear of the board.  
Designed with Simplicity and solderability in mind, this PCB is easy to
hand-solder, or reflow using a hotplate or hot-air gun.

## Partslist
```
U1    CH32V003J4M6
U2    AP2112K-3.3
C1    0603 100nF MLCC
C2    0603 100nF MLCC
R1    0603 1K5
```

## CHANGELOG
* V1.0.1 - Fixed a compatability issue with Windows
* V1.1.2 - Refactor functions and vairable names
* V1.2.0 - Added Diagonal Movement for USB HID Data, improves smoothness
* V1.3.2 - Changed Jumper Detection Method & added `JITTER` Mode

## TODO
* Random UID on boot

----
MIT License
Copyright (c) 2025 ADBeta
