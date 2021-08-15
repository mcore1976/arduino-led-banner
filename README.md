# arduino-led-banner
This is simple LED BANNER constructed of Arduino Digispark and Daisy-chained set of MAX7219 LED 8x8 matrix. 
Allows displaying of 500 characters long text message that is stored in non-volatile EEPROM memory of the chip.

To upload text for display use putty terminal and connect to virtual Serial Port of ATTINY85 Digispark ( /ttyACM0 on linux or COMxx on Windows ).
Type the text using keyboard and press ENTER. It will be stored in EEPROM memory of ARDUINO.
After ARDUINO disconnected from the PC and connected to USB PowerBank, the device will display scrolling text uploaded from USB.

The MAX 7219 modules should be connected in daisy chain. The code supports up to 30 MAX 7219 modules connected in daisy chain but remember that your USB 5V has to be strong enough to power such number of LED displays.  

connections :
 - Daisy chained  MAX7219 DIN PIN - Arduino Digispark  PB0 / P0 
 - Daisy chained  MAX7219 CS PIN  - Arduino Digispark  PB1 / P1 
 - Daisy chained  MAX7219 CLK PIN - Arduino Digispark  PB2 / P2
 - Daisy chained  MAX7219 VCC PIN - Arduino Digispark  5V pin
 - Daisy chained  MAX7219 GND PIN - Arduino Digispark  GND pin
 
 

 


