#include <ESP8266WiFi.h>

#define BUTTON1_PIN 12
#define BUTTON2_PIN 14
#define IN1_PIN 13
#define IN2_PIN 15
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  yield();
  delay(1000);
  yield();

  WiFi.mode(WIFI_OFF);
  yield();

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("System started");
  Serial.println("Ready");

  yield();
}

void loop() {
  yield();
  
  bool button1Pressed = !digitalRead(BUTTON1_PIN);
  bool button2Pressed = !digitalRead(BUTTON2_PIN);

  if (button1Pressed) {
    Serial.println("Button1 pressed");
    unlockDoor(IN1_PIN);
  }

  if (button2Pressed) {
    Serial.println("Button2 pressed");
    unlockDoor(IN2_PIN);
  }

  delay(50);
  yield();
}

void unlockDoor(int PIN) {
  digitalWrite(PIN, HIGH);
  digitalWrite(LED_PIN, LOW);

  // 分段延遲避免看門狗問題
  for (int i = 0; i < 30; i++) {
    delay(100);
    yield();
  }

  digitalWrite(PIN, LOW);
  digitalWrite(LED_PIN, HIGH);

  Serial.println("Locked");
  
  for (int i = 0; i < 10; i++) {
    delay(100);
    yield();
  }
}