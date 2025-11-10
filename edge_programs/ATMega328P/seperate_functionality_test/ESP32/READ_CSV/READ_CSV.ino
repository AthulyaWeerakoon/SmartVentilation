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
    Serial.println("\n--- log.csv contents ---");
    File file = SD.open(LOG_FILE, FILE_READ);
    if (file) {
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
      Serial.println("\n--- End of file ---");
    } else {
      Serial.println("Error opening log.csv for reading!");
    }
  } else {
    Serial.println("log.csv does not exist!");
  }
}

void loop() {
  // Nothing to do
}
