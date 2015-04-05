/**
 * Copyright (C) 2015  Stefan Langer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This program uses parts of the arduino core libraries published by Arduino LLC
 * which are released under the LGPL v2.1 or later.
 * http://www.gnu.org/licenses/lgpl
 *
 * All hardware, documentation and files distributed with this software that do
 * not fall under the GPL are distributed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License
 * http://creativecommons.org/licenses/by-sa/4.0
 */
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

#define ROW1 15
#define ROW2 14
#define ROW3 13
#define ROW4 12

#define COL1 11
#define COL2 10
#define COL3 9
#define COL4 8

#define RS_PIN        2
#define EN_PIN        3
#define D0_PIN        4
#define D1_PIN        5
#define D2_PIN        6
#define D3_PIN        7
#define LCD_BACKLED  16
#define RELAIS_PIN   17

#define uint unsigned int
#define ulong unsigned long

#define LOCK_GLYPH   0
#define UNLOCK_GLYPH 1

#define TIMEOUT 150 

#define SECOND    1000
#define MINUTE   60000
#define HOUR   3600000
#define DAY   86400000

byte lockGlyph[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B11111,
  B11111,
  B11111,
  B00000
};

byte unlockGlyph[8] = {
  B00000,
  B01110,
  B00001,
  B00001,
  B11111,
  B11111,
  B11111,
  B00000
};

const uint ROWS[] = {ROW1, ROW2, ROW3, ROW4};
const uint COLS[]  = {COL1, COL2, COL3, COL4};
const char LETTERS[][4] = {{'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

LiquidCrystal lcd(RS_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN);
String secret;
boolean circuitLocked, isDisplayOn;
int displayTimeout;

void printLockState() {
  if(!isDisplayOn) return;
  
  lcd.setCursor(0, 3);
  if (circuitLocked) lcd.write(byte(LOCK_GLYPH));
  else lcd.write(byte(UNLOCK_GLYPH));
}

void printActiveTime() {
  if(!isDisplayOn) return;
  
  ulong time=millis();
  ulong days = time / DAY;
  ulong hours = (time / HOUR) % 24;
  ulong minutes = (time / MINUTE) % 60;
  ulong seconds = (time / SECOND) % 60;
  
  printLockState();
  int dIdx = 1;
  if (days >= 10)
    dIdx = 0;
  lcd.setCursor(8 + dIdx, 3);
  lcd.print(days);
  lcd.print('d');
  if (hours < 10)
    lcd.print('0');
  lcd.print(hours);
  lcd.print('h');
  if (minutes < 10)
    lcd.print('0');
  lcd.print(minutes);
  lcd.print('m');
  if (seconds < 10)
    lcd.print('0');
  lcd.print(seconds);
  lcd.print('s');
}

void printOptions() {
  if(!isDisplayOn) return;
  
  lcd.clear();
  lcd.print("Options");
  lcd.setCursor(0, 1);
  lcd.print(" C - Change Pin");
  lcd.setCursor(0, 2);

  lcd.print(" D - ");
  if (circuitLocked) lcd.print("Unlock circuit");
  else lcd.print("Lock circuit");
  printActiveTime();
}

void printEnterCmd() {
  lcd.clear();
  lcd.print("Enter pin:");
  lcd.setCursor(0, 1);
  lcd.print("Press D to enter!");
  printActiveTime();
}

char getPressedKey() {
  for (int ri = 0; ri < 4; ri++) {
    digitalWrite(ROWS[ri], HIGH);
    for (int c = 0; c < 4; c++) {
      if (HIGH == digitalRead(COLS[c])) {
        return LETTERS[ri][c];
      }
    }
    digitalWrite(ROWS[ri], LOW);
  }
  return -1;
}

String enterPassword(int line) {
  String pin;
  for (;;) { /*ever*/
    char key = getPressedKey();
    switch (key) {
      case 'A':
      case 'B':
      case 'C':
      case -1:
        break;
      case 'D':
        return pin;
      case '#':
        pin.remove(0);
        lcd.setCursor(0, line);
        lcd.print("                    ");
        break;
      case '*':
        if (pin.length() > 1)
          pin.remove(pin.length() - 1);
        lcd.setCursor(pin.length(), line);
        lcd.print(" ");
        break;
      default:
        lcd.setCursor(pin.length(), line);
        pin += key;
        lcd.print(key);
        break;
    }
    printActiveTime();
    // prevent double clicks
    delay(300);
  }
}

boolean enterAndValidate(int line) {
  String pin = enterPassword(line);
  return secret.compareTo(pin) == 0;
}

void changePassword() {
  String newPin;
  lcd.clear();
  if (isSecretSet()) {
    lcd.print("Enter current pin!");
    lcd.setCursor(0, 1);
    lcd.print("Press D to enter!");
    if (enterAndValidate(2)) {
      lcd.clear();
      printEnterCmd();
      writePinToEEPROM(enterPassword(2));
      lcd.clear();
      lcd.print("New pin set and");
      lcd.setCursor(0, 1);
      lcd.print("written to memory!");
    } else {
      lcd.clear();
      lcd.print("Invalid pin entered!");
      lcd.setCursor(0, 1);
      lcd.print("Pin was not changed!");
    }
    printActiveTime();
    delay(5000);
  }
  else {
    printEnterCmd();
    writePinToEEPROM(enterPassword(2));
  }

}

void writePinToEEPROM(String pin) {
  secret = pin;
  Serial.print("Writing ");
  Serial.print(pin);
  Serial.println(" to EEPROM!");
  closeCircuit();
}

String readPinFromEEPROM() {
  String retVal = "1234";
  Serial.print("Read ");
  Serial.print(retVal);
  Serial.println(" from EEPROM!");
  return retVal;
}

boolean isSecretSet() {
  return secret.length() > 0;
}

void openCircuit() {
  circuitLocked = false;
  digitalWrite(RELAIS_PIN, HIGH);
  lcd.clear();
  lcd.print("Circuit unlocked!");
  printActiveTime();
}

void closeCircuit() {
  circuitLocked = true;
  digitalWrite(RELAIS_PIN, LOW);
  lcd.clear();
  lcd.print("Circuit locked!");
  printActiveTime();
}

void turnOffDisplay() {
  isDisplayOn = false;
  digitalWrite(LCD_BACKLED, LOW);
  lcd.noDisplay();
}

void turnOnDisplay() {
  isDisplayOn = true;
  displayTimeout = 0;
  digitalWrite(LCD_BACKLED, HIGH);
  lcd.display();
}

void setup() {
  // setup pins
  pinMode(LCD_BACKLED, OUTPUT);
  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, LOW);

  for (int i = 0; i < 4; i++) {
    pinMode(ROWS[i], OUTPUT);
    pinMode(COLS[i], INPUT);
    digitalWrite(ROWS[i], LOW);
    digitalWrite(COLS[i], LOW);
  }
  circuitLocked = true;
  displayTimeout = 0;
  secret = readPinFromEEPROM();
  lcd.createChar(LOCK_GLYPH, lockGlyph);
  lcd.createChar(UNLOCK_GLYPH, unlockGlyph);
  Serial.begin(9600);
  lcd.begin(20, 4);
  turnOnDisplay();
  lcd.print("Initialized and");
  lcd.setCursor(0, 1);
  lcd.print("ready to go!");
  Serial.println("Initialized and ready to go!");
  delay(3000);
  printOptions();
}

void loop() {
  if (isDisplayOn) {
    if (displayTimeout >= TIMEOUT) {
      turnOffDisplay();
    } else {
      displayTimeout++;
    }
  }

  char key = getPressedKey();
  switch (key) {
    case 'C':
      if (!isDisplayOn) turnOnDisplay();
      changePassword();
      printOptions();
      break;
    case 'D':
      if (!isDisplayOn) turnOnDisplay();
      if (circuitLocked) {
        printEnterCmd();
        if (enterAndValidate(2)) {
          openCircuit();
        } else {
          lcd.clear();
          lcd.print("Invalid pin entered!");
          lcd.setCursor(0, 1);
          lcd.print("Circuit remains");
          lcd.setCursor(0, 2);
          lcd.print("locked!");
          printActiveTime();
        }
      }
      else {
        closeCircuit();
      }
      delay(5000);
      printOptions();
      break;
    case -1:
      break;
    default:
      if (!isDisplayOn) turnOnDisplay();
  }
  printActiveTime();
  delay(200);
}
