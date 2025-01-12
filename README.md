# Insomniac Mouse MkI
USB HID Mouse Device, which moves randomly to keep PCs awake -
make games harder to play, annoy co-workers and friends, or to waste time 
watching it dance!

## Firmware
Using [ch32v003fun](https://github.com/cnlohr/ch32v003fun) and 
[rv003usb](https://github.com/cnlohr/rv003usb) to create a Software Defined
HID Mouse - it then uses `Bresenham's Line Algorithm` to plot lines for the
mouse to move.


## Jumpers
```
|   Jumper   |     Effect     |    Open    |   Closed   |
|     J1     |  MAX_DISTANCE  |    ±125    |    ±250    |
|     J2     |     UNUSED     |      -     |      -     |
|     J3     |     UNUSED     |      -     |      -     |
```

## PCB Specification

## Partslist

## CHANGELOG
* V1.0.1 - Fixed a compatability issue with Windows

## TODO
* Make Mouse Update smoother : antialias, merge two instructons into one
to stop stair-stepping (only if diagonal)
* Modes via solder jumper for the mouse to do

----
MIT License
Copyright (c) 2025 ADBeta
