# arduino-led-banner
This is nice and simple LED BANNER constructed of Arduino Digispark and Daisy-chained set of MAX7219 LED 8x8 matrix. 
Displays up to 500 characters long scrolling text message that is stored within non-volatile EEPROM memory of the chip (!0000+ writes possible) - "static mode" 
or in second mode when text is stored in RAM - message can be 80 characters long and frequently changing (every few second) - this is "news ticker mode"

To upload text for display use Putty terminal (or other serial port terminal) and connect to virtual Serial Port of ATTINY85 Digispark ( /ttyACM0 on linux or COMxx on Windows ).
You can also use FTDI232 USB-to-Serial converter and connect to ports of DIGISPARK D3, D4, GND if using SoftSerial version.

Type the text using keyboard and press ENTER. It will be stored in EEPROM memory or RAM memory of ARDUINO (depending on INO script selected).
In EEPROM version after ARDUINO disconnected from the PC and connected to USB PowerBank, the device will display scrolling text uploaded from USB / virtual Serial Port.
For more frequent message uploading there is a version that stores text to be displayed into RAM memory of ATTINY. It allows unlimited cycles of writes so it can be used for automatic batched display of some news etc. 

In RAM version - you can connect LED banner to Raspberry Pi  and upload new text message from Raspberry every few seconds to RAM of Digispark :
- set the virtual USB to serial port speed to 9600 baud

chmod o+rw /dev/ttyUSB0

stty /dev/ttyUSB0 9600

or

stty -F /dev/ttyUSB0 9600 raw -echo -echoe -echok -echoctl -echoke

- send the message to be displayed on LED banner over serial port, ending with <CR> character ASCII 13
 
 echo -ne 'One line 80 characters message for LED display! \n' > /dev/ttyUSB0

The code uses DigiCDC USB library and you need to have necessary driver in your operationg system (Linux has it built in, for Windows you need to download Digispark drivers : http://digistump.com/board/index.php?topic=2321.15) - http://digistump.com/wiki/digispark/tutorials/digicdc.

 For Windows 10 (because of USB drivers changed in this OS in comparision to Windows 7)  - you need specialized driver that can be used only in testmode and requires uploading certificate https://github.com/protaskin/LowCDC-Win10x64

The MAX 7219 modules should be connected in daisy chain. The code supports up to 12 MAX 7219 modules connected in daisy chain but remember that your USB 5V has to be strong enough to power such number of LED displays. You may  declare number of modules used in the code within 
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
 


