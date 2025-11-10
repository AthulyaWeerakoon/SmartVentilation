#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "RTClib.h"

#define I2C_ADDRESS 0x3C
#define BUTTON_PIN 2
#define SCREEN_INTERVAL 5000UL      // 5 seconds
#define MESSAGE_DURATION 2000UL     // 2 seconds
#define TIME_UPDATE_INTERVAL 1000UL // 1 second
#define DEBOUNCE_DELAY 50UL        // 150 ms debounce

SSD1306AsciiWire oled;
RTC_DS3231 rtc;

struct DisplayData {
  int aqi = 42;
  int fanSpeed = 67;
  int occupancyCount = 3;
  DateTime currentTime;
} data;

enum ScreenType { SCREEN_TIME, SCREEN_AQI, SCREEN_OCCUPANCY, SCREEN_MESSAGE };
ScreenType currentScreen = SCREEN_TIME;

unsigned long lastScreenSwitch = 0;
unsigned long lastTimeUpdate = 0;
unsigned long messageStartTime = 0;
unsigned long lastButtonTime = 0;

bool lastButtonState = HIGH;
bool buttonPressed = false;

String messageLine1, messageLine2, messageLine3;

void setup() {
  Wire.begin();
  Wire.setClock(400000L);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();

  if (!rtc.begin()) {
    oled.println("Couldn't find RTC");
    while (1);
  }

  data.currentTime = rtc.now();
  drawCurrentScreen();
}

void loop() {
  unsigned long nowMillis = millis();
  handleButton(nowMillis);
  handleTimeUpdate(nowMillis);
  handleScreenSwitch(nowMillis);
}

// ================= BUTTON HANDLER ===================
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

// ================= TIME HANDLER =====================
void handleTimeUpdate(unsigned long nowMillis) {
  if (nowMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdate = nowMillis;
    data.currentTime = rtc.now(); // always update time

    if (currentScreen == SCREEN_TIME) {
      drawCurrentScreen(); // refresh once per second on time screen
    }
  }
}

// ================= SCREEN SWITCHER ==================
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

// ================= NAVIGATION =======================
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

// ================= DRAWING ==========================
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
  sprintf(buffer, "%02d/%02d/%04d", data.currentTime.day(), data.currentTime.month(), data.currentTime.year());
  oled.setCursor(0, 0);
  oled.println(buffer);

  sprintf(buffer, "%02d:%02d:%02d", data.currentTime.hour(), data.currentTime.minute(), data.currentTime.second());
  oled.setCursor(0, 16);
  oled.println(buffer);
}

void drawAQIScreen() {
  oled.setCursor(0, 8);  // vertically centered
  oled.print("AQI: ");
  oled.println(data.aqi);

  oled.setCursor(0, 16);
  oled.print("Fan: ");
  oled.print(data.fanSpeed);
  oled.println("%");
}

void drawOccupancyScreen() {
  oled.setCursor(0, 8);
  oled.println("Occupancy");
  oled.setCursor(0, 16);
  oled.print("Count: ");
  oled.println(data.occupancyCount);
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
