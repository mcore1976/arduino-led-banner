# arduino-led-banner
This is nice and simple LED BANNER constructed of Arduino Digispark and Daisy-chained set of MAX7219 LED 8x8 matrix. 
Displays up to 500 characters long scrolling text message that is stored within non-volatile EEPROM memory of the chip.

To upload text for display use Putty terminal (or other serial port terminal) and connect to virtual Serial Port of ATTINY85 Digispark ( /ttyACM0 on linux or COMxx on Windows ).
You can also use FTDI232 USB-to-Serial converter and connect to ports of DIGISPARK D3, D4, GND if using SoftSerial version.

Type the text using keyboard and press ENTER. It will be stored in EEPROM memory of ARDUINO.
After ARDUINO disconnected from the PC and connected to USB PowerBank, the device will display scrolling text uploaded from USB / virtual Serial Port. 

The code uses DigiCDC USB library and you need to have necessary driver in your operationg system (Linux has it built in, for Windows you need to download Digispark drivers : http://digistump.com/board/index.php?topic=2321.15) - http://digistump.com/wiki/digispark/tutorials/digicdc.

The MAX 7219 modules should be connected in daisy chain. The code supports up to 12 MAX 7219 modules connected in daisy chain but remember that your USB 5V has to be strong enough to power such number of LED displays.  Please declare number of modules used in the code within 
#define MODULESNUMBER           <my-number-of-MAX-7219-modules>
 
 Both types of scrolling are available : RIGHT to LEFT (for Roman characters)  , LEFT to RIGHT (for Arabic characters). Choose proper source code INO version.

connections :
 
USB version
 - Daisy chained  MAX7219 DIN PIN - Arduino Digispark  PB0 / P0 
 - Daisy chained  MAX7219 CS PIN  - Arduino Digispark  PB1 / P1 
 - Daisy chained  MAX7219 CLK PIN - Arduino Digispark  PB2 / P2
 - Daisy chained  MAX7219 VCC PIN - Arduino Digispark  5V pin
 - Daisy chained  MAX7219 GND PIN - Arduino Digispark  GND pin

 additional connections for FTDI232 Soft serial version
 
 - connect FTDI232 TX to Digispark pin P3 / D3 (RX),
 - connect FTDI232 RX to Digispark pin P4 / D4 (TX),
 - connect FTDI232 GND to Digispark GND pin
 
 
 
You can see how the device works in this video :  https://youtu.be/sqZCjQiLk7E 
 


