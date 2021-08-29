// ********************************************************************
// SCROLLING LED TICKER ARRAY from N 8x8 LED MAX7219 modules  
// connected to single ATTINY85 controlling chip
// (c) Adam Loboda '2021
// find me at adam.loboda@wp.pl
// 
// this version takes characters from USB and stores into non volatile
// EEPROM memory so you could use device without PC connectivity
// max text size is up to 500 characters
// 
// the message is scrolled from RIGHT to LEFT direction (arabic version)
//
// ********************************************************************

#include <stdint.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
// library for EEPROM access
#include  <avr/eeprom.h>
// library for serial USB
#include <DigiCDC.h>

// CONNECTIVITY BEETWEEN DIGISPARK AND MAX7219 DAISY CHAINED LED MATRIX
# define  MAX7219_DIN_PIN   PB0
# define  MAX7219_CS_PIN    PB1
# define  MAX7219_CLK_PIN   PB2

// declare number of letters in scrolled text here. Max is 500 letters
// due to EEPROM memory size in ATTINY85 Digispark
#define NUMCHARS          500
// declare number of daisy chained MAX7219 modules here. Max is 12 modules here
// due to small RAM amount in ATTINY 85, USB library takes a lot of RAM
#define MODULESNUMBER           12
// calculation of buffer length needed for displaying, because every font letter is 8 bytes long
#define BUFLENGTH   (MODULESNUMBER+1)*8


#define MAX7219_REG_NOOP                (0x00)
#define MAX7219_REG_DIGIT0              (0x01)
#define MAX7219_REG_DIGIT1              (0x02)
#define MAX7219_REG_DIGIT2              (0x03)
#define MAX7219_REG_DIGIT3              (0x04)
#define MAX7219_REG_DIGIT4              (0x05)
#define MAX7219_REG_DIGIT5              (0x06)
#define MAX7219_REG_DIGIT6              (0x07)
#define MAX7219_REG_DIGIT7              (0x08)
#define MAX7219_REG_DECODEMODE    (0x09)
#define MAX7219_REG_INTENSITY   (0x0A)
#define MAX7219_REG_SCANLIMIT   (0x0B)
#define MAX7219_REG_SHUTDOWN    (0x0C)
#define MAX7219_REG_DISPLAYTEST   (0x0F)

#define MAX7219_DIN_HIGH()              (PORTB |= _BV(MAX7219_DIN_PIN))
#define MAX7219_DIN_LOW()               (PORTB &= ~_BV(MAX7219_DIN_PIN))
#define MAX7219_CLK_HIGH()              (PORTB |= _BV(MAX7219_CLK_PIN))
#define MAX7219_CLK_LOW()               (PORTB &= ~_BV(MAX7219_CLK_PIN))
#define MAX7219_CS_HIGH()               (PORTB |= _BV(MAX7219_CS_PIN))
#define MAX7219_CS_LOW()                (PORTB &= ~_BV(MAX7219_CS_PIN))


// -----------------------------------------------------------------
// -----------------------------------------------------------------

// buffer for bitmap representaion of displayed text on LED modules
// you will have to extended lines with zeroes if you have more than 12 LED modules
static uint8_t inputbuf[BUFLENGTH] = 
   { 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 };

// RAM buffer for manipulation and displaying characters on LED displays
// you will have to extended lines with zeroes if you have more than 12 LED modules
static uint8_t outputbuf[BUFLENGTH] =
   { 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0, 
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0 };

// ASCII ROMAN character definition for CP437 font - 8 bytes (rows) per each character
// whole table does not fit into FLASH so it is cut down to half 0-127 ASCII chars


const uint8_t  fontdef[] PROGMEM = 
  { 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x00
   0x7E, 0x81, 0x95, 0xB1, 0xB1, 0x95, 0x81, 0x7E, // 0x01
   0x7E, 0xFF, 0xEB, 0xCF, 0xCF, 0xEB, 0xFF, 0x7E, // 0x02
   0x0E, 0x1F, 0x3F, 0x7E, 0x3F, 0x1F, 0x0E, 0x00, // 0x03
   0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08, 0x00, // 0x04
   0x18, 0xBA, 0xFF, 0xFF, 0xFF, 0xBA, 0x18, 0x00, // 0x05
   0x10, 0xB8, 0xFC, 0xFF, 0xFC, 0xB8, 0x10, 0x00, // 0x06
   0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00, // 0x07
   0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF, // 0x08
   0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00, // 0x09
   0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF, // 0x0A
   0x70, 0xF8, 0x88, 0x88, 0xFD, 0x7F, 0x07, 0x0F, // 0x0B
   0x00, 0x4E, 0x5F, 0xF1, 0xF1, 0x5F, 0x4E, 0x00, // 0x0C
   0xC0, 0xE0, 0xFF, 0x7F, 0x05, 0x05, 0x07, 0x07, // 0x0D
   0xC0, 0xFF, 0x7F, 0x05, 0x05, 0x65, 0x7F, 0x3F, // 0x0E
   0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99, // 0x0F
   0x7F, 0x3E, 0x3E, 0x1C, 0x1C, 0x08, 0x08, 0x00, // 0x10
   0x08, 0x08, 0x1C, 0x1C, 0x3E, 0x3E, 0x7F, 0x00, // 0x11
   0x00, 0x24, 0x66, 0xFF, 0xFF, 0x66, 0x24, 0x00, // 0x12
   0x00, 0x5F, 0x5F, 0x00, 0x00, 0x5F, 0x5F, 0x00, // 0x13
   0x06, 0x0F, 0x09, 0x7F, 0x7F, 0x01, 0x7F, 0x7F, // 0x14
   0x40, 0xDA, 0xBF, 0xA5, 0xFD, 0x59, 0x03, 0x02, // 0x15
   0x00, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x00, // 0x16
   0x80, 0x94, 0xB6, 0xFF, 0xFF, 0xB6, 0x94, 0x80, // 0x17
   0x00, 0x04, 0x06, 0x7F, 0x7F, 0x06, 0x04, 0x00, // 0x18
   0x00, 0x10, 0x30, 0x7F, 0x7F, 0x30, 0x10, 0x00, // 0x19
   0x08, 0x08, 0x08, 0x2A, 0x3E, 0x1C, 0x08, 0x00, // 0x1A
   0x08, 0x1C, 0x3E, 0x2A, 0x08, 0x08, 0x08, 0x00, // 0x1B
   0x3C, 0x3C, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, // 0x1C
   0x08, 0x1C, 0x3E, 0x08, 0x08, 0x3E, 0x1C, 0x08, // 0x1D
   0x30, 0x38, 0x3C, 0x3E, 0x3E, 0x3C, 0x38, 0x30, // 0x1E
   0x06, 0x0E, 0x1E, 0x3E, 0x3E, 0x1E, 0x0E, 0x06, // 0x1F
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ' '
   0x00, 0x06, 0x5F, 0x5F, 0x06, 0x00, 0x00, 0x00, // '!'
   0x00, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x00, // '"'
   0x14, 0x7F, 0x7F, 0x14, 0x7F, 0x7F, 0x14, 0x00, // '#'
   0x24, 0x2E, 0x6B, 0x6B, 0x3A, 0x12, 0x00, 0x00, // '$'
   0x46, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x62, 0x00, // '%'
   0x30, 0x7A, 0x4F, 0x5D, 0x37, 0x7A, 0x48, 0x00, // '&'
   0x04, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, // '''
   0x00, 0x1C, 0x3E, 0x63, 0x41, 0x00, 0x00, 0x00, // '('
   0x00, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00, 0x00, // ')'
   0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08, // '*'
   0x08, 0x08, 0x3E, 0x3E, 0x08, 0x08, 0x00, 0x00, // '+'
   0x00, 0x80, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00, // ','
   0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, // '-'
   0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, // '.'
   0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00, // '/'
   0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E, 0x00, // '0'
   0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00, 0x00, // '1'
   0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00, 0x00, // '2'
   0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00, // '3'
   0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x00, // '4'
   0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x00, // '5'
   0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00, 0x00, // '6'
   0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x00, // '7'
   0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00, // '8'
   0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x00, // '9'
   0x00, 0x00, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, // ':'
   0x00, 0x80, 0xE6, 0x66, 0x00, 0x00, 0x00, 0x00, // ';'
   0x08, 0x1C, 0x36, 0x63, 0x41, 0x00, 0x00, 0x00, // '<'
   0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00, 0x00, // '='
   0x00, 0x41, 0x63, 0x36, 0x1C, 0x08, 0x00, 0x00, // '>'
   0x02, 0x03, 0x51, 0x59, 0x0F, 0x06, 0x00, 0x00, // '?'
   0x3E, 0x7F, 0x41, 0x5D, 0x5D, 0x1F, 0x1E, 0x00, // '@'
   0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00, 0x00, // 'A'
   0x41, 0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, // 'B'
   0x1C, 0x3E, 0x63, 0x41, 0x41, 0x63, 0x22, 0x00, // 'C'
   0x41, 0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00, // 'D'
   0x41, 0x7F, 0x7F, 0x49, 0x5D, 0x41, 0x63, 0x00, // 'E'
   0x41, 0x7F, 0x7F, 0x49, 0x1D, 0x01, 0x03, 0x00, // 'F'
   0x1C, 0x3E, 0x63, 0x41, 0x51, 0x73, 0x72, 0x00, // 'G'
   0x7F, 0x7F, 0x08, 0x08, 0x7F, 0x7F, 0x00, 0x00, // 'H'
   0x00, 0x41, 0x7F, 0x7F, 0x41, 0x00, 0x00, 0x00, // 'I'
   0x30, 0x70, 0x40, 0x41, 0x7F, 0x3F, 0x01, 0x00, // 'J'
   0x41, 0x7F, 0x7F, 0x08, 0x1C, 0x77, 0x63, 0x00, // 'K'
   0x41, 0x7F, 0x7F, 0x41, 0x40, 0x60, 0x70, 0x00, // 'L'
   0x7F, 0x7F, 0x0E, 0x1C, 0x0E, 0x7F, 0x7F, 0x00, // 'M'
   0x7F, 0x7F, 0x06, 0x0C, 0x18, 0x7F, 0x7F, 0x00, // 'N'
   0x1C, 0x3E, 0x63, 0x41, 0x63, 0x3E, 0x1C, 0x00, // 'O'
   0x41, 0x7F, 0x7F, 0x49, 0x09, 0x0F, 0x06, 0x00, // 'P'
   0x1E, 0x3F, 0x21, 0x71, 0x7F, 0x5E, 0x00, 0x00, // 'Q'
   0x41, 0x7F, 0x7F, 0x09, 0x19, 0x7F, 0x66, 0x00, // 'R'
   0x26, 0x6F, 0x4D, 0x59, 0x73, 0x32, 0x00, 0x00, // 'S'
   0x03, 0x41, 0x7F, 0x7F, 0x41, 0x03, 0x00, 0x00, // 'T'
   0x7F, 0x7F, 0x40, 0x40, 0x7F, 0x7F, 0x00, 0x00, // 'U'
   0x1F, 0x3F, 0x60, 0x60, 0x3F, 0x1F, 0x00, 0x00, // 'V'
   0x7F, 0x7F, 0x30, 0x18, 0x30, 0x7F, 0x7F, 0x00, // 'W'
   0x43, 0x67, 0x3C, 0x18, 0x3C, 0x67, 0x43, 0x00, // 'X'
   0x07, 0x4F, 0x78, 0x78, 0x4F, 0x07, 0x00, 0x00, // 'Y'
   0x47, 0x63, 0x71, 0x59, 0x4D, 0x67, 0x73, 0x00, // 'Z'
   0x00, 0x7F, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00, // '['
   0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, // backslash
   0x00, 0x41, 0x41, 0x7F, 0x7F, 0x00, 0x00, 0x00, // ']'
   0x08, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x08, 0x00, // '^'
   0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, // '_'
   0x00, 0x00, 0x03, 0x07, 0x04, 0x00, 0x00, 0x00, // '`'
   0x20, 0x74, 0x54, 0x54, 0x3C, 0x78, 0x40, 0x00, // 'a'
   0x41, 0x7F, 0x3F, 0x48, 0x48, 0x78, 0x30, 0x00, // 'b'
   0x38, 0x7C, 0x44, 0x44, 0x6C, 0x28, 0x00, 0x00, // 'c'
   0x30, 0x78, 0x48, 0x49, 0x3F, 0x7F, 0x40, 0x00, // 'd'
   0x38, 0x7C, 0x54, 0x54, 0x5C, 0x18, 0x00, 0x00, // 'e'
   0x48, 0x7E, 0x7F, 0x49, 0x03, 0x02, 0x00, 0x00, // 'f'
   0x98, 0xBC, 0xA4, 0xA4, 0xF8, 0x7C, 0x04, 0x00, // 'g'
   0x41, 0x7F, 0x7F, 0x08, 0x04, 0x7C, 0x78, 0x00, // 'h'
   0x00, 0x44, 0x7D, 0x7D, 0x40, 0x00, 0x00, 0x00, // 'i'
   0x60, 0xE0, 0x80, 0x80, 0xFD, 0x7D, 0x00, 0x00, // 'j'
   0x41, 0x7F, 0x7F, 0x10, 0x38, 0x6C, 0x44, 0x00, // 'k'
   0x00, 0x41, 0x7F, 0x7F, 0x40, 0x00, 0x00, 0x00, // 'l'
   0x7C, 0x7C, 0x18, 0x38, 0x1C, 0x7C, 0x78, 0x00, // 'm'
   0x7C, 0x7C, 0x04, 0x04, 0x7C, 0x78, 0x00, 0x00, // 'n'
   0x38, 0x7C, 0x44, 0x44, 0x7C, 0x38, 0x00, 0x00, // 'o'
   0x84, 0xFC, 0xF8, 0xA4, 0x24, 0x3C, 0x18, 0x00, // 'p'
   0x18, 0x3C, 0x24, 0xA4, 0xF8, 0xFC, 0x84, 0x00, // 'q'
   0x44, 0x7C, 0x78, 0x4C, 0x04, 0x1C, 0x18, 0x00, // 'r'
   0x48, 0x5C, 0x54, 0x54, 0x74, 0x24, 0x00, 0x00, // 's'
   0x00, 0x04, 0x3E, 0x7F, 0x44, 0x24, 0x00, 0x00, // 't'
   0x3C, 0x7C, 0x40, 0x40, 0x3C, 0x7C, 0x40, 0x00, // 'u'
   0x1C, 0x3C, 0x60, 0x60, 0x3C, 0x1C, 0x00, 0x00, // 'v'
   0x3C, 0x7C, 0x70, 0x38, 0x70, 0x7C, 0x3C, 0x00, // 'w'
   0x44, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0x44, 0x00, // 'x'
   0x9C, 0xBC, 0xA0, 0xA0, 0xFC, 0x7C, 0x00, 0x00, // 'y'
   0x4C, 0x64, 0x74, 0x5C, 0x4C, 0x64, 0x00, 0x00, // 'z'
   0x08, 0x08, 0x3E, 0x77, 0x41, 0x41, 0x00, 0x00, // ''
   0x00, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x00, // '|'
   0x41, 0x41, 0x77, 0x3E, 0x08, 0x08, 0x00, 0x00, // '}'
   0x02, 0x03, 0x01, 0x03, 0x02, 0x03, 0x01, 0x00, // '~'
   
/*
   0x70, 0x78, 0x4C, 0x46, 0x4C, 0x78, 0x70, 0x00, // 0x7F
   0x0E, 0x9F, 0x91, 0xB1, 0xFB, 0x4A, 0x00, 0x00, // 0x80
   0x3A, 0x7A, 0x40, 0x40, 0x7A, 0x7A, 0x40, 0x00, // 0x81
   0x38, 0x7C, 0x54, 0x55, 0x5D, 0x19, 0x00, 0x00, // 0x82
   0x02, 0x23, 0x75, 0x55, 0x55, 0x7D, 0x7B, 0x42, // 0x83
   0x21, 0x75, 0x54, 0x54, 0x7D, 0x79, 0x40, 0x00, // 0x84
   0x21, 0x75, 0x55, 0x54, 0x7C, 0x78, 0x40, 0x00, // 0x85
   0x20, 0x74, 0x57, 0x57, 0x7C, 0x78, 0x40, 0x00, // 0x86
   0x18, 0x3C, 0xA4, 0xA4, 0xE4, 0x40, 0x00, 0x00, // 0x87
   0x02, 0x3B, 0x7D, 0x55, 0x55, 0x5D, 0x1B, 0x02, // 0x88
   0x39, 0x7D, 0x54, 0x54, 0x5D, 0x19, 0x00, 0x00, // 0x89
   0x39, 0x7D, 0x55, 0x54, 0x5C, 0x18, 0x00, 0x00, // 0x8A
   0x01, 0x45, 0x7C, 0x7C, 0x41, 0x01, 0x00, 0x00, // 0x8B
   0x02, 0x03, 0x45, 0x7D, 0x7D, 0x43, 0x02, 0x00, // 0x8C
   0x01, 0x45, 0x7D, 0x7C, 0x40, 0x00, 0x00, 0x00, // 0x8D
   0x79, 0x7D, 0x16, 0x12, 0x16, 0x7D, 0x79, 0x00, // 0x8E
   0x70, 0x78, 0x2B, 0x2B, 0x78, 0x70, 0x00, 0x00, // 0x8F
   0x44, 0x7C, 0x7C, 0x55, 0x55, 0x45, 0x00, 0x00, // 0x90
   0x20, 0x74, 0x54, 0x54, 0x7C, 0x7C, 0x54, 0x54, // 0x91
   0x7C, 0x7E, 0x0B, 0x09, 0x7F, 0x7F, 0x49, 0x00, // 0x92
   0x32, 0x7B, 0x49, 0x49, 0x7B, 0x32, 0x00, 0x00, // 0x93
   0x32, 0x7A, 0x48, 0x48, 0x7A, 0x32, 0x00, 0x00, // 0x94
   0x32, 0x7A, 0x4A, 0x48, 0x78, 0x30, 0x00, 0x00, // 0x95
   0x3A, 0x7B, 0x41, 0x41, 0x7B, 0x7A, 0x40, 0x00, // 0x96
   0x3A, 0x7A, 0x42, 0x40, 0x78, 0x78, 0x40, 0x00, // 0x97
   0x9A, 0xBA, 0xA0, 0xA0, 0xFA, 0x7A, 0x00, 0x00, // 0x98
   0x01, 0x19, 0x3C, 0x66, 0x66, 0x3C, 0x19, 0x01, // 0x99
   0x3D, 0x7D, 0x40, 0x40, 0x7D, 0x3D, 0x00, 0x00, // 0x9A
   0x18, 0x3C, 0x24, 0xE7, 0xE7, 0x24, 0x24, 0x00, // 0x9B
   0x68, 0x7E, 0x7F, 0x49, 0x43, 0x66, 0x20, 0x00, // 0x9C
   0x2B, 0x2F, 0xFC, 0xFC, 0x2F, 0x2B, 0x00, 0x00, // 0x9D
   0xFF, 0xFF, 0x09, 0x09, 0x2F, 0xF6, 0xF8, 0xA0, // 0x9E
   0x40, 0xC0, 0x88, 0xFE, 0x7F, 0x09, 0x03, 0x02, // 0x9F
   0x20, 0x74, 0x54, 0x55, 0x7D, 0x79, 0x40, 0x00, // 0xA0
   0x00, 0x44, 0x7D, 0x7D, 0x41, 0x00, 0x00, 0x00, // 0xA1
   0x30, 0x78, 0x48, 0x4A, 0x7A, 0x32, 0x00, 0x00, // 0xA2
   0x38, 0x78, 0x40, 0x42, 0x7A, 0x7A, 0x40, 0x00, // 0xA3
   0x7A, 0x7A, 0x0A, 0x0A, 0x7A, 0x70, 0x00, 0x00, // 0xA4
   0x7D, 0x7D, 0x19, 0x31, 0x7D, 0x7D, 0x00, 0x00, // 0xA5
   0x00, 0x26, 0x2F, 0x29, 0x2F, 0x2F, 0x28, 0x00, // 0xA6
   0x00, 0x26, 0x2F, 0x29, 0x2F, 0x26, 0x00, 0x00, // 0xA7
   0x30, 0x78, 0x4D, 0x45, 0x60, 0x20, 0x00, 0x00, // 0xA8
   0x38, 0x38, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, // 0xA9
   0x08, 0x08, 0x08, 0x08, 0x38, 0x38, 0x00, 0x00, // 0xAA
   0x4F, 0x6F, 0x30, 0x18, 0xCC, 0xEE, 0xBB, 0x91, // 0xAB
   0x4F, 0x6F, 0x30, 0x18, 0x6C, 0x76, 0xFB, 0xF9, // 0xAC
   0x00, 0x00, 0x00, 0x7B, 0x7B, 0x00, 0x00, 0x00, // 0xAD
   0x08, 0x1C, 0x36, 0x22, 0x08, 0x1C, 0x36, 0x22, // 0xAE
   0x22, 0x36, 0x1C, 0x08, 0x22, 0x36, 0x1C, 0x08, // 0xAF
   0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00, // 0xB0
   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, // 0xB1
   0xDD, 0xFF, 0xAA, 0x77, 0xDD, 0xAA, 0xFF, 0x77, // 0xB2
   0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, // 0xB3
   0x10, 0x10, 0x10, 0xFF, 0xFF, 0x00, 0x00, 0x00, // 0xB4
   0x14, 0x14, 0x14, 0xFF, 0xFF, 0x00, 0x00, 0x00, // 0xB5
   0x10, 0x10, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, // 0xB6
   0x10, 0x10, 0xF0, 0xF0, 0x10, 0xF0, 0xF0, 0x00, // 0xB7
   0x14, 0x14, 0x14, 0xFC, 0xFC, 0x00, 0x00, 0x00, // 0xB8
   0x14, 0x14, 0xF7, 0xF7, 0x00, 0xFF, 0xFF, 0x00, // 0xB9
   0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, // 0xBA
   0x14, 0x14, 0xF4, 0xF4, 0x04, 0xFC, 0xFC, 0x00, // 0xBB
   0x14, 0x14, 0x17, 0x17, 0x10, 0x1F, 0x1F, 0x00, // 0xBC
   0x10, 0x10, 0x1F, 0x1F, 0x10, 0x1F, 0x1F, 0x00, // 0xBD
   0x14, 0x14, 0x14, 0x1F, 0x1F, 0x00, 0x00, 0x00, // 0xBE
   0x10, 0x10, 0x10, 0xF0, 0xF0, 0x00, 0x00, 0x00, // 0xBF
   0x00, 0x00, 0x00, 0x1F, 0x1F, 0x10, 0x10, 0x10, // 0xC0
   0x10, 0x10, 0x10, 0x1F, 0x1F, 0x10, 0x10, 0x10, // 0xC1
   0x10, 0x10, 0x10, 0xF0, 0xF0, 0x10, 0x10, 0x10, // 0xC2
   0x00, 0x00, 0x00, 0xFF, 0xFF, 0x10, 0x10, 0x10, // 0xC3
   0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, // 0xC4
   0x10, 0x10, 0x10, 0xFF, 0xFF, 0x10, 0x10, 0x10, // 0xC5
   0x00, 0x00, 0x00, 0xFF, 0xFF, 0x14, 0x14, 0x14, // 0xC6
   0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x10, // 0xC7
   0x00, 0x00, 0x1F, 0x1F, 0x10, 0x17, 0x17, 0x14, // 0xC8
   0x00, 0x00, 0xFC, 0xFC, 0x04, 0xF4, 0xF4, 0x14, // 0xC9
   0x14, 0x14, 0x17, 0x17, 0x10, 0x17, 0x17, 0x14, // 0xCA
   0x14, 0x14, 0xF4, 0xF4, 0x04, 0xF4, 0xF4, 0x14, // 0xCB
   0x00, 0x00, 0xFF, 0xFF, 0x00, 0xF7, 0xF7, 0x14, // 0xCC
   0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, // 0xCD
   0x14, 0x14, 0xF7, 0xF7, 0x00, 0xF7, 0xF7, 0x14, // 0xCE
   0x14, 0x14, 0x14, 0x17, 0x17, 0x14, 0x14, 0x14, // 0xCF
   0x10, 0x10, 0x1F, 0x1F, 0x10, 0x1F, 0x1F, 0x10, // 0xD0
   0x14, 0x14, 0x14, 0xF4, 0xF4, 0x14, 0x14, 0x14, // 0xD1
   0x10, 0x10, 0xF0, 0xF0, 0x10, 0xF0, 0xF0, 0x10, // 0xD2
   0x00, 0x00, 0x1F, 0x1F, 0x10, 0x1F, 0x1F, 0x10, // 0xD3
   0x00, 0x00, 0x00, 0x1F, 0x1F, 0x14, 0x14, 0x14, // 0xD4
   0x00, 0x00, 0x00, 0xFC, 0xFC, 0x14, 0x14, 0x14, // 0xD5
   0x00, 0x00, 0xF0, 0xF0, 0x10, 0xF0, 0xF0, 0x10, // 0xD6
   0x10, 0x10, 0xFF, 0xFF, 0x10, 0xFF, 0xFF, 0x10, // 0xD7
   0x14, 0x14, 0x14, 0xFF, 0xFF, 0x14, 0x14, 0x14, // 0xD8
   0x10, 0x10, 0x10, 0x1F, 0x1F, 0x00, 0x00, 0x00, // 0xD9
   0x00, 0x00, 0x00, 0xF0, 0xF0, 0x10, 0x10, 0x10, // 0xDA
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xDB
   0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, // 0xDC
   0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // 0xDD
   0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, // 0xDE
   0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, // 0xDF
   0x38, 0x7C, 0x44, 0x6C, 0x38, 0x6C, 0x44, 0x00, // 0xE0
   0xFC, 0xFE, 0x2A, 0x2A, 0x3E, 0x14, 0x00, 0x00, // 0xE1
   0x7E, 0x7E, 0x02, 0x02, 0x06, 0x06, 0x00, 0x00, // 0xE2
   0x02, 0x7E, 0x7E, 0x02, 0x7E, 0x7E, 0x02, 0x00, // 0xE3
   0x63, 0x77, 0x5D, 0x49, 0x63, 0x63, 0x00, 0x00, // 0xE4
   0x38, 0x7C, 0x44, 0x7C, 0x3C, 0x04, 0x04, 0x00, // 0xE5
   0x80, 0xFE, 0x7E, 0x20, 0x20, 0x3E, 0x1E, 0x00, // 0xE6
   0x04, 0x06, 0x02, 0x7E, 0x7C, 0x06, 0x02, 0x00, // 0xE7
   0x99, 0xBD, 0xE7, 0xE7, 0xBD, 0x99, 0x00, 0x00, // 0xE8
   0x1C, 0x3E, 0x6B, 0x49, 0x6B, 0x3E, 0x1C, 0x00, // 0xE9
   0x4C, 0x7E, 0x73, 0x01, 0x73, 0x7E, 0x4C, 0x00, // 0xEA
   0x30, 0x78, 0x4A, 0x4F, 0x7D, 0x39, 0x00, 0x00, // 0xEB
   0x18, 0x3C, 0x24, 0x3C, 0x3C, 0x24, 0x3C, 0x18, // 0xEC
   0x98, 0xFC, 0x64, 0x3C, 0x3E, 0x27, 0x3D, 0x18, // 0xED
   0x1C, 0x3E, 0x6B, 0x49, 0x49, 0x00, 0x00, 0x00, // 0xEE
   0x7E, 0x7F, 0x01, 0x01, 0x7F, 0x7E, 0x00, 0x00, // 0xEF
   0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x00, 0x00, // 0xF0
   0x44, 0x44, 0x5F, 0x5F, 0x44, 0x44, 0x00, 0x00, // 0xF1
   0x40, 0x51, 0x5B, 0x4E, 0x44, 0x40, 0x00, 0x00, // 0xF2
   0x40, 0x44, 0x4E, 0x5B, 0x51, 0x40, 0x00, 0x00, // 0xF3
   0x00, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x07, 0x06, // 0xF4
   0x60, 0xE0, 0x80, 0xFF, 0x7F, 0x00, 0x00, 0x00, // 0xF5
   0x08, 0x08, 0x6B, 0x6B, 0x08, 0x08, 0x00, 0x00, // 0xF6
   0x24, 0x36, 0x12, 0x36, 0x24, 0x36, 0x12, 0x00, // 0xF7
   0x00, 0x06, 0x0F, 0x09, 0x0F, 0x06, 0x00, 0x00, // 0xF8
   0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, // 0xF9
   0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, // 0xFA
   0x10, 0x30, 0x70, 0xC0, 0xFF, 0xFF, 0x01, 0x01, // 0xFB
   0x00, 0x1F, 0x1F, 0x01, 0x1F, 0x1E, 0x00, 0x00, // 0xFC
   0x00, 0x19, 0x1D, 0x17, 0x12, 0x00, 0x00, 0x00, // 0xFD
   0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, // 0xFE
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xFF

   */
};


//8x8 Persian Letters - taken from https://github.com/idreamsi
//This is the font definition.
//You can use http://gurgleapps.com/tools/matrix to create your own font.
/*
const uint8_t  fontdef[] PROGMEM = {
  0x00, 0x1C, 0x20, 0x08, 0x08, 0x08, 0x08, 0x00, //0 alef1
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, //1 alef2 
  0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, //2 alef3
  0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x00, 0x02, //3 be1
  0x00, 0x00, 0x40, 0x81, 0x81, 0x7E, 0x00, 0x10, //4 be2
  0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x00, 0x07, //5 pe1
  0x00, 0x00, 0x40, 0x81, 0x81, 0x7E, 0x00, 0x38, //6 pe2
  0x00, 0x03, 0x00, 0x01, 0x01, 0x06, 0x00, 0x00, //7 te1
  0x00, 0x14, 0x40, 0x81, 0x81, 0x7E, 0x00, 0x00, //8 te2
  0x02, 0x05, 0x00, 0x01, 0x01, 0x06, 0x00, 0x00, //9 the1
  0x08, 0x14, 0x40, 0x81, 0x81, 0x7E, 0x00, 0x00, //10  the2
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x00, 0x04, //11  jim1
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x40, 0x3A, //12  jim2
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x00, 0x1C, //13  che1
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x40, 0x37, //14  che2
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x00, 0x00, //15  hee1
  0x00, 0x00, 0x0C, 0x12, 0x01, 0x3E, 0x40, 0x38, //16  hee2
  0x00, 0x20, 0x0C, 0x12, 0x01, 0x3E, 0x00, 0x00, //17  khe1
  0x00, 0x20, 0x0C, 0x12, 0x01, 0x3E, 0x40, 0x38, //18  khe2
  0x00, 0x00, 0x04, 0x02, 0x01, 0x12, 0x0C, 0x00, //19  dal
  0x00, 0x10, 0x04, 0x02, 0x01, 0x12, 0x0C, 0x00, //20  zal
  0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x0C, //21  re
  0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x02, 0x0C, //22  ze
  0x02, 0x05, 0x00, 0x01, 0x01, 0x01, 0x02, 0x0C, //23  zhe
  0x00, 0x00, 0x00, 0x15, 0x15, 0x6A, 0x00, 0x00, //24  sin1
  0x00, 0x00, 0x00, 0x15, 0x95, 0x9A, 0x90, 0x60, //25  sin2
  0x04, 0x0A, 0x00, 0x15, 0x15, 0x6A, 0x00, 0x00, //26  shin1
  0x04, 0x0A, 0x00, 0x15, 0x95, 0x9A, 0x90, 0x60, //27  shin2
  0x00, 0x00, 0x06, 0x29, 0x31, 0x5E, 0x00, 0x00, //28  sad1
  0x00, 0x00, 0x06, 0x49, 0x91, 0x9E, 0x90, 0x60, //29  sad2
  0x00, 0x10, 0x06, 0x29, 0x31, 0x5E, 0x00, 0x00, //30  zad1
  0x00, 0x10, 0x06, 0x49, 0x91, 0x9E, 0x90, 0x60, //31  zad2
  0x00, 0x20, 0x26, 0x29, 0x31, 0x7E, 0x00, 0x00, //32  taa
  0x00, 0x28, 0x26, 0x29, 0x31, 0x7E, 0x00, 0x00, //33  zaa
  0x00, 0x00, 0x03, 0x04, 0x04, 0x0F, 0x00, 0x00, //34  ein1
  0x00, 0x00, 0x1E, 0x22, 0x1C, 0x77, 0x00, 0x00, //35  ein2
  0x00, 0x1E, 0x22, 0x1C, 0x24, 0x23, 0x20, 0x1C, //36  ein3
  0x00, 0x03, 0x04, 0x04, 0x0F, 0x10, 0x10, 0x0F, //37  ein4
  0x00, 0x08, 0x03, 0x04, 0x04, 0x0F, 0x00, 0x00, //38  qein1
  0x08, 0x00, 0x1E, 0x22, 0x1C, 0x77, 0x00, 0x00, //39  qein2
  0x40, 0x1E, 0x22, 0x1C, 0x24, 0x23, 0x20, 0x1C, //40  qein3
  0x08, 0x03, 0x04, 0x04, 0x0F, 0x10, 0x10, 0x0F, //41  qein4
  0x04, 0x00, 0x06, 0x09, 0x09, 0x1E, 0x00, 0x00, //42  fe1
  0x04, 0x00, 0x06, 0x89, 0x89, 0x7E, 0x00, 0x00, //43  fe2
  0x06, 0x00, 0x06, 0x09, 0x09, 0x1E, 0x00, 0x00, //44  qaf1
  0x06, 0x00, 0x06, 0x09, 0x49, 0x47, 0x41, 0x3E, //45  qaf2
  0x00, 0x0F, 0x10, 0x1E, 0x01, 0x3E, 0x00, 0x00, //46  kaf1
  0x00, 0x07, 0x08, 0x8E, 0x81, 0x7E, 0x00, 0x00, //47  kaf2
  0x07, 0x0F, 0x10, 0x1E, 0x01, 0x3E, 0x00, 0x00, //48  gaf1
  0x07, 0x0F, 0x10, 0x9E, 0x81, 0x7E, 0x00, 0x00, //49  gaf2
  0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, //50  lam1  
  0x00, 0x01, 0x01, 0x11, 0x21, 0x22, 0x1C, 0x00, //51  lam2
  0x00, 0x00, 0x06, 0x09, 0x09, 0x36, 0x00, 0x00, //52  mim1  
  0x00, 0x04, 0x0A, 0x19, 0x29, 0x46, 0x40, 0x20, //53  mim2
  0x00, 0x02, 0x00, 0x01, 0x01, 0x06, 0x00, 0x00, //54  noon1
  0x00, 0x00, 0x08, 0x21, 0x41, 0x41, 0x22, 0x1C, //55  noon2
  0x00, 0x00, 0x06, 0x09, 0x09, 0x07, 0x01, 0x0E, //56  vaav
  0x08, 0x04, 0x0A, 0x15, 0x09, 0x36, 0x00, 0x00, //57  he1
  0x00, 0x00, 0x30, 0x48, 0x56, 0xE9, 0x50, 0x20, //58  he2
  0x00, 0x07, 0x09, 0x09, 0x05, 0x00, 0x00, 0x00, //59  he3
  0x00, 0x08, 0x04, 0x0A, 0x11, 0x11, 0x0E, 0x00, //60  he4
  0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x00, 0x06, //61  ye1
  0x00, 0x00, 0x00, 0x4C, 0x92, 0x89, 0x84, 0x78, //62  ye2
  0x00, 0x06, 0x29, 0x48, 0x46, 0x41, 0x21, 0x1E, //63  ye3
  0x0C, 0x12, 0x10, 0x08, 0x04, 0x00, 0x04, 0x00, //64  soal
  0x00, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00, //65  tajob
  0x00, 0x00, 0x00, 0x04, 0x08, 0x0C, 0x0C, 0x00, //66  vir
  0x10, 0x08, 0x04, 0x04, 0x04, 0x04, 0x08, 0x10, //67  kmn1
  0x04, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08, 0x04, //68  kmn2
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, //69  slash
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 //70  backslash
};
*/


// -----------------------------------------------------------------------------------------
// -------- Modified MAX7219 procedures to handle 'N' character daisy chain set ------------
// ---------Enable CS signal, then send 'N' commands , then disable CS signal   ------------
// -----------------------------------------------------------------------------------------

// writes a single byte to MAX7219 using DIN & CLK line
void
MAX7219_write(uint8_t value)
{
        uint8_t i;

        __asm("nop");
        for (i = 0; i < 8; ++i, value <<= 1) {
                MAX7219_CLK_LOW();
                __asm("nop");
                if (value & 0x80) {
                        MAX7219_DIN_HIGH();
                } else {
                        MAX7219_DIN_LOW();
                }
                MAX7219_CLK_HIGH();
        }
}


// sends the 2 byte command from table 'commands' to set of N modules MAX7219 connected in the same time
void
MAX7219_sendN(uint8_t N, uint8_t commands[])
{
        uint8_t  i;

        MAX7219_CS_HIGH();
        for (i=0; i<N; i++)
        {   
           MAX7219_write(commands[(i*2)]);
           MAX7219_write(commands[(i*2)+1]);
        };
        MAX7219_CS_LOW();
        __asm("nop");
        MAX7219_CS_HIGH();
};



// initiates set of 'N' MAX7219 modules at once
void
MAX7219_initN(uint8_t N)
{
  uint8_t commands[MODULESNUMBER * 2];
        uint8_t  i ; 

  DDRB |= _BV(MAX7219_DIN_PIN)|_BV(MAX7219_CLK_PIN)|_BV(MAX7219_CS_PIN);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_DECODEMODE;
           commands[(i*2)+1] = 0x00;
        };
        MAX7219_sendN(N, commands);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_SCANLIMIT;
           commands[(i*2)+1] = 0x07;
        };
        MAX7219_sendN(N, commands);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_INTENSITY;
           commands[(i*2)+1] = 0x0f;
        };
        MAX7219_sendN(N, commands);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_DISPLAYTEST;
           commands[(i*2)+1] = 0x00;
        };
        MAX7219_sendN(N, commands);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_SHUTDOWN;
           commands[(i*2)+1] = 0x01;
        };
        MAX7219_sendN(N, commands);

        for (i=0; i<N; i++)
        {   
           commands[(i*2)] = MAX7219_REG_INTENSITY;
           commands[(i*2)+1] = 8;
        };
        MAX7219_sendN(N, commands);

};

// -----------------------------------------------------------------
// -----------------------------------------------------------------


void setup() { 

     // initialize set of 6 daisy chained modules with MAX7219
     MAX7219_initN(MODULESNUMBER);

     // initialize SerialUSB library 
     SerialUSB.begin();
}


// -----------------------------------------------------------------
// -------------------------------- MAIN CODE ----------------------
// -----------------------------------------------------------------


void loop() {
  // put your main code here, to run repeatedly:
     uint8_t  row, i, j, k;                   // for bitmap manipulation
     uint8_t  letter;                         // current character taken from defined text
     uint16_t offset;                         // offset for current character in flash memory
     uint16_t offset2;                        // offset for columns 
     uint16_t offset3;                        // location of current char from USB reading
     uint16_t textsize;                       // size of text in USB characters buffer
     uint16_t currentstart;                   // offset for current character in flash memory of the chip
     uint8_t  commands[MODULESNUMBER * 2];
 
     // set starting position for USB char reading
     offset3 = 0;
     // read old sie of text message from EEPROM settings (last two bytes 509 & 510 of EEPROM)
     textsize  = eeprom_read_byte((uint8_t*)509) + eeprom_read_byte((uint8_t*)510);

     // neverending loop for scrolling
     while (1)
     {

      
     // loop for scrolling 1 times - as many times as NUMCHARS !
     // currentstart = 0;

     // text is scrolled from LEFT to RIGHT
      currentstart = ( textsize - MODULESNUMBER);

      
     // while ( currentstart <  ( textsize - MODULESNUMBER)  )
       while ( currentstart > 0  )
       {
            // convert 'sentence' characters into bitmaps 8x8 by program memory 
            // table lookup and put it into buffer for further manipulation
            // we are converting only MODULESNUMBER+1 characters to conserve SRAM 
            // just for single character scroll and then we shift offset
            for(j = 0; j<(MODULESNUMBER + 1); j++)
                   {  
                   // read actual ASCII code of letter from FLASH memory of the chip 
                   // letter = pgm_read_byte(sentence + j + currentstart);
                   // read actual ASCII code of letter from RAM memory of the chip    
                   // letter = sentence [ j + currentstart ];    
                   // read actual ASCII code of letter from EEPROM memory of the chip
                   letter = eeprom_read_byte((uint8_t*)(j + currentstart));

                   // copy 8 rows of this ASCII letter bitmap to buffer      
                   for(i=0; i<8; i++)
                        { 
                          inputbuf[ (j*8) + i ] = pgm_read_byte(fontdef + (letter*8) + i ) ;
                        };  // end of 'i' loop
                   };  // end of 'j' loop


            //  Scroll by 8 columns - single character scroll
            //  From RIGHT to LEFT direction
            // offset2 = 0; 
            offset2 = 8; 


            // while (offset2 < 8)
            while (offset2 > 0)
              {
                      // convert rows of N displays into columns ( 90 degrees rotation )
                      for (letter=0; letter< MODULESNUMBER; letter++)
                      {
                       offset = letter * 8;  // ofset to current character in 8x8 font matrix
           
                         // transpose single 8x8bit font matrix by 90 degrees
                         for(int i = 0; i < 8; i++) 
                           {
                              for(int j = 7; j > -1; j--) 
                                {
                                   outputbuf[i+offset] = (outputbuf[i+offset] << 1) | ((inputbuf[j+offset+offset2] >> (7 - i)) & 0x01);
                                };  // end of 'j' loop
                            }; // end of 'i'loop
                      
                       }; // end of 'letter' loop 
                 

                     // sending 8 rows data to N character display from 'outputbuf' array       
                      for(row=0; row<8; row++)
                      { 
                      // send bitmap buffer content to N modules at once, first module is last in sequence 
                         for (k=0; k < MODULESNUMBER ; k++)
                            {   
                               commands[(k*2)] = row+1;
                               commands[(k*2)+1] = outputbuf[row + ((MODULESNUMBER - k -1 )*8)];
                            };  // end of 'k' loop
                         MAX7219_sendN(MODULESNUMBER, commands);
                       };  // end of 'row' loop

                 // increase current column for scrolling
                 // offset2++; 

                 // decrease current column for scrolling
                  offset2--; 

               // HERE YOU ADJUST SPEED OF THE SCROLL !!!
              SerialUSB.delay(25);


         // check if there is any character from USB
         // if so put it to the text buffer for display 
           if (SerialUSB.available()) {
            
               uint8_t usbinput = SerialUSB.read();        // read character from USB serial port
               
               SerialUSB.write(usbinput);                  // write an ECHO on terminal
                  
               if (usbinput == 13)  
               {  SerialUSB.write(10);                     // send CRLF in terminal
               
                  textsize = offset3;                      // text from USB is complete

                  // save new length of text into EEPROM variables
                  if (offset3 > 255) 
                  {
                      eeprom_write_byte((uint8_t*)509, 255);
                      eeprom_write_byte((uint8_t*)510, (uint8_t)(offset3 - 255) );                      
                  }
                  else 
                  { 
                      eeprom_write_byte((uint8_t*)509, 0);
                      eeprom_write_byte((uint8_t*)510, (uint8_t)offset3 );                      
                  };
                  
                  // fill text table with zeros up to end of it
                  for (offset3; offset3<NUMCHARS; offset3++)   eeprom_write_byte((uint8_t*)offset3, 0x00); 
                  offset3  = 0;                            // resume to start position
               }
               // save character read from USB into  EEPROM
               // non volatile memory at 'offset3' address 
               else   eeprom_write_byte((uint8_t*)offset3,usbinput); 

               // increment offset for EEPROM char writting 
               offset3++;                                  
               if (offset3 == NUMCHARS) offset3 = 0;
               
           };   // end of handling Serial USB reading characters and saving EEPROM

         }; // end of 'offset2' WHILE LOOP for scrolling single character

          // increase current position on text to scroll  
          // currentstart++;  

         // decrease current position on text to scroll  - scroll LEFT to RIGHT direction
          currentstart--;  

   
       };// end of LOOP for 'currentstart'  for scrolling all the text

     }; // END OF NEVERENDING LOOP

}