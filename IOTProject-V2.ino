#include <SPI.h>
#include <MFRC522.h>

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPLkdJn7DAB"
#define BLYNK_DEVICE_NAME "IoT Door Lock"
#define LED_RED     D0
#define LED_GREEN   D1
#define LOCK        D2
#define VPIN        V0

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"

int DoorState = 0;
BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN);
}
BLYNK_WRITE(V0){
  DoorState = param.asInt();
  }

constexpr uint8_t RST_PIN = D3;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;     // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
String tag;

void setup()
{
  pinMode(LOCK, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  Serial.begin(115200);
  delay(100);
  digitalWrite(LOCK, LOW);
  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  BlynkEdgent.begin();
}

void loop() {
  BlynkEdgent.run();
  if(DoorState==1){
    digitalWrite(LOCK, HIGH);
    digitalWrite(LED_GREEN, HIGH);
  }
  else{
    digitalWrite(LOCK, LOW);
    digitalWrite(LED_GREEN, LOW);
  }
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if (rfid.PICC_ReadCardSerial()) {
    for (byte i = 0; i < 4; i++) {
      tag += rfid.uid.uidByte[i];
    Serial.println(tag);
    }
    if ((tag == "991272462")||(tag == "2811520233")) {
      Serial.println("Access Granted!");
      digitalWrite(LOCK, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      Blynk.logEvent("lock_notification", "Door unlocked");
      delay(100);
      digitalWrite(LED_GREEN, LOW);
      delay(100);
      digitalWrite(LED_GREEN, HIGH);
      delay(100);
      digitalWrite(LED_GREEN, LOW);
      delay(100);
      digitalWrite(LED_GREEN, HIGH);
      delay(100);
      digitalWrite(LED_GREEN, LOW);
      delay(4500);
      digitalWrite(LOCK, LOW);
  } 
    else {
        Serial.println("Access Denied!");
        Blynk.logEvent("wrong_rfid", "Wrong card detected");
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);
        delay(100);
    }

    tag = "";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}
