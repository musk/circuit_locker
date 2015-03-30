#define ROW1 13
#define ROW2 12
#define ROW3 11
#define ROW4 10

#define COL1 9
#define COL2 8
#define COL3 7
#define COL4 6

const int rows[] = {ROW1, ROW2, ROW3, ROW4};
const int cols[] = {COL4, COL3, COL2, COL1};
const char letters[][4] = {{'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
//const String digits[] PROGMEM = { "1", "2", "3", "4"};

void setup() {

  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);

  digitalWrite(3, LOW);
  digitalWrite(4, LOW);

  for (int i = 0; i < 4; i++) {
    pinMode(rows[i], OUTPUT);
    pinMode(cols[i], INPUT);
    digitalWrite(rows[i], LOW);
    digitalWrite(cols[i], LOW);
  }
  Serial.begin(9600);
}
int checkRow(int rowPin) {
  int retVal = -1;
  digitalWrite(rowPin, HIGH);
  for (int c = 0; c < 4; c++) {
    if (HIGH == digitalRead(cols[c])) {
      retVal = c;
      break;
    }
  }
  digitalWrite(rowPin, LOW);
  return retVal;
}

void loop() {
  for(int i=0;i<4;i++) {
      int retVal=checkRow(rows[i]);
      if(retVal != -1) {
        Serial.println(letters[i][retVal]);
        digitalWrite(4, HIGH);
        delay(200);
        digitalWrite(4, LOW);
      }
  }

  digitalWrite(3, HIGH);
  delay(100);
  digitalWrite(3, LOW);
}
