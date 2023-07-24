#define FirmwareVersion "1.0"                         // версия прошивки
//#define clearMemory                                   // записать в память начальные значения
//#define DebagEnable                                   // вывод в порт
#define SerialBusEnable                               // Отправка в виртуальный порт
//#define SensorEnable                                  // Считывание сенсоров
#define BuzerEnable                                   // Звуковое сопровождение
#define EEPROM_Enable                                 // сохранение данных в Arduino
#define RFcontrollerEnable                            // считывание кодов RF пультов
#define ButtonEnable                                  // опрашивать кнопки
//#define ArduinoMEGA                                   // Arduino MEGA - другие порты для BitRead

#include <Arduino.h>

#if defined DebagEnable
#include "Print.h"
class Uart : public Print {
public:
void begin(uint32_t baudrate) {uint16_t speed = (F_CPU / (8L * baudrate)) - 1;
UBRR0 = speed; UCSR0A = (1 << U2X0); UCSR0B = (1 << TXEN0) | (1 << RXEN0); UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);}
void end() {UCSR0B = 0;}
size_t write(uint8_t data) {while (!(UCSR0A & (1 << UDRE0))); UDR0 = data;}
bool available() {return (UCSR0A & (1 << RXC0));}
char read() {byte data = UDR0; return data;}
private:
}; Uart uart;

#define CMn2(x,y) uart.println(x,y)
#define CM(x) uart.print(x)
#define CMn(x) uart.println(x)
#define CM(x) uart.print(x)
#define CMf(x,y) uart.printf(x,y)
#else
#define CMn2(x,y)
#define CM2(x,y)
#define CMn(x)
#define CM(x)
#define CMf(x,y)
#endif

//template<typename T>
//void ktl(T& kt) {static T ktr; if(ktr!=kt) {uart.print("=================== Check "); uart.print(ktr); uart.print(" -> "); uart.println(kt); ktr=kt;}}

#define EVERY_MS(x) \
static uint32_t tmr;\
bool flag = millis() - tmr >= (x);\
if (flag) tmr += (x);\
if (flag)                                           // Выполнять каждые x мс - EVERY_MS(100) {...}

#define returnMS(x) \
{static uint32_t tms;\
if (millis() - tms >= x) tms = millis();\
else return;}                                       // Возврат, если прошло меньше x мс

#define returnSec(x) \
{static uint32_t tmss;\
if (millis() - tmss >= x*1000) tmss = millis();\
else return;}                                       // Возврат, если прошло меньше x с

#define sBus(x) SerialBus(x)

#define for_i(from, to) for(uint8_t i = (from); i < (to); i++)
#define for_c(from, to) for(uint8_t c = (from); c < (to); c++)
#define for_r(from, to) for(uint8_t r = (from); r < (to); r++)
#define for_t(from, to) for(uint8_t t = (from); t < (to); t++)

const uint8_t saveGroup=10;                   // количество сохраняемых пультов
const uint8_t cellButton=8;                   // количество кнопок на пульте
uint8_t prgMode;                              // режим программирования
uint8_t prgProcess[5];                        // каким выключателем запущено программирование

#include <X3MfastPIN.h>
X3MfastPIN fastpin;
#define dRead(x) fastpin.digiRead(x)
#define dWrite(x,y) fastpin.digiWrite(x,y)

#if defined DebagEnable
#if defined BuzerEnable
#include <X3MBuzer.h>
X3MBuzer Buz(3);                                      // PIN пищалки
#define Buzer(x) Buz.go(x)
#else
#define Buzer(x)
#endif
#if defined SensorEnable
const uint8_t KitchenSenosor=A0;                      // PIN датчика маха кухни
#endif
#else
#if defined BuzerEnable
#include <X3MBuzer.h>
X3MBuzer Buz(A4);                                        // PIN пищалки
#define Buzer(x) Buz.go()
#else
#define Buzer(x)
#endif
#if defined SensorEnable
const uint8_t KitchenSenosor=A0;                      // PIN датчика маха кухни
#endif
#endif

#if defined ButtonEnable
#include <X3MKeyb.h>
X3MKeyboard But[5]{{4,5},{6,7},{8,9},{10,11},{12,13}};
#endif

#if defined EEPROM_Enable
#include <EEPROM.h>
#else
#define SaveData() CMn("Data must be saved")
#define ReadData() CMn("Data must be readed")
#endif
struct Save {
uint16_t saveCount=0;                           // счетчик записи в одну и ту же ячейку
uint32_t butCode[saveGroup][cellButton];        // коды кнопок радиопульта
uint16_t BtnHOLD=500;                           // время до которого второе нажатие будет double click
boolean Zumer=1;                                // звуковое сопровождение
boolean Sensor[4]{0,0,0,0};                     // срабатывание сенсора 0-по нулю, 1-по единице
} save;
uint16_t startAdress;                           // стартовый адрес хранения данных в EEPROM
uint32_t rfCode=9999999;                        // код кнопки пульта 433МГц

struct sendStrukt {                             // структура передаваемых данных через serialBus
uint16_t val=0;
};
sendStrukt CODE; sendStrukt seviceCOD; sendStrukt sendData;

boolean initialization=0;                       // инициализация произведена

#if defined RFcontrollerEnable
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
#endif
struct buttonCod {
uint32_t b_Code;
boolean b_long;
};

uint32_t RFtimer;

#if defined SerialBusEnable
#include "GBUS.h"
#if defined DebagEnable
#if defined ArduinoMEGA
GBUS bus(&Serial1, 5, 20); // адрес 5, буфер 20 байт
#else
#include <SoftwareSerial.h>
SoftwareSerial mySerial(A4, A5); // RX, TX
GBUS bus(&mySerial, 5, 20); // адрес 5, буфер 20 байт
#endif
#else
GBUS bus(&Serial, 5, 20);
#endif
#endif

extern int __bss_end; extern void *__brkval; // Функция, возвращающая количество свободного ОЗУ
int memoryFree() {int freeValue; if((int)__brkval==0) freeValue=((int)&freeValue)-((int)&__bss_end); else freeValue=((int)&freeValue)-((int)__brkval); return freeValue;}
void(* resetFunc) (void) = 0; // объявляем функцию reset

#if defined EEPROM_Enable
void SaveData() {
uint16_t addr = 4095;
EEPROM.put (addr, 'x');
addr = 0;
EEPROM.get (addr, startAdress);
if(save.saveCount >= 1000 || startAdress + sizeof(save) > 4095) {startAdress += sizeof(save); save.saveCount = 0; // меняем адрес каждые 1000 сохранений
if ((startAdress + sizeof(save)) > 4095) {startAdress = sizeof(startAdress);} EEPROM.put (0, startAdress);} save.saveCount++;
addr = startAdress;
EEPROM.put (addr, save);
addr = addr+(sizeof(save));
EEPROM.put (addr,save.saveCount);
CM("Save "); CM(int32_t(sizeof(save))); CM(" byte "); CM("to adress: "); CM(startAdress); CM(" | "); 
CM(save.saveCount); CMn(" num(s).");}

void ReadData() {uint8_t initread=0; int16_t addr=4095; boolean needSave=0;
EEPROM.get (addr, initread); if (char(initread) != 'x') {
for_t(0,saveGroup) for_i(0,8) save.butCode[t][i]=9999999; SaveData();} else { // !! запись с пультами
addr = 0;
EEPROM.get (addr, startAdress);
addr = startAdress;
EEPROM.get (addr, save);
}
CM("Read adr: "); CM(startAdress); CM(", "); CM(int32_t(sizeof(save))); CMn(" byte");
#if defined ButtonEnable
for_i(0,5) But[i].setDoubleTime(&save.BtnHOLD);
CM("Dbl clk: "); CM(But[0].getCoefficient()); CM(" : "); CM(But[0].getDoubleTime()); CM(" - "); CMn(But[0].getLongTime());
#endif
#if defined RFcontrollerEnable
CM("RF code "); for_t(0,saveGroup) {for_i(0,cellButton) {CM(save.butCode[t][i]); CM(" ");} CMn();}
#endif
#if defined BuzerEnable
CM("Sound "); CMn(save.Zumer ? "en" : "dis"); save.Zumer ? Buz.en() : Buz.dis();
#endif
#if defined SensorEnable
CMn("Sensor trigger: ");
CM("Alarm: "); CMn(save.Sensor[1] ? "1" : "0");
CM("Input door: "); CMn(save.Sensor[3] ? "1" : "0");
CM("Bath: "); CMn(save.Sensor[2] ? "1" : "0");
CM("Kitchen: "); CMn(save.Sensor[0] ? "1" : "0");
#endif
if(needSave) SaveData();}
#endif

#if defined SerialBusEnable
void SerialBus(uint16_t data=0) {if(!initialization) return; static uint16_t send_Codes[10], lastReciveCode=0, lastDataCode, repeatCount; static uint8_t statusBus, LED;
static uint32_t sendTimer, confirmTimer, inTimer=millis()-500, alarmTimer; //static bCode bSt; if(bSt!=busState) {bSt=busState; CM("----------------------- Bus state "); CMn(bSt);}
static boolean AlarmState;
if(data) {CM("--> Data: "); CMn(data); if(!lastDataCode) inTimer=millis();} if(lastDataCode && millis()-inTimer>400) lastDataCode=0;
bus.tick(); if(bus.statusChanged()) {statusBus=bus.getStatus(); if(statusBus==7 || statusBus==2) {if(!AlarmState) {AlarmState=1; alarmTimer=millis(); CMn("Bus error");}} else if(AlarmState) {AlarmState=0; CMn("Bus Ok");}}
if(data && (data!=lastDataCode || millis()-inTimer>400)) {lastDataCode=data; for_i(0,10) {if(!send_Codes[i]) {send_Codes[i]=data; CM("In Data "); CM(data); CM(" -> cell "); CMn(i); break;}}}
if(send_Codes[0] && !sendData.val) {sendData.val=send_Codes[0]; CM("Send Data "); CM(sendData.val); CM(" cell <- "); CMn(0);}
if(!AlarmState && sendData.val && !statusBus && (sendData.val<1000 || millis()-sendTimer>400)) {sendData.val>1000 ? repeatCount++ : repeatCount=0;
RFtimer=millis(); bus.sendData(3, sendData); CM("<==== Code "); CMn(sendData.val); if(sendData.val<1000) sendData.val+=2000; sendTimer=millis();}
if(!AlarmState && seviceCOD.val && !statusBus && millis()-confirmTimer>5) {RFtimer=millis(); bus.sendData(3, seviceCOD); CM("<= service code "); CMn(seviceCOD.val); seviceCOD.val=0; confirmTimer=millis();}
bus.tick(); if(bus.statusChanged()) {statusBus=bus.getStatus(); if(statusBus==7 || statusBus==2) {if(!AlarmState) {AlarmState=1; alarmTimer=millis(); CMn("Bus error");}} else if(AlarmState) {AlarmState=0; CMn("Bus Ok");}}
if(bus.gotData()) {RFtimer=millis(); bus.readData(CODE); CM("====> Code "); CMn(CODE.val);
if(CODE.val==2000) {CM("Cod = 2000  "); sendData.val=0; CMn("Repeat send code");}
else if(CODE.val>2000) {CM("Cod > 2000  "); if(lastReciveCode==CODE.val-2000) {seviceCOD.val=lastReciveCode+1000; CM("<= Confirm again "); CMn(seviceCOD.val);} else {
seviceCOD.val=2000; CMn("<= This code not read");}}
else if(CODE.val>1000) {CM("Cod > 1000  "); for_i(0,10) {if(send_Codes[i]==CODE.val-1000) {CM("Code "); CM(send_Codes[i]);
CMn(" >>>>>>>>>>>> OK"); for_i(0,9) send_Codes[i]=send_Codes[i+1]; send_Codes[9]=0; sendData.val=0; break;}}}
else if(CODE.val<1000) {CM("Cod < 1000  "); seviceCOD.val=CODE.val+1000; lastReciveCode=CODE.val; CM("<= Confirm "); CMn(seviceCOD.val);}}

switch (CODE.val) {
case 300: if(save.Sensor[0]!=0) {save.Sensor[0]=0; SaveData();} CMn("Сенсор кухни по 0"); break; // kitchen sensor по 0
case 301: if(save.Sensor[0]!=1) {save.Sensor[0]=1; SaveData();} CMn("Сенсор кухни по 1"); break; // kitchen sensor по 1
case 302: if(save.Sensor[1]!=0) {save.Sensor[1]=0; SaveData();} CMn("Сенсор alarm по 0"); break; // alarm sensor по 1
case 303: if(save.Sensor[1]!=1) {save.Sensor[1]=1; SaveData();} CMn("Сенсор alarm по 1"); break; // alarm sensor по 1
case 304: if(save.Sensor[2]!=0) {save.Sensor[2]=0; SaveData();} CMn("Сенсор ванны по 0"); break; // bath sensor по 1
case 305: if(save.Sensor[2]!=1) {save.Sensor[2]=1; SaveData();} CMn("Сенсор ванны по 1"); break; // bath sensor по 1
case 306: if(save.Sensor[3]!=0) {save.Sensor[3]=0; SaveData();} CMn("Сенсор двери по 0"); break; // input door sensor по 1
case 307: if(save.Sensor[3]!=1) {save.Sensor[3]=1; SaveData();} CMn("Сенсор двери по 1"); break; // input door sensor по 1
case 218: float Btnkf=save.BtnHOLD/500.00; CM("Clk time: "); char value[8]; dtostrf(Btnkf,1,2,value); CM("Dbl clk: "); CM(value); CM(" : "); dtostrf(500*Btnkf,1,0,value); CM(value); CMn(" msek.");
dtostrf(Btnkf,1,2,value); data=(Btnkf*100/5+136);
if(data!=lastDataCode || millis()-inTimer>400) {lastDataCode=data; for_i(0,10) {if(!send_Codes[i] || send_Codes[i]==data)
{send_Codes[i]=data; CM("Clk Data "); CM(data); CM(" -> cell "); CMn(i); break;}}} break;
} CODE.val=0;
//if(CODE.val>151 && CODE.val<217) {CM("Click button: "); float Btnkf=(CODE.val-136)*5; Btnkf=Btnkf*0.01; CM(Btnkf); CM(" : "); char value[8]; dtostrf(500*Btnkf,1,0,value); CM(value); CMn(" msek."); save.BtnHOLD=500*Btnkf;}
if((AlarmState && millis()-alarmTimer>5000) || repeatCount>150) {CMn("Connection error. Reset"); delay(100); resetFunc();}
if((AlarmState && millis()-alarmTimer>2000) || repeatCount>75) {static boolean process; if(!AlarmState && repeatCount<10) {if(!process) {process=0; LED=0; pinMode(13,INPUT_PULLUP);} return;}
process=1; static uint32_t timer, waitTime; if(millis()-timer<waitTime) return; timer=millis();
LED++; switch(LED) {
case 1: case 3: waitTime=250; pinMode(13, OUTPUT); dWrite(13,0); break;
case 2: case 4: pinMode(13,INPUT_PULLUP); waitTime=50; break;
case 5: waitTime=750; pinMode(13, OUTPUT); dWrite(13,0); LED=0; break;}
returnSec(1); Buzer(16);}}
#else
void SerialBus(uint16_t data=0) {if(data) {CM("<== Bus "); CMn(data);}}
#endif

#if defined SensorEnable
void readSensor() {static boolean firstStart[3], inp, bath, alarm; static boolean init, waitProc[3], changeState[3], state[3]; static uint32_t timer[3];
if(millis()-timer[0]>100) {if(save.Sensor[3]) state[0]=dRead(A3); else state[0]=!dRead(A3);} if(millis()-timer[1]>100) {if(save.Sensor[2]) state[1]=dRead(A2); else state[1]=!dRead(A2);}
#if defined ArduinoMEGA
if(millis()-timer[2]>100) {if(save.Sensor[1]) state[2]=dRead(A4); else state[2]=!dRead(A4);} // A1 сгоревший
#else
if(millis()-timer[2]>100) {if(save.Sensor[1]) state[2]=dRead(A1); else state[2]=!dRead(A1);}
#endif
if(!waitProc[0] && inp!=state[0]) {inp=state[0]; waitProc[0]=1; timer[0]=millis(); if(firstStart[0]) {CM("Sensor A3 = "); if(!inp) {CMn("LOW"); sBus(140);} else {CMn("HIGH"); Buzer(1); sBus(141);}}}
if(!waitProc[1] && bath!=state[1]) {bath=state[1]; waitProc[1]=1; timer[1]=millis(); if(firstStart[1]) {CM("Sensor A2 = "); if(!bath) {CMn("LOW"); Buzer(1); sBus(142);} else {CMn("HIGH"); sBus(143);}}}
if(!waitProc[2] && alarm!=state[2]) {alarm=state[2]; waitProc[2]=1; timer[2]=millis(); if(firstStart[2]) {CM("Sensor A1 = "); if(!alarm) {CMn("LOW"); Buzer(1); sBus(144);} else {CMn("HIGH"); sBus(145);}}}

for_i(0,3) {
if(!firstStart[i] && millis()-timer[i]>1000) {firstStart[i]=1; CM("Init sensor A"); CM(i); CMn(" Ok");}
if(firstStart[i] && waitProc[i] && millis()-timer[i]>3000) {waitProc[i]=0; changeState[i]=0; CM("Timer A"); CM(3-i); CMn(" OFF");} // A0-вход A1-сигнализация A2-ванна A3-вход
if(firstStart[i] && waitProc[i] && !changeState[i] && inp!=state[i] && millis()-timer[i]<3000) {changeState[i]=1; timer[i]=millis(); CM("Bounce A"); CM(3-i); CMn(" ");}
if(firstStart[i] && changeState[i] && inp==state[i] && millis()-timer[i]<3000) {changeState[i]=0; timer[i]=millis(); CM("Timer A"); CM(3-i); CMn(" reset");}}
if(!init && initialization && firstStart[0] && firstStart[1] && firstStart[2]) {init=1; CMn("Init sens Ok");}}

void readKithenSensor() {static uint8_t CODE; boolean doublePress=0; boolean longPress=0;
static boolean pressEvent, EventEND, firstPressUp, secondPressUP, secondPress;
static uint32_t pressStartTime, Press_StopTime, btTime; static boolean btState, btStateSave, stateNUL;
if(!save.Sensor[0]) btState=!dRead(KitchenSenosor); else btState=dRead(KitchenSenosor); if(btState && EventEND) Press_StopTime=millis(); if(EventEND && millis()-Press_StopTime<200) return;
if (millis() == btTime) return; btTime=millis();
//static boolean ks[2]; if(ks[0]!=dRead(btn[0])) {ks[0]=dRead(btn[0]); CM("Key0 "); CM(nBut); CM(" "); ks[0] ? CMn("UP") : CMn("DOWN");} if(ks[1]!=dRead(btn[1])) {ks[1]=dRead(btn[1]); ks[1] ? CMn("Key1 UP") : CMn("Key1 DOWN");}
if(!pressEvent && !btState ) return;
if (!stateNUL && !btState  && pressEvent && EventEND && millis()-Press_StopTime > 500) {firstPressUp=0; pressEvent=0; 
longPress=0; doublePress=0; secondPress=0; secondPressUP=0; EventEND=0; return;} if (EventEND) return;
if (btState && !pressEvent) {pressEvent=1; pressStartTime=millis();} // нажали 1 раз
if(!stateNUL && !firstPressUp && !btState && millis()-pressStartTime<100) {stateNUL=1; btStateSave=btState;} // не реагировать на пальцы
if(stateNUL && btStateSave!=btState && pressEvent && !firstPressUp && millis()-pressStartTime<100) {btStateSave=btState; pressStartTime=millis();}
if(stateNUL && millis()-pressStartTime>=100) stateNUL=0;
if (!stateNUL && !btState && pressEvent && !firstPressUp) firstPressUp=1; // отпустили 1 раз
if (!secondPress && btState && pressEvent && firstPressUp) {secondPress=1; pressStartTime=millis();} // нажали 2 раз
if (!btState && pressEvent && firstPressUp && secondPress && !secondPressUP) secondPressUP=1; // отпустили 2 раз
if (btState && secondPress && pressEvent && millis() - pressStartTime >= 500) {doublePress=1; longPress=1; goto outButton;} // держим на 2 раз - 99
if (btState && !secondPress && pressEvent && millis() - pressStartTime >= 400) {longPress=1; goto outButton;} // держим -139
if (pressEvent && !firstPressUp) return;
if (!secondPress && millis() - pressStartTime > 500) {goto outButton;} // отпустили быстро
if (pressEvent && !secondPress) return;
if (secondPressUP && millis() - pressStartTime <= 500) {doublePress=1; goto outButton;} // двойное нажатие
return; outButton:
if(!longPress && !doublePress) {CODE=41; Buzer(1);} else if(!longPress && doublePress) {CODE=47; Buzer(22);} else if(longPress && !doublePress) {CODE=42; Buzer(21);} else
if(longPress && doublePress) {CODE=47; Buzer(22);} EventEND=1; Press_StopTime=millis(); CM("CODE: "); CMn(CODE); sBus(CODE); CODE=0;}
#endif

#if defined RFcontrollerEnable
void switchRFCode(buttonCod& readRF) {if(prgMode) return; // CM("RFCode: "); CM(readRF.b_Code); if(readRF.b_long) CMn(" long"); else CMn();
for_t(0,saveGroup) for_i(0,cellButton) {if(readRF.b_Code==save.butCode[t][i]) {!readRF.b_long ? sBus(i*2+121) : sBus(i*2+122); readRF.b_long ? Buzer(21) : Buzer(18); return;}}}

void programRF(buttonCod readCod={0,0}, uint8_t in=0) {if(prgMode!=1) return; static uint32_t RFCODE, lastRFCODE, waitVoice, btnCode[cellButton], progModeTime; // программирование RF
static boolean startProg, completeProg, progRfCode, chekPult, AllCellSave, freeBlokRF, btAlreadySaved; static uint8_t saveRFGroup; // 10 пультов по 8 кнопок
static enum rfbutton {but1, but2, but3, but4, but5, but6, but7, but8, buz1, buz2, buz3, buz4, buz5, buz6, buz7, buz8, idl, end} mode; static boolean MoveCels;

if(in) {switch (in) {
case 100: startProg=0; break;
case 101: for_t(0,saveGroup) for_i(0,8) save.butCode[t][i]=9999999; SaveData(); CMn("Delete all RF Ok"); Buzer(15); sBus(120); startProg=0; waitVoice=2550; break;
case 102: progRfCode=0; mode=end; CMn("Prog RF cancel"); startProg=0; return; break;
case 103: if(mode!=idl) {CM("Button "); CM(mode/2); CMn(" skiped");
switch(mode) {
case but1: btnCode[0]=9999999; mode=buz2; break;
case but2: btnCode[1]=9999999; mode=buz3; break;
case but3: btnCode[2]=9999999; mode=buz4; break;
case but4: btnCode[3]=9999999; mode=buz5; break;
case but5: btnCode[4]=9999999; mode=buz6; break;
case but6: btnCode[5]=9999999; mode=buz7; break;
case but7: btnCode[6]=9999999; mode=buz8; break;
case but8: btnCode[7]=9999999; mode=end; break;
default: CM("Other state"); break;}
Buzer(19); sBus(118); progModeTime=millis(); waitVoice=1700; RFCODE=0;} break;
case 104: mode=end; break;}}

if(readCod.b_Code) {for_i(0,5) But[i].resetPrgTimer(); CM("-> RFCode: "); CM(readCod.b_Code); if(readCod.b_long) CMn(" long"); else CMn();}

if(!startProg) {AllCellSave=1; RFCODE=0; lastRFCODE=0; startProg=1; chekPult=0; completeProg=0; progRfCode=0; progModeTime=millis(); mode=buz1; saveRFGroup=0; MoveCels=1;
for_i(0,cellButton) btnCode[i]=9999999; for_t(0,saveGroup) {freeBlokRF=1; for_i(0,cellButton) if(save.butCode[t][i]!=9999999) freeBlokRF=0; btAlreadySaved=0;
if(freeBlokRF) {saveRFGroup=t; AllCellSave=0; CM("Group "); CM(t); CMn(" is free"); break;} else CM("Group "); CM(t); CMn(" is busy");}
if(AllCellSave) CMn("All 10 RF saved"); waitVoice=2480; CMn("Start RF prog"); return;}

if(lastRFCODE!=readCod.b_Code && !RFCODE && readCod.b_Code) {RFCODE=readCod.b_Code; lastRFCODE=readCod.b_Code;}
static rfbutton stateMode; if(stateMode!=mode && mode!=idl && millis()-progModeTime>waitVoice) {stateMode=mode; btAlreadySaved=0;

switch(mode) {
case buz1: Buzer(9); sBus(108); mode=but1; CMn("Prg but 1"); break;
case buz2: Buzer(9); sBus(109); mode=but2; CMn("Prg but 2"); break;
case buz3: Buzer(9); sBus(110); mode=but3; CMn("Prg but 3"); break;
case buz4: Buzer(9); sBus(111); mode=but4; CMn("Prg but 4"); break;
case buz5: Buzer(9); sBus(112); mode=but5; CMn("Prg but 5"); break;
case buz6: Buzer(9); sBus(113); mode=but6; CMn("Prg but 6"); break;
case buz7: Buzer(9); sBus(114); mode=but7; CMn("Prg but 7"); break;
case buz8: Buzer(9); sBus(115); mode=but8; CMn("Prg but 8"); break;
default: CM("Other state"); break;} progModeTime=millis(); waitVoice=1700;}

if(!chekPult && mode!=idl && RFCODE && millis()-progModeTime>waitVoice) {CM("Check RF Code "); CMn(RFCODE);
for_t(0,saveGroup) for_i(0,cellButton) {if(saveRFGroup!=t && save.butCode[t][i]==RFCODE) {saveRFGroup=t; MoveCels=0; CM("Already saved in group "); CMn(t); sBus(107); progModeTime=millis(); waitVoice=2500; break;}}
if(AllCellSave && MoveCels) {for_t(1,saveGroup) for_i(0,cellButton) save.butCode[t-1][i]=save.butCode[t][i]; saveRFGroup=9; CMn("All busy, <-move, save in 9"); MoveCels=0;} 
if(MoveCels) {CMn("New remote");} chekPult=1; rfCode=9999999;}

if(chekPult && mode!=idl && RFCODE && millis()-progModeTime>waitVoice) {boolean find=0; uint8_t numsButton;
switch(mode) {
case but1: numsButton=1; break;
case but2: numsButton=2; break;
case but3: numsButton=3; break;
case but4: numsButton=4; break;
case but5: numsButton=5; break;
case but6: numsButton=6; break;
case but7: numsButton=7; break;
case but8: numsButton=8; break;
default: CM("Other state"); break;}
for_i(0,numsButton) if(RFCODE==btnCode[i]) {find=1; break;} if(find) {btAlreadySaved=1; CM(RFCODE); rfCode=0; RFCODE=0; CM(" already saved in "); // кнопка уже была сохранена
CMn(mode/2); sBus(119); progModeTime=millis(); waitVoice=2330;}}

if (!btAlreadySaved && chekPult && mode!=idl && RFCODE && millis()-progModeTime>waitVoice) {CM("Wait button ");
switch(mode) { // кнопка нажата и не была уже сохранена
case but1: btnCode[0]=RFCODE; mode=buz2; CM("1"); break;
case but2: btnCode[1]=RFCODE; mode=buz3; CM("2"); break;
case but3: btnCode[2]=RFCODE; mode=buz4; CM("3"); break;
case but4: btnCode[3]=RFCODE; mode=buz5; CM("4"); break;
case but5: btnCode[4]=RFCODE; mode=buz6; CM("5"); break;
case but6: btnCode[5]=RFCODE; mode=buz7; CM("6"); break;
case but7: btnCode[6]=RFCODE; mode=buz8; CM("7"); break;
case but8: btnCode[7]=RFCODE; mode=end;  CM("8"); break;
default: CM("Other state"); break;}
CMn(" code"); Buzer(20); sBus(116); progRfCode=1; RFCODE=0; chekPult=0; progModeTime=millis(); waitVoice=170;} btAlreadySaved=0;

if(!completeProg && mode==end) {mode=idl; if(progRfCode) {completeProg=1; CM("Save in "); CM(saveRFGroup); CM(" group: ");
for_i(0,cellButton) {save.butCode[saveRFGroup][i]=btnCode[i]; CM(btnCode[i]); CM(" ");} CMn();
for_i(0,5) if(prgProcess[i]) {But[i].okPrgPult(); break;} CMn("Prog FR Ok"); Buzer(6); sBus(105); SaveData();} else
{CMn("All empty."); prgMode=0; for_i(0,5) if(prgProcess[i]) {But[i].stopPrgPult(); break;} Buzer(7); sBus(106);}}
if(completeProg && millis()-progModeTime>waitVoice) {completeProg=0; prgMode=0;}
} // ---------------------------------------------------------------------------- программирование RF

void RFresive() {static uint32_t lastCode, readCode, timerStart; static boolean start, stop, longRFPress; buttonCod retStr; retStr.b_Code=0; retStr.b_long=0;
if(start && stop && !longRFPress && millis()-RFtimer>200) {start=0; stop=0;}
if(start && stop && longRFPress && millis()-RFtimer>700) {start=0; stop=0;}
if (mySwitch.available()) {RFtimer=millis(); readCode=mySwitch.getReceivedValue();
if(start && readCode!=lastCode) {retStr.b_Code=lastCode; retStr.b_long=longRFPress;
lastCode=readCode; stop=0; RFtimer=millis(); timerStart=millis(); longRFPress=0; mySwitch.resetAvailable(); 
if(!retStr.b_long) {CM("<== RF: "); CMn(readCode); !prgMode ? switchRFCode(retStr) : programRF(retStr);}} // нажали другую кнопку быстро
if(stop && millis()-RFtimer<=500) {RFtimer=millis(); mySwitch.resetAvailable();}
if(!start && !stop) {RFtimer=millis(); timerStart=millis(); start=1; retStr.b_long=0; longRFPress=0;} else
if(!longRFPress && !stop && start && millis()-timerStart>500 && lastCode==readCode) {longRFPress=1; stop=1; retStr.b_Code=lastCode; retStr.b_long=longRFPress;
RFtimer=millis(); mySwitch.resetAvailable(); CM("<== RF: "); CM(readCode); CMn(" long"); !prgMode ? switchRFCode(retStr) : programRF(retStr);} lastCode=readCode; mySwitch.resetAvailable();} else {
if(!longRFPress && start && !stop && millis()-RFtimer>400) {retStr.b_Code=lastCode; retStr.b_long=longRFPress; stop=1;
RFtimer=millis(); CM("<== RF: "); CMn(readCode); !prgMode ? switchRFCode(retStr) : programRF(retStr);}}}
#else
void programRF(buttonCod readCod={0,0}, uint8_t in=0) {CMn("RF not available.");
#if defined ButtonEnable
for_i(0,5) if(prgProcess[i]) {But[i].stopPrg(); break;}
#endif
Buzer(7); sBus(uint16_t(106));}
#endif

#if defined ButtonEnable
void scanCode() {
uint8_t CODE=0; static uint8_t saveCODE[5]; uint8_t readCODE[5], Switch; //for_t(0,5) {CODE=But.code(); if(CODE) {CODE+=20*(t+1); CM("Switch "); CM(t+1); CM(" CODE: "); CMn(CODE); sBus(CODE);}}}
for_i(0,5) {readCODE[i]=But[i].code(); if(saveCODE[i]!=readCODE[i]) {saveCODE[i]=readCODE[i]; Switch=i; CODE=readCODE[i]; CM("But "); CM(i); CM(" CODE: "); CMn(CODE); break;}}

//static uint32_t ckp, cks; ckp=RFCODE; if(cks!=ckp) {CM(" ---------------------- > Change "); CM(cks); CM(" -> "); CMn(ckp); cks=ckp;}
//ktl(But[0].check());

if(!CODE) return;
if(prgProcess[Switch]) {switch (CODE) {// действия в режиме программирования
case 25: CMn("Через сек выход"); Buzer(8); break;
case 26: CMn("Программирование кнопок отменено"); prgProcess[Switch]=0; But[Switch].setDoubleTime(&save.BtnHOLD); prgMode=0; Buzer(7); sBus(106); return; break;
case 27: CMn("Время истекло"); prgProcess[Switch]=0; if(prgMode==1) programRF({0,0},102); else {But[Switch].setDoubleTime(&save.BtnHOLD);} prgMode=0; Buzer(7); sBus(106); return; break;
case 28: CMn("Нажмите кнопку"); Buzer(10); sBus(104); break;
case 29: CMn("Сброс таймера выхода"); Buzer(11); break;
case 30: if(prgMode==1) {CMn("Пропуск кнопки пульта"); programRF({0,0},103);} break;
case 31: CMn("Программирование пультов отменено"); prgProcess[Switch]=0; programRF({0,0},102); prgMode=0; Buzer(7); sBus(106); return; break;
case 32: CMn("Программирование пультов завершено"); prgProcess[Switch]=0; programRF({0,0},104); prgMode=0; return; break;
case 33: CMn("Программирование кнопок завершено"); prgProcess[Switch]=0; auto dblTimePtr {But[Switch].getDoubleTime()}; for_i(0,5) if(i!=Switch) But[i].setDoubleTime(&dblTimePtr); // auto - указатель на ф-ю
save.BtnHOLD=dblTimePtr; SaveData(); prgMode=0; Buzer(6); sBus(105); return; break;
case 34: CMn("Удаление RF пультов"); programRF({0,0},101); break;
}}

if(!prgMode) {switch (CODE) {// запуск режима программирования
case 21: CMn("Программирование пультов"); prgProcess[Switch]=1; prgMode=1; programRF({0,0},100); Buzer(5); sBus(103); break;
case 22: CMn("Режим программирования 2"); prgProcess[Switch]=1; break; // не задействован
case 23: CMn("Программирование Double click"); prgProcess[Switch]=1; Buzer(3); sBus(100); prgMode=2; break;
case 24: CMn("Тренинг Double click"); prgProcess[Switch]=1; Buzer(4); sBus(101); prgMode=3; break;
}}

switch (CODE) {// включение/выключение звука клавиш
case 15: if(!save.Zumer) {save.Zumer=1; Buz.en(); Buzer(13); SaveData(); sBus(95); CMn("Sonud ON");} break;
case 16: if(save.Zumer) {Buzer(14); save.Zumer=0; Buz.dis(); SaveData(); sBus(96); CMn("Sonud OFF");} break;}

if(save.Zumer) switch (CODE) { // озвучка нажатия кнопок
case 1: case 2: case 3: Buzer(18); break;
case 4: case 5: case 6: Buzer(21); break;
case 7: case 8: case 9: case 13: case 14: case 17: case 19: Buzer(22); break;
case 10: case 11: case 12: case 15: case 16: Buzer(23); break;
case 18: Buzer(23);
case 20: Buzer(14);}

switch (CODE) {// озвучка при программировании
case 35: case 36: Buzer(1); break;
case 37: case 38: Buzer(2); break;}
if(prgMode || CODE>20) return; sBus(CODE+20*Switch);}
#endif

void Initialize() {if(initialization) return; static uint32_t startTime=millis(); if(millis()-startTime<1000) return;
initialization=1; CMn("Start after 1 sek"); Buzer(17); float Btnkf=save.BtnHOLD/500.00; sBus(uint8_t(Btnkf*100.00/5.00+136));}

void setup() {
#if defined DebagEnable
uart.begin(115200); delay(10); CMn("\n");
#if defined SerialBusEnable
#if defined ArduinoMEGA
Serial1.begin(57600); // 19-(RX) 18-(TX)
#else
mySerial.begin(57600);
#endif
#endif
#else
Serial.begin(57600);
#endif
#if defined RFcontrollerEnable
mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
#endif
#if defined SensorEnable
pinMode(A1,INPUT); pinMode(A2,INPUT); pinMode(A3,INPUT);
#endif
#if defined clearMemory
SaveData();
#else
ReadData();
#endif
//for_t(0,saveGroup) for_i(0,8) save.butCode[t][i]=t+i+1; SaveData();
CMn("Init end");}

void loop() { //sendDataLoop(); //EVERY_MS(1000) CMn(memoryFree()); buzloop(); //loopspeed();
//EVERY_MS(1000) {static uint8_t cnt=119; CM("Sound "); CMn(cnt); sBus(cnt); return; cnt++; if(cnt>120) cnt=119;}
Initialize(); //ktl(But[0].CODE);
//uint32_t kt=But[0].progModeTime; static uint32_t ktr; if(ktr!=kt) {uart.print(" ===== ===== ===== ==== Prog "); uart.print(ktr); uart.print(" -> "); uart.println(kt); ktr=kt;}

#if defined SensorEnable
readSensor();
readKithenSensor();
#endif
#if defined BuzerEnable
Buzer();
#endif
if(!initialization) return;
#if defined ButtonEnable
scanCode();
#endif
sBus();
#if defined RFcontrollerEnable
RFresive();
programRF();
#endif
#if defined DebagEnable
#endif
}