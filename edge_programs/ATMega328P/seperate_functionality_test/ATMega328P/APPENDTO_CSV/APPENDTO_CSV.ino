#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

#define CS_PIN 10
const char LOG_FILE[] = "log.csv";

// RTC object
RTC_DS3231 rtc;

// Example data structure (placeholder values)
struct SensorData {
  float mq2;
  float mq135;
  float airQualityIndex;
  int occupancy;
  int fanPWM;
};

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize SD
  Serial.println("Initializing SD card...");
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD init failed!");
    while (1);
  }
  Serial.println("SD init success.");

  // Initialize RTC
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1);
  }

  bool newFile = false;

  // Create file if missing
  if (!SD.exists(LOG_FILE)) {
    Serial.println("log.csv not found, creating new file...");
    File file = SD.open(LOG_FILE, FILE_WRITE);
    if (file) {
      file.println("Timestamp,MQ-2,MQ-135,Air Quality Index,Occupancy,Fan PWM");
      file.close();
      newFile = true;
      Serial.println("Created log.csv with header.");
    } else {
      Serial.println("Error creating log.csv!");
    }
  } else {
    Serial.println("log.csv already exists.");
  }

  // Example usage — append twice
  SensorData test1 = {256.4, 312.7, 178.3, 1, 128};
  SensorData test2 = {280.9, 330.1, 190.5, 0, 64};

  appendLogEntry(test1);
  appendLogEntry(test2);

  Serial.println("Done writing test entries.");
}

void loop() {
  // Nothing here
}

void appendLogEntry(const SensorData &data) {
  File file = SD.open(LOG_FILE, FILE_WRITE);
  if (!file) {
    Serial.println("Error opening log.csv for appending!");
    return;
  }

  DateTime now = rtc.now();

  file.print(now.year()); file.print('/');
  file.print(now.month()); file.print('/');
  file.print(now.day()); file.print(' ');
  file.print(now.hour()); file.print(':');
  file.print(now.minute()); file.print(':');
  file.print(now.second());

  file.print(',');
  file.print(data.mq2, 1);
  file.print(',');
  file.print(data.mq135, 1);
  file.print(',');
  file.print(data.airQualityIndex, 1);
  file.print(',');
  file.print(data.occupancy);
  file.print(',');
  file.println(data.fanPWM);

  file.close();
  Serial.println("Appended one log entry.");
}
