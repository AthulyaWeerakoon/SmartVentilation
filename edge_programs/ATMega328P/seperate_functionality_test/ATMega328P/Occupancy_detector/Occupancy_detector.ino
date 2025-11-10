#define IR1 5
#define IR2 4

const unsigned long DEBOUNCE_DELAY = 10;

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

int occupancy = 0;
char firstBroken = 0;   // 'A' or 'B'
char secondBroken = 0;  // 'A' or 'B'
char firstCleared = 0;  // 'A' or 'B'
char secondCleared = 0; // 'A' or 'B'
bool cycleActive = false;

void setup() {
  Serial.begin(9600);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  Serial.println("IR Occupancy Counter Started");
}

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
          occupancy++;
          Serial.println("Entry detected!");
        } else if (firstBroken == 'B' && secondBroken == 'A') {
          if (occupancy > 0) occupancy--;
          Serial.println("Exit detected!");
        }
      } else {
        Serial.println("Order mismatch — cycle ignored.");
      }

      Serial.print("Occupancy: ");
      Serial.println(occupancy);
    }

    // Reset everything for next cycle
    firstBroken = 0;
    secondBroken = 0;
    firstCleared = 0;
    secondCleared = 0;
    cycleActive = false;
  }
}

void loop() {
  updateSensor(s1);
  updateSensor(s2);
  checkOccupancy();

  // Optional debug
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("IR1: ");
    Serial.print(digitalRead(IR1));
    Serial.print("\tIR2: ");
    Serial.print(digitalRead(IR2));
    Serial.print("\tOccupancy: ");
    Serial.println(occupancy);
    lastPrint = millis();
  }
}
