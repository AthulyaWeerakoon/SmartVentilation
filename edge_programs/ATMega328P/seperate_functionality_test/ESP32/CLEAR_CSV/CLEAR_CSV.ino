#include <SPI.h>
#include <SD.h>

#define CS_PIN 10
const char LOG_FILE[] = "log.csv";

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Initializing SD card...");
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD init failed!");
    while (1);
  }
  Serial.println("SD init success.");

  if (SD.exists(LOG_FILE)) {
    Serial.println("log.csv exists, clearing contents...");
    File file = SD.open(LOG_FILE, FILE_WRITE);  // open for writing (overwrites content)
    if (file) {
      file.close();  // opening in write mode resets file pointer to 0 and clears content
      Serial.println("log.csv cleared successfully.");
    } else {
      Serial.println("Error opening log.csv for writing!");
    }
  } else {
    Serial.println("log.csv does not exist.");
  }
}

void loop() {
  // Nothing to do
}
