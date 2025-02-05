#include <LiquidCrystal.h>

LiquidCrystal lcd(10, 11, 2, 3, 4, 5);
const int LEFT_LED = 6;
const int RIGHT_LED = 9;
const int BUTTON = 12;
const char* topString = "Bienvenue aux JPO de l'INSA Toulouse !           ";
const int topStringLength = 42;

int chaserOffset = 0;

const int MAX_TIME = 1*60;
int remainingSeconds = MAX_TIME;
const int threshold = 50;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(LEFT_LED, OUTPUT);
    pinMode(RIGHT_LED, OUTPUT);
    lcd.begin(16, 2);
    // lcd.print("1234567890123456abcdefghijklmnopz");
    // lcd.print("1234567890123456abcdefghijklmnopz");
    lcd.setCursor(0,1);
    lcd.print("RESTANT:");
    delay(1000);
}

void writeCharAt(char c, int x, int y) {
  lcd.setCursor(x,y);
  lcd.write(c);
}

void chaserUpdate() {
  chaserOffset = (chaserOffset+1) % topStringLength;
  for (int x = 0; x < 16; x++) {
    writeCharAt(topString[(chaserOffset+x)%topStringLength],x,0);
  }
}

void timeUpdate(int seconds) {
}

void refresh() {
  int buttonState = digitalRead(BUTTON);
  if (buttonState == HIGH) {
    remainingSeconds = MAX_TIME;
  }
  
  if (remainingSeconds < threshold) {
    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, HIGH);
  } else {
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
  }
}

void waitWhileRefresh(int time, int precision) {
  //lcd.scrollDisplayLeft();

  chaserUpdate();
  for (int i=0; i<precision;i++) {
    delay(time/precision);
    refresh();
  }
}

void loop() {
  for (int i=0;i<5;i++) {
  digitalWrite(LED_BUILTIN, HIGH);
  waitWhileRefresh(200,10);
  digitalWrite(LED_BUILTIN, LOW);
  waitWhileRefresh(200,10);
  }
  if (remainingSeconds > 0)
    remainingSeconds--;
  timeUpdate(remainingSeconds);
}
