/*
  Copyright (c) 2025 Antony Rinaldi
*/

//ESP32 Stuff
#include <Arduino.h>
#include "PN5180iClass.h"
#include <WiFi.h>
#include <string>

#include "secrets.hpp"
#include <ctime>
#include "format.hpp"
#include "piezo.hpp"



//Specify pin numbers
#define ONBOARD_LED 2
#define IO_LED 15

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#ifndef WIFI_TIMEOUT
#define WIFI_TIMEOUT 10.0
#endif

using namespace std;

// Pins
const uint8_t PN5180_NSS  = 5;   //  NSS
const uint8_t PN5180_BUSY = 17;  // BUSY
const uint8_t PN5180_RST  = 16;  // RESET

PN5180iClass reader(PN5180_NSS, PN5180_BUSY, PN5180_RST);
Piezo pz;


uint64_t last_scan = 0x0;
time_t last_scan_time;
uint64_t last_detected = 0x0;

bool genCardPseudoID(uint64_t *pseudoID) {
    iClassErrorCode rc;

    uint8_t raw[8]; //raw response
    rc = reader.Identify(raw);

    if (rc != ICLASS_EC_OK) return false;

    uint64_t hash = raw[0];
    for (int i = 1; i < 8; i++) {
      hash = hash << 8;
      hash += raw[i];
    }
    *pseudoID = hash;
    Serial.println(format("20D; %02x%02x%02x%02x%02x%02x%02x%02x", raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7]).c_str());
    return true;
} //0609600230

void on_scan(uint64_t id) {
  Serial.println(format("20B; %020llu; ", id).c_str());
}

void setup() { 
  //Open serial connection
  Serial.begin(115200);
  Serial.println("10A; Serial opened");
  pz.begin(14, 0);
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(IO_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  digitalWrite(IO_LED, HIGH);
  pz.playTone(400, 500);
  pz.playTone(1000, 500);
  pz.playTone(1800, 500);
  pz.playTone(3500, 500);
  pz.playTone(5000, 500);

  //WiFi connection
  // const char* ssid = ((string) WIFI_SSID).c_str();
  // const char* password = ((string) WIFI_PASSWORD).c_str();
  char* ssid = WIFI_SSID;
  char* password = WIFI_PASSWORD;

  Serial.println(format("30A; connecting to %s", ssid).c_str());
  WiFi.begin(ssid, password);
  double timeout = WIFI_TIMEOUT;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    digitalWrite(IO_LED, LOW);
    delay(250);
    digitalWrite(IO_LED, HIGH);
    pz.playTone(3500, 250);
    timeout -=.5;
  }
  if (timeout > 0) {
    Serial.println(format("30B; connected; %s", WiFi.localIP().toString()).c_str());
  } else {
    Serial.println("30C; no connection");
    digitalWrite(IO_LED, LOW);
    delay(WIFI_TIMEOUT/0.002);
    exit(1);
  }


  //RFID Reader
  reader.begin();
  delay(50);
  if (!reader.setupRF()) {
    Serial.println("10C; RFID Failed");
    exit(1);
  }
  last_scan_time = (time_t) -5;
  for (int i = 0; i < 3; i++) {
    Serial.println("10B; RFID Ready");
  }
  digitalWrite(IO_LED, LOW);
}

int toneloops = 0;

void loop() {
  if (toneloops > 0 && --toneloops <= 0) {
    pz.stopTone();
  }

  iClassErrorCode rc = reader.ActivateAll();

  if(rc == ICLASS_EC_OK) {
    digitalWrite(ONBOARD_LED, HIGH);
    digitalWrite(IO_LED, HIGH);
    uint64_t psID;
    if (genCardPseudoID(&psID)) {
      pz.setTone(4200);
      Serial.println(format("20A; iClass card found; %016llx", psID).c_str());
      bool match = (psID == last_detected);
      last_detected = psID;
      time_t now;
      time(&now);
      if (match && difftime(now, last_scan_time) > 5.0 && (last_scan != psID || difftime(now, last_scan_time) > 15.0)) {
        last_scan = psID;
        time(&last_scan_time);
        on_scan(psID); 
      }
      toneloops = 8;
    } else {
      if (toneloops <= 0) {
        pz.setTone(1000);
        toneloops = 5;
      }
      Serial.println("20C; iClass card found; Failed to generate psuedo identifier hash");
    }

    
    
  } else {
    digitalWrite(ONBOARD_LED, LOW);
    digitalWrite(IO_LED, LOW);
    //Serial.println("No card detected.");
  }

  delay(50);
}


