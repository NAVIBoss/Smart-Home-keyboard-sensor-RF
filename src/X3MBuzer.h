#pragma once
#include <Arduino.h>

class X3MBuzer {
public:
X3MBuzer(uint8_t in);
void go(uint8_t in);
void en();
void dis();

private:
uint8_t mode, sound, BuzerPIN;
boolean Zumer;
uint32_t timeWait, timeBUZ;
void digiWrite(uint8_t pin, bool x);
void BUZ(uint8_t in) {digiWrite(BuzerPIN,!in);}
};

X3MBuzer::X3MBuzer(uint8_t in) {BuzerPIN=in; pinMode(in, OUTPUT); digitalWrite(in,1);}

#if defined ArduinoMEGA
void X3MBuzer::digiWrite(uint8_t pin, bool x) {digitalWrite(pin,x);}
#else
void X3MBuzer::digiWrite(uint8_t pin, bool x) {switch (pin) {           
case 3: bitClear(TCCR2A, COM2B1); break;
case 5: bitClear(TCCR0A, COM0B1); break;
case 6: bitClear(TCCR0A, COM0A1); break;
case 9: bitClear(TCCR1A, COM1A1); break;
case 10: bitClear(TCCR1A, COM1B1); break;
case 11: bitClear(TCCR2A, COM2A1); break;}  // PWM disable 
if(pin<8) {bitWrite(PORTD, pin, x);} else if(pin<14) {bitWrite(PORTB, (pin-8), x);} else if (pin<20) {bitWrite(PORTC, (pin-14), x);}}    // Set pin to HIGH / LOW 
#endif

void X3MBuzer::en() {Zumer=1;}
void X3MBuzer::dis() {Zumer=0;}

void X3MBuzer::go(uint8_t in=0) {static uint8_t nextSound;
if((!Zumer && in!=16 && !mode) || (!in && !mode)) return; if(in && mode) {nextSound=in; CM("Save sound: "); CMn(in);}
if (!in && millis()-timeBUZ<timeWait) return; timeBUZ=millis(); if(in && !mode) {mode=1; sound=in; CM("-> Sound: "); CMn(sound);}
switch (sound) {
case 1: switch (mode) { // Клик кнопки
case 1: BUZ(1); timeWait=15; break;
case 2: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 2: switch (mode) { // Удержание кнопки
case 1: case 3: BUZ(1); timeWait=15; break;
case 2: BUZ(0); timeWait=40; break;
case 4: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 3: switch (mode) { // Режим программирования нажатия кнопок
case 1: BUZ(1); timeWait=50; break;
case 2: timeWait=30; break;
case 4: case 6: case 8: BUZ(0); timeWait=20; break;
case 5: case 7: case 9: BUZ(1); timeWait=5; break;
case 10: BUZ(0); timeWait=20; break;
case 11: BUZ(1); timeWait=50; break;
case 12: BUZ(0); timeWait=30; break;
case 13: BUZ(1); timeWait=70; break;
case 14: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 4: switch (mode) { // Режим тренировки удержания кнопок
case 1: BUZ(1); timeWait=50; break;
case 2: timeWait=30; break;
case 4: case 6: case 8: case 10: case 12:BUZ(0); timeWait=20; break;
case 5: case 7: case 9: case 11: case 13: BUZ(1); timeWait=5; break;
case 14: BUZ(0); timeWait=20; break;
case 15: BUZ(1); timeWait=50; break;
case 16: BUZ(0); timeWait=30; break;
case 17: BUZ(1); timeWait=70; break;
case 18: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 5: switch (mode) { // Режим программирования кнопок пульта
case 1: BUZ(1); timeWait=50; break;
case 2: timeWait=30; break;
case 4: case 6: case 8: BUZ(0); timeWait=20; break;
case 5: case 7: case 9: BUZ(1); timeWait=5; break;
case 10: BUZ(0); timeWait=20; break;
case 11: BUZ(1); timeWait=50; break;
case 12: timeWait=70; break;
case 13: case 15: case 17: BUZ(0); timeWait=20; break;
case 14: case 16: case 18: BUZ(1); timeWait=5; break;
case 19: BUZ(0); timeWait=20; break;
case 20: BUZ(1); timeWait=50; break;
case 21: BUZ(0); timeWait=30; break;
case 22: BUZ(1); timeWait=70; break;
case 23: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 6: switch (mode) { // Программирование успешно
case 1: BUZ(1); timeWait=70; break;
case 2: BUZ(0); timeWait=160; break;
case 3: case 5: case 7: BUZ(0); timeWait=20; break;
case 4: case 6: BUZ(1); timeWait=5; break;
case 8: BUZ(0); timeWait=70; break;
case 9: BUZ(1); timeWait=180; break;
case 10: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 7: switch (mode) { // Отмена режима программирования
case 1: BUZ(1); timeWait=100; break;
case 3: case 5: BUZ(1); timeWait=100; break;
case 2: case 4: case 6: BUZ(0); timeWait=15; break;
case 7: BUZ(1); timeWait=500; break;
case 8: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 8: switch (mode) { // Скоро выход из режима программирования (удержание одной кнопки)
case 1: BUZ(1); timeWait=4; break;
case 2: case 4: case 6: BUZ(1); timeWait=2; break;
case 8: case 10: BUZ(1); timeWait=1; break;
case 3: case 5: case 7: case 9: case 11: BUZ(0); timeWait=15; break;
case 12: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 9: switch (mode) { // Запрос нажатия следующей кнопки (программирование)
case 1: BUZ(1); timeWait=15; break;
case 3: BUZ(1); timeWait=15; break;
case 2: BUZ(0); timeWait=40; break;
case 4: BUZ(0); timeWait=100; break;
case 5: BUZ(1); timeWait=70; break;
case 6: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 10: switch (mode) { // Ожидание нажатия кнопки
case 1: BUZ(1); timeWait=15; break;
case 3: case 5: case 7: case 9: BUZ(1); timeWait=15; break;
case 2: case 6: case 10: BUZ(0); timeWait=40; break;
case 4: case 8: BUZ(0); timeWait=100; break;
case 11: BUZ(1); timeWait=70; break;
case 12: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 11: switch (mode) { // Отмена таймера выхода из режима программирования (отпустили раньше)
case 1: BUZ(1); timeWait=0; break;
case 2: BUZ(0); timeWait=15; break;
case 3: BUZ(1); timeWait=2; break;
case 5: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 12: switch (mode) { // Скоро выход из режима программирования (удержание одной кнопки)
case 1: BUZ(1); timeWait=3; break;
case 2: BUZ(0); timeWait=15; break;
case 3: BUZ(1); timeWait=0; break;
case 4: BUZ(0); timeWait=15; break;
case 5: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 13: switch (mode) { // Включить звук
case 1: BUZ(1); timeWait=0; break;
case 2: case 4: BUZ(0); timeWait=15; break;
case 3: case 5: BUZ(1); timeWait=2; break;
case 6: BUZ(0); timeWait=15; break;
case 7: BUZ(1); timeWait=5; break;
case 8: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 14: switch (mode) { // Выключить звук
case 1: BUZ(1); timeWait=0; break;
case 2: case 4: BUZ(1); timeWait=1; break;
case 8: BUZ(1); timeWait=0; break;
case 3: case 5: case 7: case 9: BUZ(0); timeWait=5; break;
case 10: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 15: switch (mode) { // Удаление всех пультов
case 1: BUZ(1); timeWait=4; break;
case 2: case 4: case 6: BUZ(1); timeWait=2; break;
case 8: case 10: BUZ(1); timeWait=1; break;
case 3: case 5: case 7: case 9: case 11: BUZ(0); timeWait=15; break;
case 12: BUZ(1); timeWait=50; break;
case 13: case 15: case 17: BUZ(1); timeWait=2; break;
case 19: case 21: BUZ(1); timeWait=1; break;
case 14: case 16: case 18: case 20: case 22: BUZ(0); timeWait=4; break;
case 23: BUZ(1); timeWait=4; break;
case 24: BUZ(0); timeWait=50; break;
case 25: BUZ(1); timeWait=50; break;
case 26: case 28: case 30: BUZ(1); timeWait=0; break;
case 32: case 34: BUZ(1); timeWait=2; break;
case 27: case 29: case 31: case 33: case 35: BUZ(0); timeWait=15; break;
case 36: BUZ(1); timeWait=4; break;
case 37: BUZ(0); timeWait=50; break;
case 38: case 40: case 42: BUZ(1); timeWait=0; break;
case 44: case 46: BUZ(1); timeWait=2; break;
case 39: case 41: case 43: case 45: case 47: BUZ(0); timeWait=4; break;
case 48: BUZ(1); timeWait=4; break;
case 49: BUZ(0); timeWait=50; break;
case 50: BUZ(1); timeWait=4; break;
case 51: case 53: case 55: BUZ(1); timeWait=2; break;
case 57: case 59: BUZ(1); timeWait=1; break;
case 52: case 54: case 56: case 58: case 60: BUZ(0); timeWait=15; break;
case 61: BUZ(1); timeWait=50; break;
case 62: case 64: case 66: BUZ(1); timeWait=2; break;
case 68: case 70: BUZ(1); timeWait=1; break;
case 63: case 65: case 67: case 69: case 71: BUZ(0); timeWait=4; break;
case 72: BUZ(1); timeWait=4; break;
case 73: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 16: switch (mode) { // Потеряна связь с Wemos
case 1: BUZ(1); timeWait=4; break;
case 2: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 17: switch (mode) { // Запуск контроллера
case 1: BUZ(1); timeWait=20; break;
case 2: BUZ(0); timeWait=30; break;
case 3: case 5: case 7: BUZ(0); timeWait=20; break;
case 4: case 6: BUZ(1); timeWait=5; break;
case 8: BUZ(0); timeWait=30; break;
case 9: BUZ(1); timeWait=50; break;
case 10: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 18: switch (mode) { // Нажатие кнопки пульта
case 1: BUZ(1); timeWait=2; break;
case 2: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 19: switch (mode) { // Кнопка пропущена
case 1: BUZ(1); timeWait=180; break;
case 2: case 4: BUZ(0); timeWait=50; break;
case 3: case 5: BUZ(1); timeWait=80; break;
case 6: BUZ(0); timeWait=20; break;
case 7: BUZ(1); timeWait=70; break;
case 8: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 20: switch (mode) { // Код кнопки сохранен
case 1: BUZ(1); timeWait=30; break;
case 3: case 5: BUZ(1); timeWait=30; break;
case 2: case 4: case 6: BUZ(0); timeWait=25; break;
case 7: BUZ(1); timeWait=50; break;
case 8: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 21: switch (mode) { // Удержание кнопки пульта
case 1: BUZ(1); timeWait=2; break;
case 2: BUZ(0); timeWait=35; break;
case 3: BUZ(1); timeWait=10; break;
case 4: BUZ(0); mode=0; break;} break; 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 22: switch (mode) { // Удержание кнопки пульта
case 1: BUZ(1); timeWait=2; break;
case 2: BUZ(0); timeWait=150; break;
case 3: BUZ(1); timeWait=1; break;
case 4: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 23: switch (mode) { // Удержание кнопки пульта
case 1: BUZ(1); timeWait=2; break;
case 2: timeWait=35; break;
case 4: case 6: BUZ(0); timeWait=35; break;
case 3: case 5: case 7: BUZ(1); timeWait=10; break;
case 8: BUZ(0); mode=0; break;} break;
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
case 24: switch (mode) { // Удержание кнопки
case 1: case 3: BUZ(1); timeWait=1; break;
case 2: BUZ(0); timeWait=20; break;
case 4: BUZ(0); timeWait=20; break;
case 5: mode=0; break;
} break; 
default: mode=0;
} if(mode) mode++; if(mode>73) {BUZ(0); mode=0;}
if(!mode && nextSound) {mode=1; sound=nextSound; nextSound=0; CM(sound); CMn(" -> sound");}}