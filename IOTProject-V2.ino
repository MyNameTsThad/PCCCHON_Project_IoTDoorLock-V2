#include <WiFi.h>
#include <WiFiClient.h>
#include "BlynkEdgent.h"

#include <SPI.h>
#include <MFRC522.h>

#define SSID "Watcharapong_2.4G"
#define PASSWORD "0868190813"
#define LED_RED D0
#define LED_GREEN D1
#define LOCK D2
#define VP_LOCK V1 //define this

#define BLYNK_TEMPLATE_ID "<BLYNK_TEMPLATE_ID>" //define this
#define BLYNK_DEVICE_NAME "<BLYNK_DEVICE_NAME>" //define this
#define BLYNK_AUTH_TOKEN "<BLYNK_AUTH_TOKEN>" //define this
#define BLYNK_FIRMWARE_VERSION "0.1.0" //maybe this too idk

#define BLYNK_PRINT Serial
#define USE_NODE_MCU_BOARD

constexpr uint8_t RST_PIN = D3; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;  // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
String tag;

int VP_LOCK_VAL = 0;

void setup()
{
	Serial.begin(115200);
	BlynkEdgent.begin();
	SPI.begin();	 // Init SPI bus
	rfid.PCD_Init(); // Init MFRC522

	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LOCK, OUTPUT);
	digitalWrite(LOCK, LOW);
	Blynk.virtualWrite(VP_LOCK, 0);

	WiFi.begin(SSID, PASSWORD);
	Serial.printf("WiFi connecting to %s\n", SSID);
	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(400);
	}
	Serial.printf("\nWiFi connected\nIP : ");
	Serial.println(WiFi.localIP());
	Serial.println(LINE.getVersion());
}

BLYNK_WRITE(VP_LOCK){
  VP_LOCK_VAL = param.asInt();
}

void loop()
{
	BlynkEdgent.run();

  if (VP_LOCK_VAL == 1){
    digitalWrite(LOCK, LOW);
  } else {
    digitalWrite(LOCK, HIGH);
  }
  
	if (!rfid.PICC_IsNewCardPresent())
		return;
	if (rfid.PICC_ReadCardSerial())
	{
		for (byte i = 0; i < 4; i++)
		{
			tag += rfid.uid.uidByte[i];
		}
		Serial.println(tag);
		if ((tag == "991272462") || (tag == "2811520233"))
		{
			Serial.println("Access Granted!");
			digitalWrite(LOCK, HIGH);
			Blynk.virtualWrite(VP_LOCK, 1);
			for (byte j = 0; j < 3; j++)
			{
				digitalWrite(LED_GREEN, HIGH);
				delay(100);
				digitalWrite(LED_GREEN, LOW);
				delay(100);
			}
			delay(4400);
			digitalWrite(LOCK, LOW);
			Blynk.virtualWrite(VP_LOCK, 0);
		}
		else
		{
			Serial.println("Access Denied!");
			for (byte j = 0; j < 3; j++)
			{
				digitalWrite(LED_RED, HIGH);
				delay(100);
				digitalWrite(LED_RED, LOW);
				delay(100);
			}
			delay(900);
		}

		tag = "";
		rfid.PICC_HaltA();
		rfid.PCD_StopCrypto1();
	}
}
