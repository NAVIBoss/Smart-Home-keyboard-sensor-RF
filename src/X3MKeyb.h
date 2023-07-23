#pragma once
#include <Arduino.h>

class X3MKeyboard {
public:
X3MKeyboard(uint8_t btn0, uint8_t btn1);        // конструктор объявляет пины, к которым подключены кнопки (нажимается -> 0)
uint8_t code();                                 // код нажатой комбинации (опрашивается в loop)
void setDebounceNums(uint8_t nums);             // установить количество считываний для фиксации состояния нажато/отпущено
void setCoefficient(float* kf);                 // установить коэффициент времени двойного клина (BTN_HOLD=500*Btnkf)
void setDoubleTime(uint16_t* time);             // установить время, в течение которого возможно второе нажатие для Double Click
void setLongTime(uint16_t* time);               // установить время, после которого считаем, что клавиша удерживается
void resetPrgTimer();                           // сбром таймера простоя при программировании
void stopPrg();                                 // отмена программирования кнопок
void stopPrgPult();                             // отмена программирования пультов
void okPrg();                                   // программирование кнопок завершено
void okPrgPult();                               // программирование пультов завершено
float getCoefficient();                         // получить коэффициент времени двойного клина (BTN_HOLD=500*Btnkf)
uint16_t getDoubleTime();                       // получить время, в течение которого возможно второе нажатие для Double Click
uint16_t getLongTime();                         // получить время, после которого считаем, что клавиша удерживается

private:
uint8_t CODE=0;                                 // код комбинации нажатия кнопок
uint8_t progMode;                               // режимы программирования (11, 10, 18, 20) + 3сек.
uint8_t returnCode=0;                           // служебные коды событий
int16_t BtnDbSt=10;                             // количество подсчетов фиксированного состояния кнопки для определения нажатия
int16_t BtnLONG=400;                            // время удержания кнопки нажатой для получения long
float Btnkf=1.0;                                // время фиксации длинного нажатия, если кнопка не была отпущена  // BTN_HOLD=500*BTN_COEFFICIETNT
uint32_t pressStartTime;                        // время начала ивента нажатия кнопки
uint32_t progModeTime;
boolean firsPress=0, secondPress=0;
uint8_t btn[2];
boolean doublePress=0;
boolean longPress=0;
uint8_t prgMode[4];                             // режимы программирования (1-пульты, 2- +/- время двойного клика, 3-тренинг времени)
boolean btState[2];
uint16_t BtnHOLD=500;                           // Время до которого второе нажатие будет double click
uint8_t exitCnt;
uint32_t count;
boolean registr[2]={0,0};
int16_t press[2];
uint8_t mCODE0, mCODE1;
boolean codeOk, firstPressUp, secondPressUP, differentKey;
uint32_t btTime;

boolean progStart, warnStop, stopProg, delAllPults;
float learnTime[3];
boolean startPress, startLearn;
boolean btnStateSave[2];
uint8_t countLearn;
boolean act;
uint32_t startTime, pressDelay;
void programmProcess();
};

X3MKeyboard::X3MKeyboard(uint8_t btn0, uint8_t btn1=0) {btn[0]=btn0; pinMode(btn[0],INPUT_PULLUP);
if(btn1) {btn[1]=btn1; pinMode(btn[1],INPUT_PULLUP);} else btn[1]=99;}

void X3MKeyboard::stopPrg() {if(!progMode) {progModeTime=millis(); CODE=0; return;} progMode=0; for_i(0,4)prgMode[i]=0; progModeTime=millis(); CODE=0; returnCode=26;}
void X3MKeyboard::stopPrgPult() {if(!progMode) {progModeTime=millis(); CODE=0; return;} progMode=0; for_i(0,4)prgMode[i]=0; progModeTime=millis(); CODE=0; returnCode=31;}
void X3MKeyboard::okPrg() {if(!progMode) {progModeTime=millis(); CODE=0; return;} progMode=0; for_i(0,4)prgMode[i]=0; progModeTime=millis(); CODE=0; returnCode=33;}
void X3MKeyboard::okPrgPult() {if(!progMode) {progModeTime=millis(); CODE=0; return;} progMode=0; for_i(0,4)prgMode[i]=0; progModeTime=millis(); CODE=0; returnCode=32;}
void X3MKeyboard::resetPrgTimer() {if(!progMode) return; progModeTime=millis();}

void X3MKeyboard::setDebounceNums(uint8_t nums) {BtnDbSt=nums; if(BtnDbSt<4) BtnDbSt=4; if(BtnDbSt>20) BtnDbSt=20; CM("Debounce nums:"); CMn(BtnDbSt);}
void X3MKeyboard::setCoefficient(float* kf) {Btnkf=*kf; BtnHOLD=500.00*Btnkf; Btnkf>1.00 ? BtnLONG=400+50.00*Btnkf : BtnLONG=400; CM("Set kf "); CM(Btnkf); CM(" : "); CM(BtnHOLD); CM(" - "); CMn(BtnLONG);}
void X3MKeyboard::setDoubleTime(uint16_t* time) {BtnHOLD=*time; Btnkf=BtnHOLD/500.00; Btnkf=(round(Btnkf*100.00/5.00))*5.00/100.00; if(Btnkf<0.80) Btnkf=0.80; if(Btnkf>4.00) Btnkf=4.00; Btnkf>1.00 ? BtnLONG=400+50*Btnkf : BtnLONG=400; CM("Set dbl "); CM(Btnkf); CM(" : "); CM(BtnHOLD); CM(" - "); CMn(BtnLONG);}
void X3MKeyboard::setLongTime(uint16_t* time) {BtnLONG=*time; CM("Set "); CM(Btnkf); CM(" : "); CM(BtnHOLD); CM(" - "); CMn(BtnLONG);}
float X3MKeyboard::getCoefficient() {return Btnkf;}
uint16_t X3MKeyboard::getDoubleTime() {return BtnHOLD;}
uint16_t X3MKeyboard::getLongTime() {return BtnLONG;}

uint8_t X3MKeyboard::code() {programmProcess(); if(returnCode) {CM("return from prog start "); CMn(returnCode); uint8_t rCcode=returnCode; returnCode=0; return rCcode;}
// ----------------------------------------------------------------------------------------------------------- фильтр дребезга
for_i(0,2) registr[i]=!dRead(btn[i]);
if(!firsPress && !registr[0] && !registr[1]) return 0;
for_i(0,2) if((press[i]>0 && !registr[i]) || (press[i]<0 && registr[i])) press[i]=0;

if(millis()==btTime) {return 0;} btTime=millis();
for_i(0,2) {
if(press[i]< BtnDbSt && press[i]>=0 &&  registr[i]) press[i]++;
if(press[i]>-BtnDbSt && press[i]<=0 && !registr[i]) press[i]--;}
//if((press[0] > -BtnDbSt && press[0] < BtnDbSt) || (press[1] > -BtnDbSt && press[1] < BtnDbSt)) return 0;
for_i(0,2) {if(!btState[i] && press[i]>=BtnDbSt) {btState[i]=1;} if(btState[i] && press[i]<= -BtnDbSt) {btState[i]=0;}}
// ----------------------------------------------------------------------------------------------------------- фильтр дребезга

if(!btState[0] && !btState[1] && firsPress && codeOk && (millis()-pressStartTime > 20 || progMode==4)) {// если учим, то заканчиваем сразу
firstPressUp=0; firsPress=0; longPress=0; doublePress=0; secondPress=0; secondPressUP=0; differentKey=0; codeOk=0;}

if(!codeOk) {
if((btState[0] || btState[1]) && !firsPress) {firsPress=1; pressStartTime=millis(); CODE=0; mCODE0=0; mCODE1=0;} // нажали 1 раз

if(firsPress && !firstPressUp && mCODE0!=3) {
if(btState[0] && !btState[1]) {mCODE0=1;} if(!btState[0] && btState[1]) {mCODE0=2;}
if(btState[0] && btState[1]) {mCODE0=3; Buzer(24);}} // комбинация кнопок пока не отпустили

if(!firstPressUp && !btState[0] && !btState[1] && firsPress) {firstPressUp=1;} // отпустили 1 раз
if(!secondPress && (btState[0] || btState[1]) && firstPressUp) {secondPress=1; pressStartTime=millis();} // нажали 2 раз

if(secondPress && !secondPressUP && mCODE1!=3) {
if(btState[0] && !btState[1]) {mCODE1=1;} if(!btState[0] && btState[1]) {mCODE1=2;}
if(btState[0] && btState[1]) {mCODE1=3; Buzer(24);} mCODE0!=mCODE1 ? differentKey=1 : differentKey=0;}

if(!secondPressUP && !btState[0] && !btState[1] && secondPress) {secondPressUP=1;} // отпустили 2 раз
if((btState[0] || btState[1]) && secondPress && firsPress && millis() - pressStartTime >= BtnLONG) {doublePress=1; longPress=1; goto outButton;} // Double Long
if((btState[0] || btState[1]) && !secondPress && firsPress && millis() - pressStartTime >= BtnLONG) {longPress=1; goto outButton;} // Long
if(progMode!=4 && firstPressUp && millis() - pressStartTime > BtnHOLD && !secondPress && mCODE0 != 0) {goto outButton;} // отпустили быстро - Click
if(secondPressUP && millis() - pressStartTime <= BtnHOLD) {doublePress=1; goto outButton;}} // Double Click

return 0; outButton: if(!differentKey) {CODE=mCODE0;
if(!longPress && doublePress) CODE=mCODE0+6;
else if(longPress && !doublePress) CODE=mCODE0+3;
else if(longPress && doublePress) CODE=mCODE0+9;}

CM("mCODE0: "); CM(mCODE0); CM(" mCODE1: "); CM(mCODE1); CM(" different: "); CMn(differentKey);

if(differentKey) {
     if(mCODE0==2 && mCODE1==1) longPress ? CODE=16 : CODE=14;
else if(mCODE0==2 && mCODE1==3) longPress ? CODE=16 : CODE=14; // 24 : 22
else if(mCODE0==1 && mCODE1==2) longPress ? CODE=15 : CODE=13;
else if(mCODE0==1 && mCODE1==3) longPress ? CODE=15 : CODE=13; // 23 : 21
else if(mCODE0==3 && mCODE1==1) longPress ? CODE=18 : CODE=17;
else if(mCODE0==3 && mCODE1==2) longPress ? CODE=20 : CODE=19;}
codeOk=1; pressStartTime=millis();
if(returnCode) {CM("return from prog end "); CMn(returnCode); CODE=returnCode; returnCode=0;} return CODE; return 0;}

void X3MKeyboard::programmProcess() {

if(!progMode){
if(CODE==10 && secondPress && millis()-pressStartTime>3000) {progModeTime=millis(); CODE=0; CMn("Prg start 0"); progMode=1; returnCode=21;} // пульты
if(CODE==11 && secondPress && millis()-pressStartTime>3000) {progModeTime=millis(); CODE=0; CMn("Prg start 1"); progMode=2; returnCode=22;} // #2
if(CODE==18 && secondPress && millis()-pressStartTime>3000) {progModeTime=millis(); CODE=0; CMn("Prg start 2"); progMode=3; returnCode=23;} // время Double click
if(CODE==20 && secondPress && millis()-pressStartTime>3000) {progModeTime=millis(); CODE=0; CMn("Prg start 3"); progMode=4; returnCode=24;} // тренинг Double click
}
else {// ------------------------------------------------------------------------------------------------------------------------------------------------ программирование запущено
for_i(0,2) if(btnStateSave[i]!=btState[i]) {btnStateSave[i]=btState[i]; progModeTime=millis(); exitCnt=0; delAllPults=0;} // изменилось состояние, сброс таймера
if(progMode==1 && (CODE==1 || CODE==2)) {CODE=0; progModeTime=millis(); returnCode=30;} // пропуск кнопки пульта
if(millis()-progModeTime>=30000) {stopProg=0; progMode==1 ? returnCode=31 : returnCode=26; progMode=0; CODE=0; for_i(0,4) prgMode[i]=0; return;} //программирование пультов/кнопок отменено
if(exitCnt==0 && millis()-progModeTime>=7500) {exitCnt++; returnCode=28;} // нажмите кнопку
if(exitCnt==1 && millis()-progModeTime>=15000) {exitCnt++; returnCode=28;} // нажмите кнопку
if(exitCnt==2 && millis()-progModeTime>=22500) {exitCnt++; returnCode=28;} // нажмите кнопку
if(CODE==6) {for_i(0,4) prgMode[i]=0; CODE=6; progMode==1 ? returnCode=32 : returnCode=33; progMode=0; return;} // программирование пультов/кнопок завершено
if(!delAllPults && progMode==1 && CODE==12 && millis()-progModeTime>=3000) {delAllPults=1; CODE=0; returnCode=34;} // удаление сохраненных пультов
if((stopProg || warnStop) && !firsPress) {warnStop=0; stopProg=0; CODE=0; progModeTime=millis(); returnCode=29;} // сброс таймера выхода
if(warnStop && stopProg && (CODE==4 || CODE==5) && millis()-progModeTime>=2000) {stopProg=0; progMode==1 ? returnCode=31 : returnCode=26; progMode=0; for_i(0,4) prgMode[i]=0; return;} //программирование пультов/кнопок отменено
if(!warnStop && stopProg && (CODE==4 || CODE==5) && millis()-progModeTime>=1000) {progModeTime=millis(); warnStop=1; returnCode=25;} //предупреждение об остановке программирования
if(!stopProg && (CODE==4 || CODE==5) && millis()-progModeTime>=1000) {warnStop=0; stopProg=1; progModeTime=millis();} // через сек предупредить об остановке программирования
if(progMode==1 && stopProg && warnStop && (CODE==4 || CODE==5) && millis()-progModeTime>=2000) {stopProg=0; CODE=0; for_i(0,4) prgMode[i]=0; returnCode=26; progMode=0; return;} //программирование отменено

if(progMode==3) {if(act && !firsPress) {act=0; CODE=0;}
if(!act && CODE==1) {act=1; Btnkf-=0.05; BtnHOLD-=25;}
if(!act && CODE==7) {act=1; Btnkf-=0.20; BtnHOLD-=100;}
if(!act && CODE==2) {act=1; Btnkf+=0.05; BtnHOLD+=25;}
if(!act && CODE==8) {act=1; Btnkf+=0.20; BtnHOLD+=100;}
if(CODE==1 || CODE==2 || CODE==7 || CODE==8) {
if(Btnkf<0.80) Btnkf=0.80; if(Btnkf > 4.00) Btnkf=4.00; if(BtnHOLD<400) BtnHOLD=400; if(BtnHOLD>2000) BtnHOLD=2000;//0.80-4.00 152-216
Btnkf>1.00 ? BtnLONG=400+50*Btnkf : BtnLONG=400; CM("Dbl clk: "); CM(Btnkf); CM(" : "); CM(BtnHOLD); CM(" - "); CMn(BtnLONG);
switch (CODE) {
case 1: CODE=0; returnCode=36; break;
case 7: CODE=0; returnCode=38; break;
case 2: CODE=0; returnCode=35; break;
case 8: CODE=0; returnCode=37; break;}}}

if(progMode==4 && firsPress) {
if(startLearn) {for_i(0,3) learnTime[i]=500.00; startLearn=0;} if(!firsPress) startPress=0;
if(!startPress && firsPress && !secondPress) {startPress=1; startTime=millis();}
if(startPress && secondPress) {pressDelay=millis()-startTime;
learnTime[countLearn]=pressDelay; countLearn++; if(countLearn>2) countLearn=0; CM("clk: "); CM(countLearn);
float allTime=0; for_i(0,3) allTime+=learnTime[i]; CM(" time: "); CM(pressDelay+100); CMn(" msek. "); CM("Dbl clk: ");
Btnkf=(allTime/3.00+100.00)/500.00; Btnkf=(round(Btnkf*100.00/5.00))*5.00/100.00; if(Btnkf<0.80) Btnkf=0.80; if(Btnkf>4.00) Btnkf=4.00; CM(Btnkf);
if(Btnkf<0.80) Btnkf=0.80; if(Btnkf > 4.00) Btnkf=4.00; Btnkf>1.00 ? BtnLONG=400+50*Btnkf : BtnLONG=400; BtnHOLD=500*Btnkf;
CM(" : "); CM(BtnHOLD); CM(" - "); CMn(BtnLONG); startPress=0; returnCode=36;}}}
if(progMode && !progStart && firsPress) progModeTime=millis(); if(progMode && !progStart && !firsPress) {progStart=1; exitCnt=0; stopProg=0; delAllPults=0; CODE=0;}// не активируем таймер после запуска программирования пока не отпущена клавиша
if(progStart && !progMode) {progStart=0; warnStop=0;} if(!firsPress) CODE=0;
}// ------------------------------------------------------------------------------------------------------------------ программирование

//uint8_t kt=returnCode; static uint8_t ktr; if(ktr!=kt) {uart.print(" ===== ===== ===== ==== Prog "); uart.print(ktr); uart.print(" -> "); uart.println(kt); ktr=kt;}
//ktl(secondPress);