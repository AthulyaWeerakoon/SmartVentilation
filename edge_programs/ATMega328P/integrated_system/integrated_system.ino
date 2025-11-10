#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

#define I2C_ADDRESS 0x3C
#define SCREEN_INTERVAL 5000UL
#define MESSAGE_DURATION 2000UL
#define TIME_UPDATE_INTERVAL 1000UL
#define DEBOUNCE_DELAY 10UL
#define MQ135_A A0
#define MQ2_A   A1
#define BUTTON_PIN 2
#define CS_PIN 10
#define LED_GREEN 6
#define LED_RED   7
#define FAN_PWM   9
#define IR1 5
#define IR2 4

RTC_DS3231 rtc;

// --- OLED Switcher variables ---
SSD1306AsciiWire oled;
DateTime currentTime;

enum ScreenType { SCREEN_TIME, SCREEN_AQI, SCREEN_OCCUPANCY, SCREEN_MESSAGE };
ScreenType currentScreen = SCREEN_TIME;

unsigned long lastScreenSwitch = 0;
unsigned long lastTimeUpdate = 0;
unsigned long messageStartTime = 0;
unsigned long lastButtonTime = 0;

bool lastButtonState = HIGH;
bool buttonPressed = false;

String messageLine1, messageLine2, messageLine3;

// --- AQ Monitor variables ---
const int MQ_MAX = 400;
const int MQ_CRIT = 200;
const int MQ_BASELINE = 100;

unsigned long lastUpdate = 0;
unsigned long updateInterval = 60000UL;
bool first_run = true;

const float WEIGHT_AQ = 0.8;
const float WEIGHT_OCC = 0.2;

// -- SD Logger variables --
unsigned long previousMillis = 0;
const unsigned long interval = 60000;

const char LOG_FILE[] = "log.csv";

struct SensorData {
  float mq2;
  float mq135;
  float airQualityIndex;
  int occupancy;
  int fanPWM;
};

SensorData readings = {0, 0, 0, 0, 0};

// --- Occupancy Detector variables ---
struct IRSensor {
  int pin;
  int lastRaw;
  int stableState;
  unsigned long lastChange;
  bool broken;
  bool justBroken;
  bool justCleared;
};

IRSensor s1 = {IR1, HIGH, HIGH, 0, false, false, false};
IRSensor s2 = {IR2, HIGH, HIGH, 0, false, false, false};

char firstBroken = 0;   // 'A' or 'B'
char secondBroken = 0;  // 'A' or 'B'
char firstCleared = 0;  // 'A' or 'B'
char secondCleared = 0; // 'A' or 'B'
bool cycleActive = false;

void setup() {
  // OLED & I2C
  Wire.begin();
  Wire.setClock(400000L);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();

  // RTC
  if (!rtc.begin()) {
    oled.println("Couldn't find RTC");
    while (1); // or use a non-blocking fallback
  }
  currentTime = rtc.now();
  drawCurrentScreen();

  // OCCUPANCY sensors
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  readings.occupancy = 0;

  // AQ outputs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(FAN_PWM, OUTPUT);

  // SD CS pin must be OUTPUT on many boards before SD.begin()
  pinMode(CS_PIN, OUTPUT);
  if (!SD.begin(CS_PIN)) {
    oled.println("SD init failed");
    while (1); // or degrade gracefully
  }

  bool newFile = false;

  // Create file if missing
  if (!SD.exists(LOG_FILE)) {
    File file = SD.open(LOG_FILE, FILE_WRITE);
    if (file) {
      file.println("Timestamp,MQ-2,MQ-135,Air Quality Index,Occupancy,Fan PWM");
      file.close();
      newFile = true;
      oled.println("Initializing log.csv...");
      delay(1000);
    } else {
      oled.println("Error creating log.csv");
      while (1);
    }
  } else {
    oled.println("log.csv found...");
    delay(1000);
  }

  oled.println("System online...");
  analogWrite(FAN_PWM, 80);
}

void loop() {
  // Occupancy
  updateSensor(s1);
  updateSensor(s2);
  checkOccupancy();

  // Air Quality
  updateAirQuality();

  // OLED switcher
  unsigned long nowMillis = millis();
  handleButton(nowMillis);
  handleTimeUpdate(nowMillis);
  handleScreenSwitch(nowMillis);

  // SD Logger
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    appendLogEntry(readings);
  }
}

// -- AQ utils --
void updateAirQuality() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval || first_run) {
    lastUpdate = now;

    readings.mq135 = analogRead(MQ135_A);
    readings.mq2  = analogRead(MQ2_A);

    // Normalize both sensor values between 0–1
    float norm135 = constrain((float)(readings.mq135 - MQ_BASELINE) / (MQ_MAX - MQ_BASELINE), 0.0, 1.0);
    float norm2   = constrain((float)(readings.mq2   - MQ_BASELINE) / (MQ_MAX - MQ_BASELINE), 0.0, 1.0);

    // Simple combined air quality index (weighted average)
    readings.airQualityIndex = (norm135 * 0.6 + norm2 * 0.4) * 100.0; // 0–100 scale

    // Determine fan speed (linearly mapped)
    readings.fanPWM = map((int)readings.airQualityIndex, 0, 100, 80, 255); // baseline fan speed: 80

    // LED indication
    if (readings.airQualityIndex < 60.0) { // good air
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
    } else { // poor air
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }

    // Dynamic update interval adjustment
    // Shorter interval = more frequent checks when air quality is bad
    // Combine AQI and occupancy into a scaling factor
    float aqFactor = constrain(readings.airQualityIndex / 100.0f, 0.0f, 1.0f);
    float occFactor = constrain((float)readings.occupancy / 10.0f, 0.0f, 1.0f);
    float urgency = (WEIGHT_AQ * aqFactor + WEIGHT_OCC * occFactor);

    int urgencyPct = (int)constrain(urgency * 100.0f, 0.0f, 100.0f);
    updateInterval = (unsigned long)map(urgencyPct, 0, 100, 60000, 5000);

    // Apply fan PWM
    analogWrite(FAN_PWM, readings.fanPWM);

    // Unset first run
    first_run = false;
  }
}

// -- OLED utils --
void handleButton(unsigned long nowMillis) {
  bool buttonState = digitalRead(BUTTON_PIN);

  // Debounce
  if (buttonState != lastButtonState) {
    lastButtonTime = nowMillis;
    lastButtonState = buttonState;
  }

  if (buttonState == LOW && !buttonPressed && (nowMillis - lastButtonTime > DEBOUNCE_DELAY)) {
    buttonPressed = true;
    nextScreen();
    lastScreenSwitch = nowMillis; // reset the timer
  }

  if (buttonState == HIGH && buttonPressed && (nowMillis - lastButtonTime > DEBOUNCE_DELAY)) {
    buttonPressed = false; // ready for next press
  }
}

void handleTimeUpdate(unsigned long nowMillis) {
  if (nowMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdate = nowMillis;
    currentTime = rtc.now(); // always update time

    if (currentScreen == SCREEN_TIME) {
      drawCurrentScreen(); // refresh once per second on time screen
    }
  }
}

void handleScreenSwitch(unsigned long nowMillis) {
  if (currentScreen == SCREEN_MESSAGE) {
    if (nowMillis - messageStartTime >= MESSAGE_DURATION) {
      currentScreen = SCREEN_TIME;
      drawCurrentScreen();
    }
    return;
  }

  if (nowMillis - lastScreenSwitch >= SCREEN_INTERVAL) {
    nextScreen();
    lastScreenSwitch = nowMillis;
  }
}

void nextScreen() {
  switch (currentScreen) {
    case SCREEN_TIME: currentScreen = SCREEN_AQI; break;
    case SCREEN_AQI: currentScreen = SCREEN_OCCUPANCY; break;
    case SCREEN_OCCUPANCY: currentScreen = SCREEN_TIME; break;
    default: currentScreen = SCREEN_TIME; break;
  }
  drawCurrentScreen();
}

void showOccupancyScreen() {
  currentScreen = SCREEN_OCCUPANCY;
  drawCurrentScreen();
  lastScreenSwitch = millis();
}

void showTemporaryMessage(const String& line1, const String& line2 = "", const String& line3 = "") {
  messageLine1 = line1;
  messageLine2 = line2;
  messageLine3 = line3;
  currentScreen = SCREEN_MESSAGE;
  messageStartTime = millis();
  drawMessageScreen();
}

void drawCurrentScreen() {
  oled.clear();

  switch (currentScreen) {
    case SCREEN_TIME:        drawTimeScreen(); break;
    case SCREEN_AQI:         drawAQIScreen(); break;
    case SCREEN_OCCUPANCY:   drawOccupancyScreen(); break;
    case SCREEN_MESSAGE:     drawMessageScreen(); break;
  }
}

void drawTimeScreen() {
  char buffer[32];
  sprintf(buffer, "%02d/%02d/%04d", currentTime.day(), currentTime.month(), currentTime.year());
  oled.setCursor(0, 0);
  oled.println(buffer);

  sprintf(buffer, "%02d:%02d:%02d", currentTime.hour(), currentTime.minute(), currentTime.second());
  oled.setCursor(0, 16);
  oled.println(buffer);
}

void drawAQIScreen() {
  oled.setCursor(0, 8);
  oled.print("AQI: ");
  oled.println(readings.airQualityIndex);

  oled.setCursor(0, 16);
  oled.print("Fan: ");
  oled.print((float)readings.fanPWM * 100.0 / 255.0);
  oled.println("%");
}

void drawOccupancyScreen() {
  oled.setCursor(0, 8);
  oled.println("Occupancy");
  oled.setCursor(0, 16);
  oled.print("Count: ");
  oled.println(readings.occupancy);
}

void drawMessageScreen() {
  oled.clear();

  int totalLines = 0;
  if (messageLine1.length()) totalLines++;
  if (messageLine2.length()) totalLines++;
  if (messageLine3.length()) totalLines++;

  int startY = (32 - (totalLines * 8)) / 2;
  int y = startY;

  if (messageLine1.length()) { oled.setCursor(0, y); oled.println(messageLine1); y += 8; }
  if (messageLine2.length()) { oled.setCursor(0, y); oled.println(messageLine2); y += 8; }
  if (messageLine3.length()) { oled.setCursor(0, y); oled.println(messageLine3); }
}

// -- SD utils --
void appendLogEntry(const SensorData &data) {
  File file = SD.open(LOG_FILE, FILE_WRITE);
  if (!file) {
    showTemporaryMessage("", "Error opening log.csv...", "");
    return;
  }

  file.print(currentTime.year()); file.print('/');
  file.print(currentTime.month()); file.print('/');
  file.print(currentTime.day()); file.print(' ');
  file.print(currentTime.hour()); file.print(':');
  file.print(currentTime.minute()); file.print(':');
  file.print(currentTime.second());

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
}

// -- Occupancy utils --
void updateSensor(IRSensor &s) {
  int reading = digitalRead(s.pin);

  if (reading != s.lastRaw) {
    s.lastChange = millis();
    s.lastRaw = reading;
  }

  if ((millis() - s.lastChange) > DEBOUNCE_DELAY) {
    s.justBroken = false;
    s.justCleared = false;

    if (s.stableState == HIGH && reading == LOW) {
      s.stableState = LOW;
      s.broken = true;
      s.justBroken = true;
    } else if (s.stableState == LOW && reading == HIGH) {
      s.stableState = HIGH;
      s.broken = false;
      s.justCleared = true;
    }
  }
}

void checkOccupancy() {
  // Detect break order
  if (!cycleActive && (s1.justBroken || s2.justBroken)) {
    cycleActive = true;
    if (s1.justBroken) firstBroken = 'A';
    if (s2.justBroken) firstBroken = 'B';
  }

  if (cycleActive && (s1.justBroken || s2.justBroken)) {
    if (firstBroken == 'A' && s2.justBroken && secondBroken == 0) secondBroken = 'B';
    else if (firstBroken == 'B' && s1.justBroken && secondBroken == 0) secondBroken = 'A';
  }

  // Detect restore order
  if (cycleActive && (s1.justCleared || s2.justCleared)) {
    if (firstCleared == 0) {
      if (s1.justCleared) firstCleared = 'A';
      else if (s2.justCleared) firstCleared = 'B';
    } else {
      if (s1.justCleared && firstCleared != 'A') secondCleared = 'A';
      else if (s2.justCleared && firstCleared != 'B') secondCleared = 'B';
    }
  }

  // When both beams clear again → evaluate the full cycle
  if (cycleActive && s1.stableState == HIGH && s2.stableState == HIGH) {
    if (firstBroken != 0 && secondBroken != 0 && firstCleared != 0 && secondCleared != 0) {
      // Must be same order: firstBroken==firstCleared && secondBroken==secondCleared
      if (firstBroken == firstCleared && secondBroken == secondCleared) {
        if (firstBroken == 'A' && secondBroken == 'B') {
          readings.occupancy++;
        } else if (firstBroken == 'B' && secondBroken == 'A') {
          if (readings.occupancy > 0) readings.occupancy--;
        }
      } else {
      }

      showOccupancyScreen();
    }

    // Reset everything for next cycle
    firstBroken = 0;
    secondBroken = 0;
    firstCleared = 0;
    secondCleared = 0;
    cycleActive = false;
  }
}
