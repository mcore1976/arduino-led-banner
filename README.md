# arduino-led-banner
This is simple LED BANNER constructed of Arduino Digispark and Daisy-chained set of MAX7219 LED 8x8 matrix. 
Allows displaying of 500 characters long text message that is stored in non-volatile EEPROM memory of the chip.

To upload text for display use putty terminal and connect to virtual Serial Por tof ATTINY85 Digispark ( /ttyACM0 on linux or COMxx on Windows )
using Putty or other serial terminal. Type the text using keyboard and press ENTER. It will be stored in EEPROM memory of ARDUINO.
After ARDUINO disconnected from the PC and connected to USB PowerBank, the device will display scrolling text uploaded from USB.

connections :



