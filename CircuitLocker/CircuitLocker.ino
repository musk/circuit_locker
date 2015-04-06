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
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

// keypad row pins
#define ROW1 15
#define ROW2 14
#define ROW3 13
#define ROW4 12

// keypad column pins
#define COL1 11
#define COL2 10
#define COL3 9
#define COL4 8

// pins for the LCD
#define RS_PIN        2
#define EN_PIN        3
#define D0_PIN        4
#define D1_PIN        5
#define D2_PIN        6
#define D3_PIN        7
#define LCD_BACKLED  16
// the relay pin
#define RELAIS_PIN   17

// aliases for datatypes
#define uint unsigned int
#define ulong unsigned long

// CONSTANTS used to store the customized lock characters to the LCD
#define LOCK_GLYPH   0
#define UNLOCK_GLYPH 1

// cycles to count befor shutting off display
#define TIMEOUT 150 // approximately 30 seconds

// CONSTANTS for time management
#define SECOND    1000 // millis in one second
#define MINUTE   60000 // millis in one minute
#define HOUR   3600000 // millis in one hour
#define DAY   86400000 // millis in one day

// defines the locked lock character
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

// defines the unlocked lock character
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

/**
 * Prints the lock state to the first character of the last line.
 * The lock state is represented by a lock which is either closed
 * or open depending on whether the circuit is locked or unlocked.
 * <em>This method is only executed when the display is turned on</em>
 */
void printLockState() {
  if (!isDisplayOn) return;

  lcd.setCursor(0, 3);
  if (circuitLocked) lcd.write(byte(LOCK_GLYPH));
  else lcd.write(byte(UNLOCK_GLYPH));
}

/**
 * Prints the status line <pre>
 * <LockStatus>   0d00h00m00s
 * </pre>
 * to the last line of the LCD.
 * <em>This method is only executed when the display is turned on</em>
 */
void printStatusLine() {
  if (!isDisplayOn) return;

  ulong time = millis();
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

/**
 * Prints the option text <pre>
 * Options:
 *  C - Change Pin
 *  D - Un/Lock circuit
 * </pre>
 * plus the status line to the LCD.
 * <em>This method is only executed when the display is turned on</em>
 */
void printOptions() {
  if (!isDisplayOn) return;

  lcd.clear();
  lcd.print("Options");
  lcd.setCursor(0, 1);
  lcd.print(" C - Change Pin");
  lcd.setCursor(0, 2);

  lcd.print(" D - ");
  if (circuitLocked) lcd.print("Unlock circuit");
  else lcd.print("Lock circuit");
  printStatusLine();
}

/**
 * Prints the text <pre>
 * Enter pin:
 * Press D to enter!
 * </pre>
 * plus the status line to the LCD.
 */
void printEnterCmd() {
  lcd.clear();
  lcd.print("Enter pin:");
  lcd.setCursor(0, 1);
  lcd.print("Press D to enter!");
  printStatusLine();
}

/**
 * Returns the currently pressed key.
 * To find out the current key press we iterate through the row pins
 * and set each pin to HIGH. While the pin is HIGH we iterate through
 * the column pins and read their signal. When we find the column
 * that reads HIGH we get the pressed letter by using the index of the
 * current row and the index of the current column.
 * <code>key = LETTERS[rowIdx][colIdx]</code>
 *
 * @return the pressed key or -1 if
 *         no key press was registered.
 */
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

/**
 * Reads in a numeric password from the keypad using {@link getPressedKey}.
 * 'D' stops entry and returns the currently entered password-
 * '*' deletes the last entered character
 * '#' clears the complete entry
 * All other keys are append to the password.
 * Additionally updates the status in the LCD using {@link printStatusLine}.
 *
 * @param line the line in the LCD to echo the entered key presses to.
 *
 * @return the entered password
 */
String enterPassword(int line) {
  String pin = "";
  resetKeypad();
  for (;;) { /*ever*/
    char key = getPressedKey();
    switch (key) {
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
    printStatusLine();
    // prevent double clicks
    delay(200);
  }
}

/**
 * Asks for a password and validates it against the currently set one.
 *
 * @param line the line in the LCD to echo the password to
 *
 * @return <code>true</code> when password matches <code>false</code> otherwise.
 */
boolean enterAndValidate(int line) {
  String pin = enterPassword(line);
  return secret.compareTo(pin) == 0;
}

/**
 * Initiates the password change algorithm:
 * Check whether there is a currently set password
 * - Password is not set
 *   a)  Read the new password from keypad using {@link enterPassword(int)}
 *   b)  Store the password to EEPROM using {@linnk changeAndStorePassword(String)}
 *   c)  Print status message to LCD and exit
 * - A password is set
 *   a)  Ask for the set password and validate it using {@link enterAndValidate(int)}
 *   b)  Password is correct
 *       - Read the new password from keypad using {@link enterPassword(int)}
 *       - Store the password to EEPROM using {@linnk changeAndStorePassword(String)}
 *       - Print status message to LCD and exit
 *   c)  Password is incorrect
 *       - Print status message to LCD and exit without changing the pin
 */
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
      changeAndStorePassword(enterPassword(2));
    } else {
      lcd.clear();
      lcd.print("Invalid pin entered!");
      lcd.setCursor(0, 1);
      lcd.print("Pin was not changed!");
    }
    printStatusLine();
    delay(5000);
  }
  else {
    printEnterCmd();
    changeAndStorePassword(enterPassword(2));
  }
}

/**
 * Set the specified pin as the new password and writes it to EEPROM
 * to ensure its persistents accross restarts.
 * Before writing it to EEPROM the user is asked whether he really wants to change
 * the password.
 * On success the circuit is locked.
 */
void changeAndStorePassword(String pwd) {
  lcd.clear();
  lcd.print("Change pin to");
  lcd.setCursor(0, 1);
  lcd.print(pwd);
  lcd.print("?");
  lcd.setCursor(0, 2);
  lcd.print("Yes - Press A");
  lcd.setCursor(0, 3);
  lcd.print("No - Press other key");
  resetKeypad();
  for (;;) { /*ever*/
    switch (getPressedKey()) {
      case 'A':
        secret = pwd;
        writeToEEPROM(pwd);
        lockCircuit();
        lcd.clear();
        lcd.print("New pin set and");
        lcd.setCursor(0, 1);
        lcd.print("written to memory!");
        return;
      case -1:
        break;
      default:
        lcd.clear();
        lcd.print("Pin was not changed!");
        lcd.setCursor(0, 1);
        printStatusLine();
        delay(5000);
        return;
    }
  }
}

/**
 * Writes the specified password to EEPROM.
 * METHOD IS CURRENTLY NOT IMPLEMENTED!
 *
 * @param pwd Password to store in EEPROM
 */
void writeToEEPROM(String pwd) {
  Serial.print("Writing ");
  Serial.print(pwd);
  Serial.println(" to EEPROM!");
}

/**
 * Initialize the password from EEPROM.
 * METHOD IS CURRENTLY NOT IMPLEMENTED!
 *
 * @return The password or empty string when no password has been stored yet.
 */
String initializePasswordFromEEPROM() {
  String retVal = "1234";
  Serial.print("Read ");
  Serial.print(retVal);
  Serial.println(" from EEPROM!");
  return retVal;
}

/**
 * Returns whether a password has been set.
 * Password has not been set when the internal secret is the empty string.
 *
 * @return <code>true</code> when a secret is set <code>false</code> otherwise.
 */
boolean isSecretSet() {
  return secret.length() > 0;
}

/**
 * Opens the relay by setting relay pin to HIGH, unlocking the circuit.
 * Prints status to LCD.
 */
void unlockCircuit() {
  circuitLocked = false;
  digitalWrite(RELAIS_PIN, HIGH);
  lcd.clear();
  lcd.print("Circuit unlocked!");
  printStatusLine();
}

/**
 * Closes the relay by setting relay pin to LOW, locking the circuit.
 * Prints status to LCD.
 */
void lockCircuit() {
  circuitLocked = true;
  digitalWrite(RELAIS_PIN, LOW);
  lcd.clear();
  lcd.print("Circuit locked!");
  printStatusLine();
}

/**
 * Turns off the backlight led by setting the backlight pin to LOW
 * and turns off the display using {@link LiquidCrystal#noDisplay()}.
 */
void turnOffDisplay() {
  isDisplayOn = false;
  digitalWrite(LCD_BACKLED, LOW);
  lcd.noDisplay();
}

/**
 * Turns on the backlight led by setting the backlight pin to HIGH
 * and turns on the display using {@link LiquidCrystal#display()}.
 */
void turnOnDisplay() {
  isDisplayOn = true;
  displayTimeout = 0;
  digitalWrite(LCD_BACKLED, HIGH);
  lcd.display();
}

/**
 * Sets all pins for the keypad to LOW.
 */
void resetKeypad() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ROWS[i], LOW);
    digitalWrite(COLS[i], LOW);
  }
  delay(100);
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
  // read previously stored password from EEPROM
  secret = initializePasswordFromEEPROM();
  // create the special lock symbols for LCD
  lcd.createChar(LOCK_GLYPH, lockGlyph);
  lcd.createChar(UNLOCK_GLYPH, unlockGlyph);
  // Initialize serial monitor for debugging
  Serial.begin(9600);
  // Initialize lcd and turn on display
  lcd.begin(20, 4);
  turnOnDisplay();
  // print status to lcd
  lcd.print("Initialized and");
  lcd.setCursor(0, 1);
  lcd.print("ready to go!");
  // print debug information to serial monitor
  Serial.println("Initialized and ready to go!");
  // wait 3 seconds and print the option menu
  delay(3000);
  printOptions();
}

void loop() {
  // when display is on check whether the display timeout counter has 
  // reached TIMEOUT if it has reached TIMEOUT turn off the display 
  // when not then increment the display timeout counter.
  if (isDisplayOn) {
    if (displayTimeout >= TIMEOUT) {
      turnOffDisplay();
    } else {
      displayTimeout++;
    }
  }
  // see if we have a key press 
  // when C is pressed initiate changePassword procedure.
  // when D is pressed initiate un/lock procedure.
  // if display is off activate it on any keypress
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
          unlockCircuit();
        } else {
          lcd.clear();
          lcd.print("Invalid pin entered!");
          lcd.setCursor(0, 1);
          lcd.print("Circuit remains");
          lcd.setCursor(0, 2);
          lcd.print("locked!");
          printStatusLine();
        }
      }
      else {
        lockCircuit();
      }
      delay(5000);
      printOptions();
      break;
    case -1:
      break;
    default:
      if (!isDisplayOn) turnOnDisplay();
  }
  // print status to last line of LCD and wait a 
  // short while to prevent double key presses
  printStatusLine();
  delay(200);
}
