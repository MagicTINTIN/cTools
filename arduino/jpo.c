#include <LiquidCrystal.h>

// Initialize the LCD with the pins connected to RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    lcd.begin(16, 1); // 16x2 LCD
    lcd.print("Bienvenue aux JPO de l'INSA Toulouse ! Bienvenue aux JPO de l'INSA Toulouse !");
    delay(1000);
}

//void loop() {
//    lcd.scrollDisplayLeft(); // Scroll left
//    delay(300); // Delay for smooth scrolling
//}

void loop() {
  lcd.scrollDisplayLeft();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  lcd.scrollDisplayLeft();
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
}
