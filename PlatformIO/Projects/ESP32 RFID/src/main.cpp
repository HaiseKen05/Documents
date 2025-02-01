#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

// RFID Module Pin
#define RST_PIN 14
#define SS_PIN 5
#define LOCK_PIN 4

// Lock Delay
const int lock_delay = 500;

// RFID Setup
MFRC522 mfrc522(SS_PIN, RST_PIN);
// LCD Setup
hd44780_I2Cexp lcd(0x27, 20, 4);

String read_rfid;
const String valid_rfid[] = {"e45bd4", "263e6245"};


// WiFi Configuration
const char *ssid = "SM_FreeWIFI";
const char *password = "HaiseKen05$$";


// NTP Server Configuration
const char *ntpServer = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600 * 8;
const int daylightOffset_sec = 3600;

// Timezone Configuration
const char *time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Location
const char *location = " ";

// Stores last displayed date and time
char lastDisplayedDate[20] = "";
char lastDisplayedTime[20] = "";


// RFID Module Pin
#define RST_PIN 14
#define SS_PIN 5
#define LOCK_PIN 4



void printLocalTime(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed to obtain time");
    Serial.println("Failed to obtain time");
    return;
  }
    char currentDate[20];
    char currentTime[20];
    strftime(currentDate, sizeof(currentDate), "%B %d %Y", &timeinfo);
    strftime(currentTime, sizeof(currentTime), "%A %H:%M:%S", &timeinfo);
  // Update Date and Time
  if (strcmp(lastDisplayedDate, currentDate) != 0) {
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 0);
    lcd.print(currentDate);
    strncpy(lastDisplayedDate, currentDate, sizeof(lastDisplayedDate));
  }
  if (strcmp(lastDisplayedTime, currentTime) != 0) {
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(currentTime);
    strncpy(lastDisplayedTime, currentTime, sizeof(lastDisplayedTime));
  }

  lcd.setCursor(0, 2);
  lcd.print(" ");
  lcd.setCursor(0, 2);
  lcd.print(location);
}

void timeSyncCallback(struct timeval *tv) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time Synchronized");
  Serial.println("Time Synchronized");
  printLocalTime();
}

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(String("Connecting to WiFi: ") + ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected to WiFi");
}
  void dumpbyteArray(byte *buffer, byte bufferSize) {
    read_rfid = "";
    for (byte i = 0; i < bufferSize; i++) {
      read_rfid = read_rfid + String(buffer[i], HEX);
    }
  }

  void openLock() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Granted");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome Admin");

    digitalWrite(LOCK_PIN, HIGH);
    delay(lock_delay);
    digitalWrite(LOCK_PIN, LOW);

  }

  void AccessDenied() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Please Try Again");
  }
  
  bool isValidRFID(){
    for (String uid : valid_rfid){
      if (uid == read_rfid){
        return true;
      }
    }
    return false;
  }

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.begin(20, 4);
  lcd.setBacklight(1);

  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);

  connectToWiFi();

  esp_sntp_servermode_dhcp(1);
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, ntpServer2);
}

void loop() {

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;
  dumpbyteArray(mfrc522.uid.uidByte, mfrc522.uid.size);
  
  if (isValidRFID()){
    openLock();
  } else {
    AccessDenied();
  }
  delay (1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  printLocalTime();
  delay(1000);
}

