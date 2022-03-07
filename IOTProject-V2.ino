#include <SPI.h>
#include <MFRC522.h>

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL665riMbS"
#define BLYNK_DEVICE_NAME "IoT Door Lock"
#define LED_RED D0
#define LED_GREEN D1
#define LOCK D2

#define VPIN1 V0
#define VPIN2 V1

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"

int doorState = 0;
int cardState = 0;
BLYNK_CONNECTED()
{
	Blynk.syncVirtual(VPIN1);
	Blynk.syncVirtual(VPIN2);
}
BLYNK_WRITE(VPIN1)
{
	doorState = param.asInt();
}
BLYNK_WRITE(VPIN2)
{
	cardState = param.asInt();
}

constexpr uint8_t RST_PIN = D3; // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = D4;	// Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);	// Instance of the class
MFRC522::MIFARE_Key key;

String tags[20];

void setup()
{
	pinMode(LOCK, OUTPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	Serial.begin(115200);
	delay(100);
	digitalWrite(LOCK, LOW);

	SPI.begin();	 // Init SPI bus
	rfid.PCD_Init(); // Init MFRC522

	BlynkEdgent.begin();
}

void loop()
{
	BlynkEdgent.run();
	if (doorState == 1)
	{
		digitalWrite(LOCK, HIGH);
		digitalWrite(LED_GREEN, HIGH);
	}
	else
	{
		digitalWrite(LOCK, LOW);
		digitalWrite(LED_GREEN, LOW);
	}
	if (!rfid.PICC_IsNewCardPresent())
		return;

	if (rfid.PICC_ReadCardSerial() && cardState != 3)
	{
		String currentTag;
		for (byte i = 0; i < 4; i++)
		{
			currentTag += rfid.uid.uidByte[i];
			Serial.println(currentTag);
		}

		bool isFound = false;
		for (uint8_t i = 0; i < sizeof(tags); i++)
		{
			if (currentTag == tags[i])
			{
				isFound = true;
				Serial.println("Card = " + currentTag);
				break;
			}
		}

		if (isFound)
		{
			Serial.println("Access Granted!");
			digitalWrite(LOCK, HIGH);
			digitalWrite(LED_GREEN, HIGH);
			Blynk.logEvent("lock_notification", "Door unlocked");
			Blynk.virtualWrite(VPIN2, 1);
			Blynk.setProperty(VPIN2, "label", "CARD REGISTERED");

			delay(100);
			digitalWrite(LED_GREEN, LOW);
			for (byte i = 0; i < 2; i++)
			{
				delay(100);
				digitalWrite(LED_GREEN, HIGH);
				delay(100);
				digitalWrite(LED_GREEN, LOW);
			}
			delay(4500);
			digitalWrite(LOCK, LOW);
			Blynk.virtualWrite(VPIN2, 0);
			Blynk.setProperty(VPIN2, "label", "IDLE");
		}
		else
		{
			Serial.println("Access Denied!");
			Blynk.logEvent("wrong_rfid", "Wrong card detected");
			Blynk.virtualWrite(VPIN2, 2);
			Blynk.setProperty(VPIN2, "label", "UNKNOWN CARD");

			for (byte i = 0; i < 3; i++)
			{
				digitalWrite(LED_RED, HIGH);
				delay(100);
				digitalWrite(LED_RED, LOW);
				delay(100);
			}

			Blynk.virtualWrite(VPIN2, 0);
			Blynk.setProperty(VPIN2, "label", "IDLE");
		}

		currentTag = "";
		rfid.PICC_HaltA();
		rfid.PCD_StopCrypto1();
	}
	else if (rfid.PICC_ReadCardSerial() && cardState == 3)
	{
		// edit mode
		Blynk.setProperty(VPIN2, "label", "EDIT MODE");
		String currentTag;
		for (byte i = 0; i < 4; i++)
		{
			currentTag += rfid.uid.uidByte[i];
			Serial.println(currentTag);
		}
		bool isFound = false;
		// check if the card is already in the list, remove it. if not, add it.
		for (uint8_t i = 0; i < sizeof(tags); i++)
		{
			if (currentTag == tags[i])
			{
				for (uint8_t j = i; j < sizeof(tags); ++j)
					tags[j] = tags[j + 1];
				Serial.println("Card Removed: " + currentTag);
				isFound = true;
			}
		}
		if (!isFound)
		{
			tags[sizeof(tags)] = currentTag;
			Serial.println("Card Added: " + currentTag);
		}
	}
}
