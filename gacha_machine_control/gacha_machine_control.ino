#include <WiFi.h>
#include <esp_now.h>

#define BUTTON1_PIN 5
#define BUTTON2_PIN 18
#define IN1_PIN 22
#define IN2_PIN 23
#define LED_PIN 2
#define SENSOR1_PIN 19

// 狀態變數
bool door1Unlocked = false;
bool door2Unlocked = false;
unsigned long unlockTime1 = 0;
unsigned long unlockTime2 = 0;
const unsigned long AUTO_LOCK_TIME = 5000; // 5秒自動上鎖

// 通訊結構 (必須與ESP8266相同)
typedef struct {
  int command;
  char message[32];
} LockCommand;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SENSOR1_PIN, INPUT);
  //pinMode(SENSOR2_PIN, INPUT);

  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // 設定WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // 初始化ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init fail");
    return;
  }

  Serial.println("System started");
  Serial.println("Ready");

  // 註冊接收回調函數
  esp_now_register_recv_cb(onDataReceived);
  Serial.print("ESP32 MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  
  bool button1Pressed = !digitalRead(BUTTON1_PIN);
  bool button2Pressed = !digitalRead(BUTTON2_PIN);
  bool sensor1 = digitalRead(SENSOR1_PIN);
  // bool sensor2 = digitalRead(SENSOR2_PIN);

  if (button1Pressed) {
    Serial.println("Button1 pressed");
    unlockDoor(IN1_PIN, &door1Unlocked, &unlockTime1);
    Serial.println("Door1 unlocked");
  }

  if (button2Pressed) {
    Serial.println("Button2 pressed");
    unlockDoor(IN2_PIN, &door2Unlocked, &unlockTime2);
    Serial.println("Door2 unlocked");
  }

  if (sensor1) {
    Serial.println("Sensor1 blocked");
    lockDoor(IN1_PIN, &door1Unlocked);
    Serial.println("Door1 locked");
  }

  // if (sensor2) {
  //   Serial.printIn("Sensor blocked");
  //   lockDoor(IN2_PIN, door2Unlocked);
  //   Serial.printIn("Door2 locked");
  // }

  checkAutoLock();

  delay(50);
}

void unlockDoor(int PIN, bool* doorUnlock, unsigned long* unlockTime) {
  *doorUnlock = true;
  *unlockTime = millis();

  digitalWrite(PIN, HIGH);
  updateLED();
}

void lockDoor(int PIN, bool* doorUnlock)
{
  *doorUnlock = false;

  digitalWrite(PIN, LOW);
  updateLED();
}

void updateLED() {
  if (door1Unlocked || door2Unlocked) {
    digitalWrite(LED_PIN, HIGH);   // 點亮LED
  } else {
    digitalWrite(LED_PIN, LOW);  // 熄滅LED
  }
}

void checkAutoLock() {
  unsigned long currentTime = millis();
  
  // 檢查Door1自動上鎖
  if (door1Unlocked && (currentTime - unlockTime1 >= AUTO_LOCK_TIME)) {
    Serial.println("Door1 auto-lock timeout");
    lockDoor(IN1_PIN, &door1Unlocked);
  }
  
  // 檢查Door2自動上鎖
  if (door2Unlocked && (currentTime - unlockTime2 >= AUTO_LOCK_TIME)) {
    Serial.println("Door2s auto-lock timeout");
    lockDoor(IN2_PIN, &door2Unlocked);
  }
}

// ESP-NOW接收回調函數
void onDataReceived(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
  LockCommand receivedCommand;
  memcpy(&receivedCommand, incomingData, sizeof(receivedCommand));

  Serial.print("收到指令: ");
  Serial.print(receivedCommand.command);
  Serial.print(" - ");
  Serial.println(receivedCommand.message);

  // 處理指令
  switch (receivedCommand.command) {
    case 1:
      if (!door1Unlocked) {
        Serial.println("無線解鎖門1");
        unlockDoor(IN1_PIN, &door1Unlocked, &unlockTime1);
      } else {
        Serial.println("門1已經解鎖");
      }
      break;
      
    case 2:
      if (!door2Unlocked) {
        Serial.println("無線解鎖門2");
        unlockDoor(IN2_PIN, &door2Unlocked, &unlockTime2);
      } else {
        Serial.println("門2已經解鎖");
      }
      break;
      
    default:
      Serial.println("未知指令");
      break;
  }
}