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

#define LCD_BACKLED  16

#define RS_PIN  2
#define EN_PIN  3
#define D0_PIN  4
#define D1_PIN  5
#define D2_PIN  6
#define D3_PIN  7

#define uint unsigned int
#define ulong unsigned long

#define LOCK_GLYPH 0
#define UNLOCK_GLYPH 1

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
boolean circuitLocked = true;

char getPressedKey() {
  for (int ri = 0; ri < 4; ri++) {
    digitalWrite(ROWS[ri], HIGH);
    for (int c = 0; c < 4; c++) {
      if (HIGH == digitalRead(COLS[c])) {
        return LETTERS[ri][c];
      }
    }
    digitalWrite(rows[ri], LOW);
  }
  return -1;
}

String enterPassword(int line) {
  String pin;
  for (;;) { /*ever*/
    key = getPressedKey();
    switch (key) {
      case 'A':
      case 'B':
      case 'C':
      case -1:
        continue;
      case 'D':
        return pin;
      case '#':
        pin.remove(0);
      case '*':
        if (pin.length() > 1)
          pin.remove(pin.length() - 1);
        break;
      default:
        pin += key;
        break;
    }
    printActiveTime();
    lcd.setCursor(0,line);
    lcd.print(pin);
    // prevent double clicks
    delay(200);
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
    lcd.setCursor(0,1);
    lcd.print("Press D when done!");
    if (enterAndValidate(2)) {
      lcd.clear();
      lcd.print("Enter new pin!");
      writePinToEEPROM(enterPassword(1));
    } else {
      lcd.setCursor(0,1);
      lcd.print("Invalid pin entered!");
      lcd.setCursor(0,2);
      lcd.print("Pin was not changed!");
      printActiveTime();
    }
  }
  else {
    lcd.print("Enter new pin!");
    writePinToEEPROM(enterPassword(1));
  }

}

void writePinToEEPROM(String pin) {
  secret = pin;
  Serial.print("Writing ");
  Serial.print(pin);
  Serial.println(" to EEPROM!");
}

String readPinFromEEPROM() {
  String retVal = "1234567890";
  Serial.print("Read ");
  Serial.print(retVal);
  Serial.println(" from EEPROM!");
  return retVal;
}

boolean isSecretSet() {
  return secret.length() > 0;
}

void printLockState() {
  lcd.setCursor(0, 3);
  if (circuitLocked) lcd.write(LOCK_GLYPH);
  else lcd.write(UNLOCK_GLYPH);
}

void printActiveTime() {
  ulong seconds = (millis() / 1000) % 60;
  ulong minutes = (millis() / 60000) % 60;
  ulong hours = (millis() / 3600000) % 60;
  ulong days = (millis() / 86400000) % 24;

  printLockState();
  lcd.setCursor(8, 3);
  int dIdx = 1;
  if (days >= 10)
    dIdx = 0;
  lcd.setCursor(6 + dIdx, 3);
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

void openCircuit() {
  Serial.println("Opening circuit!");
  circuitLocked = false;
  printLockState();
}

void closeCircuit() {
  Serial.println("Closing circuit!");
  circuitLocked = true;
  printLockState();
}

void setup() {
  // setup pins
  pinMode(LCD_BACKLED, OUTPUT);
  // activate the backlight leds for lcd
  digitalWrite(LCD_BACKLED, HIGH);

  for (int i = 0; i < 4; i++) {
    pinMode(rows[i], OUTPUT);
    pinMode(cols[i], INPUT);
    digitalWrite(rows[i], LOW);
    digitalWrite(cols[i], LOW);
  }
  lcd.createChar(LOCK_GLYPH, lockGlyph);
  lcd.createChar(UNLOCK_GLYPH, unlockGlyph);
  Serial.begin(9600);
  lcd.begin(20, 4);
  lcd.print("Initialized and\nready to go!");
  Serial.println("Initialized and ready to go!");
  delay(2000);
}

void loop() {
  lcd.clear();
  lcd.print("Options");
  lcd.setCursor(0, 1);
  lcd.print(" C - Change Pin");
  lcd.setCursor(0, 2);
  lcd.print(" D - Un-/Lock");
  printActiveTime();

  char key = getPressedKey();
  switch (key) {
    case 'C':
      changePassword();
      break;
    case 'D':
      if (circuitLocked) {
        lcd.clear();
        lcd.print("Enter pin!");
        if (enterAndValidate(1)) {
          openCircuit();
        }
      }
      else {
        closeCircuit();
      }
      break;
  }
  delay(200);
}
