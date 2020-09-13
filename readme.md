# Zorro-LAN-IDE
 
This is a Zorro-expansion card for the big Amigas (A2000/300/4000 or A500 with Zorro-interface). It has a SANAII-compatible Ethernet driver, an AT-BUS compatible IDE controller and a clockport for variuous expansions like a RapidRoard from Individual Computers.

Here is a pic of the first revision:
![Zorro-Card with LAN-Module](https://gitlab.com/MHeinrichs/Zorro-LAN-IDE/raw/master/Bilder/Aufgebaut.jpg)

All components are Autoconfig. The FastEthernet is based on a ENC624J600 lan chip. The clockport is compatible to A600/A1200 ones but resides at a different address. The IDE works from Kickstart 1.3 to 3.1.4 !

At present there are two firmwares: One for Zorro2, which runs in every Amiga with Zorro-Sots and one for A3000/4000, which puts the LAN-Chip into a Zorro3 environment. The while the Zorro2 variant can transfer 3.5mb/s at most the Zorro3-variant has a significantly faster LAN<->Amiga interface of 6-7mb/sec. Please note that these RAW data transfer numbers are not the only factor in LAN performance on Amiga and that the actual observed speeds with TCP transfers are in the range of 1-2 MB/s (depending on IP stack and CPU). 

Here is a pic of the current board revision:
![Zorro-Card with onboard LAN chip](https://gitlab.com/MHeinrichs/Zorro-LAN-IDE/raw/master/Bilder/LANIDEv03.JPG)


## Things to improve

* The second revision already adresses the issues of the Zorro3-incompatible IDE-Interface.
* There is no Z2/Z3 combo firmware, yet.
* The RJ45-breakout (actually it's a MagJack with build in transceiver) of the 2nd revision looks bad 

**DISCLAIMER!!!!**  
**THIS IS A HOBBYIST PROJECT! I'M NOT RESPONSIBLE IF IT DOESN'T WORK OR DESTROYS YOUR VALUABLE DATA!**

## License
All sources published under GNU GENERAL PUBLIC LICENSE V2.0

## Theory of operation
* This PCB has four layers and consists of a Xilinx XC95288XL, a bootrom and some bus bufferes for the IDE port, a clockport and a ENC624J600 including a RJ45-connector with integrated receiver. 

* The CPLD does four things:
    * It makes the Autoconfig for the IDE, the clockport and the LAN-chip
    * It adapts the LAN Chip to Zorro2/3 and does necessary byte swapping, so the Amiga can linearely access the whole chip.
    * It acesses the clockport
    * It controlls the IDE

## How to make the clockport work with a RapidRoad?

Make sure you connected the cable the right way! There is no "standarized way". So the easiest thing is to find out on the RapidRoad side on which row of the clockport the two outer pins are ground. These Pins mus run to the row of the Zorro-LAN-IDE-CP-card where the squared solder-hole is located. *Please triple check the cable with a multimeter! Wrong polarity of the cable can cause damage to your hardware!* 
Now you have to set up the driver to look for a clockport at the address of the expansion board. The address can be figured out by the system tool "ShowConfig" (Manufactor id: 2588 Product: 124). 
However, there is work on a software solution, that the RapidRoad finds itself automatically!

## What is in this repository?
This repository consists of several directories:

* Bilder: Some pictures of this project
* Doku: Some documentation and datasheets. Mainly for the ENC624J600
* Driver: Here is the driver developped by Henryk Richter
* Logic: Here the code, project files and pin-assignments for the CPLD is stored. There are two versions: 
* PCB: The board, schematic and bill of material (BOM) are in this directory. 
* root directory: the readme

## How to open the PCB-files?
You find the eagle board and schematic files (brd/sch) for the pcb. The software can be [downloaded](https://www.autodesk.com/products/eagle/overview) for free after registration. KiCad can import these files too. But importers are not 100% reliable... 

## How to make a PCB?
You can generate Gerber-files from the brd files according to the specs of your PCB-manufactor. Some specs: min trace width: 0.15mm/6mil, min trace dist: 0.15mm/6mil, min drill: 0.3mm

**ATTENTION: THE PCB has 4 layers!**


## How to get the parts?
Most parts can be found on digikey or mouser or a parts supplier of your choice.
A list of parts is found in the file [ZORRO-LAN-IDE-CP-V2E.txt](https://gitlab.com/MHeinrichs/Zorro-LAN-IDE/raw/master/PCB/ZORRO-LAN-IDE-CP-V2E.txt)

## How to programm the board?
The CPLD must be programmed via Xilinx IMPACT and an apropriate JTAG-dongle. The JED-File is provided. 

## It doesn't work! How to I get help?
For support visit the [a1k-forum](https://www.a1k.org/forum/showthread.php?t=56940). Yes, you need to register and it's German, but writing in English is accepted. For bug-reports write a ticket here ;). 

