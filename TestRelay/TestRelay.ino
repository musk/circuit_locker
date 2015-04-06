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
 * along with this program.  If not, see http://www.gnu.org/licenses
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
 
// define the relais pin
#define RELAIS_PIN   17

void setup() {
  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, LOW);
}

// turn relais pin HIGH wait 5 seconds then turn it LOW 
// and wait another 5 seconds
void loop() {
  digitalWrite(RELAIS_PIN, HIGH);
  delay(5000);
  digitalWrite(RELAIS_PIN, LOW);
  delay(5000);
}
