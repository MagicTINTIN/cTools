#include <LiquidCrystal.h>

// Initialize the LCD with the pins connected to RS, E, D4, D5, D6, D7
LiquidCrystal lcd(10, 11, 2, 3, 4, 5);
const int LEFT_LED = 6;
const int RIGHT_LED = 9;
const int BUTTON = 12;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(LEFT_LED, OUTPUT);
    pinMode(RIGHT_LED, OUTPUT);
    lcd.begin(16, 1); // 16x2 LCD
    lcd.print("Bienvenue aux JPO de l'INSA Toulouse ! Bienvenue aux JPO de l'INSA Toulouse !");
    delay(1000);
}

void refresh() {
  int buttonState = digitalRead(BUTTON);
  if (buttonState == HIGH) {
    digitalWrite(LEFT_LED, HIGH);
    digitalWrite(RIGHT_LED, HIGH);
  } else {
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
  }
}

void waitWhileRefresh(int time, int precision) {
  lcd.scrollDisplayLeft();
  for (int i=0; i<precision;i++) {
    delay(time/precision);
    refresh();
  }
}

void loop() {
  
  digitalWrite(LED_BUILTIN, HIGH);
  waitWhileRefresh(200,10);
  digitalWrite(LED_BUILTIN, LOW);
  waitWhileRefresh(200,10);
}
