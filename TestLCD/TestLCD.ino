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
 * along with this program.  If not, see http://www.gnu.org/licenses.
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
#include <LiquidCrystal.h>

#define RS_PIN        2
#define EN_PIN        3
#define D0_PIN        4
#define D1_PIN        5
#define D2_PIN        6
#define D3_PIN        7
#define LCD_BACKLED  16 // Analog pin A0

#define SECOND    1000
#define MINUTE   60000
#define HOUR   3600000
#define DAY   86400000

#define ulong unsigned long

LiquidCrystal lcd(RS_PIN, EN_PIN, D0_PIN, D1_PIN, D2_PIN, D3_PIN);

void printActiveTime() {
  ulong time = millis();
  ulong days = time / DAY;
  ulong hours = (time / HOUR) % 24;
  ulong minutes = (time / MINUTE) % 60;
  ulong seconds = (time / SECOND) % 60;

  int dIdx = 1;
  if (days >= 10)
    dIdx = 0;
  lcd.setCursor(2 + dIdx, 2);
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

void setup() {
  // setup pins
  pinMode(LCD_BACKLED, OUTPUT);

  digitalWrite(LCD_BACKLED, HIGH);

  lcd.begin(20, 4);

  lcd.print("Setup and Running!");
  lcd.setCursor(0, 1);
  lcd.print("Running since:");
}

void loop() {
  printActiveTime();
}
