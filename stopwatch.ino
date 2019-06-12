/*

  ШТОПЕРИЦА са 6 цифара, са приказом у формату МM:SS.DS

  (најдужи интервал који може да се прикaже је 60 m, односно 3600 s)

*/
#include "LedControl.h"

LedControl lc1 = LedControl(12, 11, 10, 2);         // 12 to DATA IN, 11 to CLK, 10 to LOAD(/CS)
LedControl lc2 = LedControl(5, 4, 3, 2);            // 5 to DATA IN, 4 to CLK, 3 to LOAD(/CS)

byte a = 0;         //цифра за приказ стотинки (од 0 до 9)
byte b = 0;         //цифра за приказ десетинки (од 0 до 9)
byte c = 0;         //цифра за приказ секунди (од 0 до 9)
byte d = 0;         //цифра за приказ десетина секунди (од 0 до 5)
byte e = 0;         //цифра за приказ минута (од 0 до 9)
byte f = 0;         //цифра за приказ десетина минута (од 0 до 5)

byte a_lap = 0;     //цифре за приказ пролазних времена
byte b_lap = 0;
byte c_lap = 0;
byte d_lap = 0;
byte e_lap = 0;
byte f_lap = 0;

unsigned long currenttime;
unsigned long previoustime = 0;
unsigned long elapsedtime = 0;
unsigned long prev_elapsedtime = 0;
unsigned long laptime;

byte startPin = 7;                             //улаз коме се доводи сигнал за старт као и за пролазно време
byte stopPin = 8;                              //улаз коме се доводи сигнал за стоп бројача
byte resetPin = 9;                             //улаз коме се доводи сигнал за ресет бројача
byte enablePin = 13;                           //улаз коме се доводи сигнал за омогућавање заустављања бројача старт сигналом

byte startButtonState = 1;
byte previousStartButtonState = 1;
byte stopButtonState = 1;
byte previousStopButtonState = 1;
byte resetButtonState = 1;
byte previousResetButtonState = 1;
byte enableButtonState = 1;
byte previousEnableButtonState = 1;
int debounceInterval = 1000;
byte startCondition = 0;

unsigned long millisPrevious;
unsigned long correctionStep;
unsigned long previousStep;


void setup() {
  pinMode(startPin, INPUT_PULLUP);
  pinMode(stopPin, INPUT_PULLUP);
  pinMode(resetPin, INPUT_PULLUP);
  pinMode(enablePin, INPUT_PULLUP);

  lc1.shutdown(0, false); // turn off power saving, enables display
  lc1.setIntensity(0, 15); // sets brightness (0~15 possible values)
  lc1.clearDisplay(0);// clear screen
  lc1.setChar(0, 2, '-', false);
  lc1.setChar(0, 5, '-', false);
  lc2.shutdown(0, false); // turn off power saving, enables display
  lc2.setIntensity(0, 15); // sets brightness (0~15 possible values)
  lc2.clearDisplay(0);// clear screen
  lc2.setChar(0, 2, '-', false);
  lc2.setChar(0, 5, '-', false);

  Serial.begin(115200);
  
}

void loop() {
  startButtonState = digitalRead(startPin);
  stopButtonState = digitalRead(stopPin);
  resetButtonState = digitalRead(resetPin);
  enableButtonState = digitalRead(enablePin);
  currenttime = micros();


  if ((previousStartButtonState == HIGH) && (startButtonState == LOW) && (startCondition == 0)) {       //извршава се на силазну ивицу старт сигнала
    startCondition = 1;
    previoustime = currenttime;
    millisPrevious = millis();
    previousStartButtonState = startButtonState;
  }


  if (startCondition == 1) {
    timeCounter();
  }
  
  if ((previousEnableButtonState == HIGH) && (enableButtonState == LOW) && (startCondition == 1)) {
    previousEnableButtonState = enableButtonState;
  }

  if ((previousStartButtonState == HIGH) && (startButtonState == LOW) && (startCondition == 1) && (previousEnableButtonState == HIGH)) {
    lapTimeCounter();
    millisPrevious = millis();
    previousStartButtonState = startButtonState;
  }


  if ((previousStartButtonState == HIGH) && (startButtonState == LOW) && (previousEnableButtonState == LOW)) {
    startCondition = 2;
  }



  if ((startButtonState == HIGH) && (millis() - millisPrevious >= debounceInterval)) {
    previousStartButtonState = startButtonState;
  }



  if ((previousStopButtonState == HIGH) && (stopButtonState == LOW) && (startCondition == 1)) {
    startCondition = 3;
  }
  if ((previousResetButtonState == HIGH) && (resetButtonState == LOW) && ((startCondition == 3) or (startCondition == 2))) {
    startCondition = 0;
    a = 0; a_lap = 0;
    b = 0; b_lap = 0;
    c = 0; c_lap = 0;
    d = 0; d_lap = 0;
    e = 0; e_lap = 0;
    f = 0; f_lap = 0;
    prev_elapsedtime = 0;
    previousEnableButtonState = HIGH;
    
  }

  
  DispData();

  Serial.print("Time: "); Serial.print(f); Serial.print(e); Serial.print(":");
  Serial.print(d); Serial.print(c); Serial.print("."); Serial.print(b); Serial.println(a);

  Serial.print("Lap:  "); Serial.print(f_lap); Serial.print(e_lap);
  Serial.print(":"); Serial.print(d_lap); Serial.print(c_lap); Serial.print("."); Serial.print(b_lap); Serial.println(a_lap);
}




void timeCounter() {
  elapsedtime = currenttime - previoustime;
  correctionStep = currenttime - previousStep;

  if (correctionStep  >= 1000000) {         //овај део кода је за фино подешавање, односно за постизање веће прецизности штоперице
    previoustime = previoustime + 94;       //сваке секунде се укупно протекло време, које се приказује на дисплеју, смањује за 94 μs
    previousStep = currenttime;             //ова вредност се експериментално утврђује и важи само за дати ардуино
  }

  if (elapsedtime >= 3600000000) {            //кад протекне 3600 секунди
    previoustime = previoustime + 3600000000;
    elapsedtime = 0;
  }

  a = (elapsedtime / 10000) % 10;        // стотинке = последња цифра количника целобројног дељења μs / 10000 (нпр. 765338972 μs / 10000 = 76533 => 3)
  b = (elapsedtime / 100000) % 10;       // десетинке = последња цифра количника целобројног дељења μs / 100000 (765338972/100000=7653 => 3)
  c = (elapsedtime / 1000000) % 10;      // секунде = последња цифра количника целобројног дељења μs / 1000000 (765338972/1000000=765 => 5)
  d = (elapsedtime / 10000000) % 6;      // десетине секунди = остатак при дељењу са 6 количника целобројног дељења μs / 10000000 (765338972/10000000=76, 76/6=12 са остатком 4)
  e = (elapsedtime / 60000000) % 10;     // минути = секунде / 60 = последња цифра количника целобројног дељења секунде / 60, односно μs / 60000000 (765338972/60000000=12 => 2)
  f = (elapsedtime / 600000000) % 6;     // десетине минута = остатак при дељењу са 6 количника целобројног дељења μs / 600000000 (765338972 μs / 600000000 = 1, 1/6=0 остатак је 1)
  // крајњи резултат из примера 765338972 μs пребачено у жељени формат је 12:45.33
}




void lapTimeCounter() {
  laptime = elapsedtime - prev_elapsedtime;
  prev_elapsedtime = elapsedtime;
  a_lap = (laptime / 10000) % 10;
  b_lap = (laptime / 100000) % 10;
  c_lap = (laptime / 1000000) % 10;
  d_lap = (laptime / 10000000) % 6;
  e_lap = (laptime / 60000000) % 10;
  f_lap = (laptime / 600000000) % 6;
}



void DispData() {
  lc1.setDigit(0, 0, (byte)a, false);
  lc1.setDigit(0, 1, (byte)b, false);
  lc1.setDigit(0, 3, (byte)c, false);
  lc1.setDigit(0, 4, (byte)d, false);
  lc1.setDigit(0, 6, (byte)e, false);
  lc1.setDigit(0, 7, (byte)f, false);
  lc2.setDigit(0, 0, (byte)a_lap, false);
  lc2.setDigit(0, 1, (byte)b_lap, false);
  lc2.setDigit(0, 3, (byte)c_lap, false);
  lc2.setDigit(0, 4, (byte)d_lap, false);
  lc2.setDigit(0, 6, (byte)e_lap, false);
  lc2.setDigit(0, 7, (byte)f_lap, false);
}
