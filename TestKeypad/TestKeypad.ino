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
#define ROW1 15
#define ROW2 14
#define ROW3 13
#define ROW4 12

#define COL1 11
#define COL2 10
#define COL3 9
#define COL4 8

#define uint unsigned int

const uint ROWS[] = {ROW1, ROW2, ROW3, ROW4};
const uint COLS[]  = {COL1, COL2, COL3, COL4};
const char LETTERS[][4] = {{'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


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


void setup() {
  // setup pins
  for (int i = 0; i < 4; i++) {
    pinMode(ROWS[i], OUTPUT);
    pinMode(COLS[i], INPUT);
    digitalWrite(ROWS[i], LOW);
    digitalWrite(COLS[i], LOW);
  }
  Serial.begin(9600);
  Serial.println("Initialized and ready to go!");
}

void loop() {
  char key = getPressedKey();
  if (key != -1) {
    Serial.print("Got key press: ");
    Serial.println(key);
  }
  // prevent double clicks
  delay(200);
}
