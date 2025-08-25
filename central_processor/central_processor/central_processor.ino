#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Keypad.h>

// 定義腳位
#define ROW1_PIN 14
#define ROW2_PIN 12
#define ROW3_PIN 13
#define ROW4_PIN 0

#define COL1_PIN 4
#define COL2_PIN 5
#define COL3_PIN 16

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// 修正：改為 byte 型別
byte rowPins[ROWS] = {ROW1_PIN, ROW2_PIN, ROW3_PIN, ROW4_PIN};
byte colPins[COLS] = {COL1_PIN, COL2_PIN, COL3_PIN};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

uint8_t esp32Address[] = {0x00, 0x4B, 0x12, 0x95, 0x4F, 0x3C};

// 通訊結構
typedef struct {
  int command;  // 1=解鎖門1, 2=解鎖門2, 0=重置所有鎖
  char message[32];
} LockCommand;

// 輸入相關變數
String inputCode = "";
unsigned long lastKeyTime = 0;
const unsigned long INPUT_TIMEOUT = 5000;  // 5秒輸入超時

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 設定WiFi為Station模式
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // 初始化ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // 設定發送回調函數
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(onDataSent);

  // 添加對等設備 (ESP32)
  esp_now_add_peer(esp32Address, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  Serial.print("Ready");

  // 顯示本機MAC地址
  Serial.print("ESP8266 MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
    char key = keypad.getKey();
  
  if (key) {
    handleKeyInput(key);
  }

  // 檢查輸入超時
  if (inputCode.length() > 0 && (millis() - lastKeyTime > INPUT_TIMEOUT)) {
    Serial.println("Input timeout");
    clearInput();
  }

  yield();
  delay(10);
}

void handleKeyInput(char key) {
  lastKeyTime = millis();
  
  Serial.print("Key: ");
  Serial.println(key);

  if (key == '*') {
    // 清除輸入
    clearInput();
    return;
  }

  if (key == '#') {
    // 處理輸入完成
    processCommand();
    return;
  }

  // 添加數字到輸入
  if (key >= '0' && key <= '9') {
    inputCode += key;
    Serial.print("Input: ");
    for (int i = 0; i < inputCode.length(); i++) {
      Serial.print("*");  // 顯示星號保護密碼
    }
    Serial.println();

    // 限制輸入長度
    if (inputCode.length() > 10) {
      Serial.println("Out of length");
      clearInput();
    }
  }
}

void processCommand() {
  Serial.print("Process command: ");
  Serial.println(inputCode);

  LockCommand command;
  
  if (inputCode == "01") {
    command.command = 1;
    strcpy(command.message, "Unlock Door 1");
    sendCommand(command);
    Serial.println("Unlock Door 1 Command");
  }
  else if (inputCode == "02") {
    command.command = 2;
    strcpy(command.message, "Unlock Door 2");
    sendCommand(command);
    Serial.println("Unlock Door 2 Command");
  }
  else {
    Serial.println("無效指令");
  }

  clearInput();
}

void sendCommand(LockCommand command) {
  esp_now_send(esp32Address, (uint8_t*)&command, sizeof(command));
}

void clearInput() {
  inputCode = "";
  Serial.println("clear");
}

// ESP-NOW發送回調函數
void onDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0) {
    Serial.println("Sent success");
  } else {
    Serial.println("Sent fail");
  }
}