# Traffic Light Simulator

ECE 3720 Final Project

**Group members:** Anthony Putrus, Mario Ivezaj, Justin Aquilina, David Hurlbert

### Link's to Google Doc's
- Notes/ideas ----> (https://docs.google.com/document/d/1CXY0ckCbr0uh1d2bbIugTZ3-Kkri9Xm0IxcwYGNHimU/edit?usp=sharing)
- Status Report ----> (https://docs.google.com/document/d/1IT9ALkAubmx2YQJg4QNXKdJEr5QOwmcFIZtEcxEPNfI/edit?usp=sharing)


## Car motion and signal outputs (Mario Ivezaj)

Drives the six traffic-signal outputs and the two car motors, and makes
the cars obey the lights: go on green, slow on yellow, stop at the stop
line on red, brake for emergency vehicles.

Files: pins.h, traffic_types.h, clock.c/.h, inputs.c/.h, lights.c/.h,
motor.c/.h, car_motion.c/.h, signal_seq.c/.h, main.c

### Hardware other than the Dragon12 (this subsystem)
- 2 small DC gear motors, 3-6 V (on-board SN754410 H-bridge, T4 M1-M4)
- 12 LEDs (4 red, 4 yellow, 4 green) with 12 x 470 ohm resistors on PA0-PA5
- 2 IR reflective sensor modules, digital output, on PH0/PH1 (planned)
- Optional 5-9 V motor supply on VEXT

### Board setup
- Move the jumper from J24 to J18 (H-bridge power; disables the 7-segment display)
- All DIP switches (SW1) up
- Do not press the keypad while running (it shares Port A)

### Demo controls
- SW5 / SW4: hold = car at the N/S / E/W stop line
- SW3: end the current green early (stand-in for the camera)
- SW2: emergency vehicle on/off

### References
- EVBplus, Dragon12-Plus Trainer User's Manual: pin assignments, jumpers, PLL
  template (all files)
- MicroDigitalEd, DC Motor Control H-Bridge sample program: H-bridge pin
  mapping and PWM settings (motor.c)
- NXP AN2882, Using Real-Time Interrupt on HCS12: RTI setting (clock.c)

## Object Detection and UART Transmission (David Hurlbert)

Implement computer vision techniques to detect 1 or more cars waiting at
the side street part of the intersection. Transmit camera data to HCS12
via UART.

Files: Raspberry Pi 5 Terminal Communication, cardet1-cpp-mcu-v2-impulse-#1.zip

### Hardware other than the Dragon12 (this subsystem)
- Raspberry Pi 5
- Raspberry Pi 5 Camera Module 3

### Board setup
- Connect mouse, keyboard, and monitor via USB and HDMI (for demo)
- Connect Pi Camera Module 3 to CAM/DISP 1
- Power with 5v USB-C Power Supply

### Demo controls
- Enter "edge-impulse-linux" in terminal to connect to machine learning dataset
- Enter "edge-impulse-linux-runner" to view camera data in real time

### References
- Raspberry Pi 5 Camera Module 3 Datasheet: Obtained specifications on Camera Module 3 and how to setup with Pi properly 
https://pip-assets.raspberrypi.com/categories/786-raspberry-pi-camera-module3/documents/RP-008151-DS-1-camera-module-3-product-brief.pdf
- Raspberry Pi 5 Edge Impulse Documentation: Explained how to setup RPI 5 to work with edge impulse, listed required terminal commands
https://docs.edgeimpulse.com/hardware/boards/raspberry-pi-5 
- C++ Library Edge Impulse Documentation: Information on how to use C++ library outputted from edge impulse 
https://docs.edgeimpulse.com/hardware/deployments/run-cpp-overview

## CAD Traffic Intersection Design and Pi to Dragon12 Communication Research (Justin Aquilina)

solid structure to mount electronics on + model intersection for cars to drive on,
camera mounted on the top. Researched 2 different potential methods to communicate between Pi and Dragon12, using UART.

Files: Fusion CAD model ---> (https://a360.co/4emT0JF)

### Notes
The CAD model is just about done, giving us a good base for the project.
The first method to communicate between boards is UART connecting TX/RX pins on both boards,
Using a Logic Level Shifter. The second method technically is not UART, using the Pi's USB to the Dragon12's USB to communicate 
Method 2 uses less components while Method 1 may be a better long term solution.

### References:
- TXS0108E Level Shifter datasheet
https://www.ti.com/lit/ds/symlink/txs0108e.pdf?ts=1794207939823
- how to use a Logic Level Shifter
https://youtu.be/f7aySy_0URE

### UART Receive and Traffic Logic (Anthony Putrus)

Receive and decode the camera byte from the Pi over UART and run the traffic light state machine. The side street is green after waiting long enough for the number of cars detected. Also, an emergency vehicle forces all red. If the Pi goes quiet for 500ms, the controller falls back to plain fixed timing. 

### Status 
pi_link,c: loosely written but not yet tested
traffic.c: foundational code, written, tested, not yet implemented with Mario's cars and David's camera

### Data Format
C1 C0 T5 T4 _ T3 T2 T1 T0
C1-C0 = how many cars waiting on side street?
T5 - T0 = how many seconds since first car was detected? (0-63)
Planned Emergency byte: C = 0, T = 0x3F = 0011_1111 

### Light Logic
Side street gets a green when wait time reaches a threshold for its car count ex: 1 car  = 45 sec, 2 = 30 sec, 3 = 15 sec (placeholder data)
Side green will last about 5s - 15s, will end earlier if Pi reports no cars waiting
Sequence: Main Green -> Main Yellow -> Main/All Red -> Side Green -> Side Yellow -> All Red -> (Repeat)
Emergency Vehicles: Trigger all red till it clears, then start backup to Main green

### References
- EVBplus, Dragon12-Plus Trainer User's Manual: pin assignments, jumpers, PLL
  template (all files)
- Claude (Anthropic) - TO BE UPLOADED WHEN DONE WITH PART
- - ECE 3720 lecture slides, "HCS12 Serial Communication Interface (SCI) Programming
  in Assembly and C," Prof. Shadi G. Alawneh, Oakland University
