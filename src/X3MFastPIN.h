#pragma once
#include <Arduino.h>

class X3MfastPIN {
public:
bool digiRead(uint8_t pin);                   // быстрое чтение состояния пинов на Arduino
void digiWrite(uint8_t pin, bool x);          // быстрая запись состояния пинов на Arduino
};

#if defined ArduinoMEGA
void X3MfastPIN::digiWrite(uint8_t pin, bool x) {digitalWrite(pin,x);}
#else
void X3MfastPIN::digiWrite(uint8_t pin, bool x) {switch (pin) {           
case 3: bitClear(TCCR2A, COM2B1); break;
case 5: bitClear(TCCR0A, COM0B1); break;
case 6: bitClear(TCCR0A, COM0A1); break;
case 9: bitClear(TCCR1A, COM1A1); break;
case 10: bitClear(TCCR1A, COM1B1); break;
case 11: bitClear(TCCR2A, COM2A1); break;}  // PWM disable 
if(pin<8) {bitWrite(PORTD, pin, x);} else if(pin<14) {bitWrite(PORTB, (pin-8), x);} else if (pin<20) {bitWrite(PORTC, (pin-14), x);}}    // Set pin to HIGH / LOW 
#endif

#if defined ArduinoMEGA
bool X3MfastPIN::digiRead(uint8_t pin) {
switch (pin) {
case 0: return bitRead(PINE, 0); break;
case 1: return bitRead(PINE, 1); break;
case 2: return bitRead(PINE, 4); break;
case 3: return bitRead(PINE, 5); break;
case 4: return bitRead(PING, 5); break;
case 5: return bitRead(PINE, 3); break;
case 6: return bitRead(PINH, 3); break;
case 7: return bitRead(PINH, 4); break;
case 8: return bitRead(PINH, 5); break;
case 9: return bitRead(PINH, 6); break;
case 10: return bitRead(PINB, 4); break;
case 11: return bitRead(PINB, 5); break;
case 12: return bitRead(PINB, 6); break;
case 13: return bitRead(PINB, 7); break;
case 14: return bitRead(PINJ, 1); break;
case 15: return bitRead(PINJ, 0); break;
case 16: return bitRead(PINH, 1); break;
case 17: return bitRead(PINH, 0); break;
case 18: return bitRead(PIND, 3); break;
case 19: return bitRead(PIND, 2); break;
case 20: return bitRead(PIND, 1); break;
case 21: return bitRead(PIND, 0); break;
case 22: return bitRead(PINA, 0); break;
case 23: return bitRead(PINA, 1); break;
case 24: return bitRead(PINA, 2); break;
case 25: return bitRead(PINA, 3); break;
case 26: return bitRead(PINA, 4); break;
case 27: return bitRead(PINA, 5); break;
case 28: return bitRead(PINA, 6); break;
case 29: return bitRead(PINA, 7); break;
case 30: return bitRead(PINC, 7); break;
case 31: return bitRead(PINC, 6); break;
case 32: return bitRead(PINC, 5); break;
case 33: return bitRead(PINC, 4); break;
case 34: return bitRead(PINC, 3); break;
case 35: return bitRead(PINC, 2); break;
case 36: return bitRead(PINC, 1); break;
case 37: return bitRead(PINC, 0); break;
case 38: return bitRead(PIND, 7); break;
case 39: return bitRead(PING, 2); break;
case 40: return bitRead(PING, 1); break;
case 41: return bitRead(PING, 0); break;
case 42: return bitRead(PINL, 7); break;
case 43: return bitRead(PINL, 6); break;
case 44: return bitRead(PINL, 5); break;
case 45: return bitRead(PINL, 4); break;
case 46: return bitRead(PINL, 3); break;
case 47: return bitRead(PINL, 2); break;
case 48: return bitRead(PINL, 1); break;
case 49: return bitRead(PINL, 0); break;
case 50: return bitRead(PINB, 3); break;
case 51: return bitRead(PINB, 2); break;
case 52: return bitRead(PINB, 1); break;
case 53: return bitRead(PINB, 0); break;
case A0: return bitRead(PINF, 0); break;
case A1: return bitRead(PINF, 1); break;
case A2: return bitRead(PINF, 2); break;
case A3: return bitRead(PINF, 3); break;
case A4: return bitRead(PINF, 4); break;
case A5: return bitRead(PINF, 5); break;
case A6: return bitRead(PINF, 6); break;
case A7: return bitRead(PINF, 7); break;
case 62: return bitRead(PINK, 0); break;
case 63: return bitRead(PINK, 1); break;
case 64: return bitRead(PINK, 2); break;
case 65: return bitRead(PINK, 3); break;
case 66: return bitRead(PINK, 4); break;
case 67: return bitRead(PINK, 5); break;
case 68: return bitRead(PINK, 6); break;
case 69: return bitRead(PINK, 7); break;
default: return 0;}}
#else
bool X3MfastPIN::digiRead(uint8_t pin) {if(pin<8) return bitRead(PIND, pin); else if(pin < 14) return bitRead(PINB, pin-8); else if(pin<20) return bitRead(PINC, pin-14); return 0;}
#endif