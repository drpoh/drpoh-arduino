/*     pech v0.3 (final)
 *  +++22.03.2016+++
 *
 *
*/
#include <LiquidCrystal.h>
#include "OneWire.h"
#define PUMP_PIN 3
#define LED_PIN 13
#define SOUND_PIN 4
#define BACKLIGHT_PIN 5
#define MAX_TEMP 50
#define DANGER 85
byte arrowUp[8] = {
  0b00100,
  0b01110,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte arrowDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b01110,
  0b00100
};
//(RS,E,D4,D5,D6,D7)
LiquidCrystal lcd(10, 11, 6, 7, 8, 9);
OneWire  ds(2);  // on pin 2 (a 4.7K resistor is necessary)
int toneVal = 100;
int upToneDone = 0;
int downToneDone = 0;
int toneDone = 0;
int pumpState = 0;
int lcdState = 0;
float temp = 0;
float lastTemp = 0;
long TEMP_INTERVAL = 3000;  // 3 sec
long PUMP_INTERVAL = 60000; //1 min
int maxTempStat = 0;
unsigned long tempPrevMillis = 0;
unsigned long pumpPrevMillis = 300000;
unsigned long warningPrevMillis = 0;
unsigned long statPrevMillis = 0;
unsigned long arrowPrevMillis = 0;
int arrowChar = 2; //= 0x3D
char tempChar[] = {0xDF, 0x43, '\0'}; //Отрисовка градусов Цельсия
char nasosVkl[] = {0x48, 0x41, 0x43, 0x4F, 0x43, 0x2D, 0x42, 0x4B, 0xCA, 0xFE, '\0'};
char nasosOtkl[] = {0x48, 0x41, 0x43, 0x4F, 0x43, 0x2D, 0x4F, 0x54, 0x4B, 0xCA, '\0'};
void setup(void) {
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  Serial.begin(9600);
  lcd.createChar(1, arrowUp);
  lcd.createChar(0, arrowDown);
  //lcd.createChar(1, nasos);
  lcd.begin(16, 2);
  Blink(200, 3);
  Beep(150, 3, 1500);
  analogWrite(BACKLIGHT_PIN, 100);
  lcd.setCursor(4, 0);
  lcd.print("pech-v0.3");
    delay(900);
  lcd.clear();
  for (int i = 0; i < 100; i++) {
    lcd.setCursor(0, 0);
    lcd.print(i);
    lcd.print("%");
    lcd.print(" PUMP CONTROL");
    lcd.setCursor(0, 1);
    lcd.print("made by ROBERT");
    delay(70);
  }
  lcd.clear();
  getTemp();
}
//*********LOOP************
void loop(void) {
  lcdUpdate();
  if (millis() - tempPrevMillis >= TEMP_INTERVAL) {
    getTemp();
    tempPrevMillis = millis();
    if (temp > maxTempStat) {
      maxTempStat = temp;
    }
  }
  //--------------------------------------------
  if (temp > DANGER) { //DANGER
    pumpState = 2;
  }
  else if (temp >= MAX_TEMP && temp <= DANGER) {   //MAX_TEMP
    pumpState = 1;
  }
  else {   //OFF
    pumpState = 0;
  }
  //------------------------------------------------
  if ((millis() - pumpPrevMillis > PUMP_INTERVAL) || temp > DANGER) {
    switch (pumpState)
    {
      case 0 :
        digitalWrite(PUMP_PIN, HIGH);
        lcdState = 0;
        PUMP_INTERVAL = 30000; // 30 sec
        if (downToneDone == 0) {
          BeepDown();
        }
        break;
      case 1 :
        digitalWrite(PUMP_PIN, LOW);
        lcdState = 1;
        PUMP_INTERVAL = 300000;  //5 min
        break;
      case 2 :
        digitalWrite(PUMP_PIN, LOW);
        lcdState = 2;
        PUMP_INTERVAL = 600000;  // 10 min
        break;
    }
    pumpPrevMillis = millis();
  }
}
//------------------------------------------
void lcdUpdate() {
  if (millis() < 60000 || lcdState > 0) {
    analogWrite(BACKLIGHT_PIN, 100);
  }
  else {
    analogWrite(BACKLIGHT_PIN, 5);
  }
  lcd.setCursor(2, 0);
  lcd.print("T=");
  lcd.print(temp, 1);
  lcd.print(tempChar);
  if (millis() - arrowPrevMillis > 180000) { //3 min  // arrow
    lcd.clear();  //очистка дисплея
    if (temp < lastTemp) arrowChar = 0; // Температура растёт или падает
    else if (temp > lastTemp) arrowChar = 1;
    else arrowChar = 2;
    lastTemp = temp;
    lcd.setCursor(0, 0);
    if (arrowChar == 2)
      lcd.write("=");
    else
      lcd.write((uint8_t)arrowChar);
    arrowPrevMillis = millis();
  }
  if (lcdState == 0) {  // lcdState == 0
    lcd.setCursor(2, 1);
    lcd.write(nasosOtkl);
    if (millis() - statPrevMillis >= 60000) { // 1 min
      statPrevMillis = millis();
      lcd.setCursor(0, 1);
      lcd.print("MAX=");
      lcd.print(maxTempStat);
      lcd.print(tempChar);
      lcd.print("     ");
      statPrevMillis = millis();
      delay(5000);
      lcd.clear();  //очистка дисплея
    }
  }
  else if (lcdState == 1) { //  lcdState == 1
    if (upToneDone == 0) {
      BeepUp();
      lcd.clear();  //очистка дисплея
    }
    lcd.setCursor(2, 1);
    lcd.write(nasosVkl);
  }
  else if (lcdState == 2) {   //  lcdState == 2
    if (millis() - warningPrevMillis >= 60000) { // 1 min
      warningPrevMillis = millis();
      toneDone = 0;
      Blink(400, 3);
    }
    if (toneDone == 0) {
      Beep(200, 10, 800);
    }
    lcd.setCursor(2, 1);
    lcd.print("!!!DANGER!!!");
    lcd.noDisplay();
    delay(300);
    lcd.display();
    delay(500);
    analogWrite(BACKLIGHT_PIN, 0);
    delay(300);
    analogWrite(BACKLIGHT_PIN, 20);
    delay(500);
  }
}
//*************blink**************
void Blink(int t, int i) {
  for (int x = 0; x < i; x++) {
    analogWrite(BACKLIGHT_PIN, 0);
    delay(t);
    analogWrite(BACKLIGHT_PIN, 20);
    delay(t);
  }
}
//*************beep*************
void BeepUp() {
  toneVal = 100;
  for (int x = 0; x < 50; x++) {
    tone(SOUND_PIN, toneVal);
    toneVal = toneVal + 100;
    delay(50);
  }
  noTone(SOUND_PIN);
  upToneDone = 1;
  downToneDone = 0;
  toneDone = 0;
  //lcd.clear();
}
void BeepDown() {
  toneVal = 4500;
  for (int x = 40; x >= 0; x--) {
    tone(SOUND_PIN, toneVal);
    toneVal = toneVal - 130;
    delay(50);
  }
  noTone(SOUND_PIN);
  downToneDone = 1;
  upToneDone = 0;
  toneDone = 0;
  //lcd.clear();
}
void Beep(int t, int i, int n) {
  for (int x = 0; x < i; x++) {
    tone(SOUND_PIN, n);
    delay(t);
    noTone(SOUND_PIN);
    delay(t);
  }
  toneDone = 1;
  downToneDone = 0;
  //lcd.clear();
}
//*********getTemp()**********
void getTemp() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(300);
    return;
  }
  for ( i = 0; i < 8; i++) {
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  temp = (float)raw / 16.0;
}

