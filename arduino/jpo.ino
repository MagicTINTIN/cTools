#include <LiquidCrystal.h>

LiquidCrystal lcd(10, 11, 2, 3, 4, 5);
const int LEFT_LED = 6;
const int RIGHT_LED = 9;
const int BUTTON = 12;
const char* topString = "Bienvenue aux JPO de l'INSA Toulouse !           ";
const int topStringLength = 42;

int chaserOffset = 0;

int lastReset = millis() / 1000;
const int MAX_TIME = 1*60+10;
int remainingSeconds = MAX_TIME;
const int threshold = 60;

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

void timeUpdate() {
  remainingSeconds = MAX_TIME - ((millis()/1000) - lastReset); // 
  if (remainingSeconds < 0) remainingSeconds = 0;
  // timeUpdate(remainingSeconds);
  int minutes_o = remainingSeconds / 60;
  int seconds_o = remainingSeconds % 60;
  char timeDisplayed[] = {'0'+minutes_o / 10, '0' + minutes_o % 10, ':', '0'+ seconds_o / 10, '0'+seconds_o % 10};
  for (int i=0;i<5;i++) {
    lcd.setCursor(11+i,1);
    lcd.print(timeDisplayed[i]);
  }
}

void refresh() {
  int buttonState = digitalRead(BUTTON);
  if (buttonState == HIGH) {
    lastReset = millis() / 1000;
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
  timeUpdate();
  for (int i=0; i<precision;i++) {
    delay(time/precision);
    refresh();
  }
}

void loop() {
  for (int i=0;i<2;i++) {
  digitalWrite(LED_BUILTIN, HIGH);
  waitWhileRefresh(250,10);
  digitalWrite(LED_BUILTIN, LOW);
  waitWhileRefresh(250,10);
  }
}
