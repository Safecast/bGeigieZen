# bGeigieZen NFW: Simplified Assembly Guide

## Safety Warnings
- Work in a well-ventilated area (solder smoke is hazardous)
- Wear safety glasses when cutting leads
- Soldering iron temperature: 200-350°C (400-650°F)
- Handle components gently, especially the LND_7317 pancake sensor (mica covering is easily damaged)
- NEVER short circuit an 18650 battery (fire/explosion hazard)

## Required Tools/Parts
- Soldering iron with fine tip and solder (60/40)
- Nippers for cutting pin headers
- Screwdrivers (small flat-head and Phillips)
- USB-C cable for programming/charging
- Optional: Isopropyl alcohol for cleaning flux
- bGeigieZen NFW kit parts are below 

## Extra Components
- Battery: 650 mm 3.7V 3000mA unprotected flat-top type 

## Assembly Steps

### 1. Prepare the Board
1. Solder the 2×15 pin (#8 in parts picture) connector to the board
   - Place the pin connector with top up on the board(#5 in parts picture) , turn over the board while holding the pin header
   - Solder one corner pin, check alignment, then solder remaining pins
2. Solder the 1×3 pin header(#7 in parts picture) for the Safepulse (#13 in parts picture)
3. Solder the 1×2 pin header(#7 in parts picture) for the Safepulse(#13 in parts picture)

### 2. Install Diode, Fuse, and Wireless Charger Connector
1. Place the diode(#17 in parts picture) with the white band pointed to the fuse (#15 in parts picture)
2. 
3. Check orientation and placement carefully
4. Solder the diode, fuse, and the wireless charger connector in place 
5. Cut the extra length of the leads of the diode off.

### 3. Install the Safepulse Module
1. Position the board with the backside up 
2. Solder all pins of the 3 pins header and the 2 pin header on the board
3. place the safepulse and solder the 
4. Solder the wire for the anode connector (red)

### 4. Prepare and Install the GPS Module
1. For M10050-HT2828 module, follow the connector pins picture below

1. #### Coloring can be different on the module. 

| GPS module | bGeigieZen board |
| ---------- | ---------------- |
| G          | G                |
| R          | TX               |
| T          | RX               |
| V          | 5V               |



### 5. Install Battery Holders



1. Install battery holder clips in appropriate holes
2. Solder one pin of each holder, check alignment, then solder remaining pins
3. Ensure flat bottom of the holder touches the board

### 6. Prepare and Install the Tube
1. HANDLE WITH EXTREME CARE - grid is fragile
2. Place tube with grid down into case 
3. remove the 3M stick paper of the tube
4. Place board in case without moving tube
5. Install M3 15mm screws for mounting M5StackCore (DON'T OVERTIGHTEN)
6. Solder the cathode wire (black) to Safepulse
7. Solder the anode connector (red wire) to the board
8. Insert SD card

### 7. Final Assembly
1. For QI charging:
   - Cut hole for receiver cable
   - Solder QI receiver (green small board with LEDs) red to plus and black to minus
2. Close case (should have soft "rubber touching case" sound)
3. Confirm that top of rubber liner is 2mm above the board

## Software Installation
1. Install USB VCP Driver
2. Install M5Burner/UIFlow Firmware Burning Tool
3. Connect the M5StackCore via USB-C
4. Select your M5StackCore type and search for "Zen"
5. Download and burn the firmware
6. Configure by editing the SAFEZEN.txt file on SD card

## Notes and Tips
- When powering down, wait a few seconds before powering up again
- Use flat-head 18650 batteries for better fit
- M5StackCore Black needs power button pressed to power up
- Ensure the aluminum connector of the anode is firmly screwed on (loose connections cause incorrect counts)
- Most kits already have protective mesh screen on sensor (check before assembly)

*For detailed instructions or troubleshooting, refer to the complete manual.*