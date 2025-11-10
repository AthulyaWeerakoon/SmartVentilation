#define MQ135_A A0
#define MQ2_A   A1
#define LED_GREEN 6
#define LED_RED   7
#define FAN_PWM   9  // example PWM pin (can be changed)

const int MQ_MAX = 400;   // observed upper bound (approx)
const int MQ_CRIT = 200;  // critical threshold (adjustable)
const int MQ_BASELINE = 100; // approximate "good" baseline

// --- Dynamic timing ---
unsigned long lastUpdate = 0;
unsigned long updateInterval = 2000UL; // start with 1 minute

// --- Runtime variables ---
float airQualityIndex = 0.0;
int fanPWM = 0;
int occupancy = 3;  // placeholder (will become variable later)

// --- Configurable weights ---
const float WEIGHT_AQ = 0.8;  // air quality importance
const float WEIGHT_OCC = 0.2; // occupancy importance

void setup() {
  Serial.begin(9600);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(FAN_PWM, OUTPUT);
}

void loop() {
  updateAirQuality(); // non-blocking periodic update
  // other code here (e.g., occupancy updates, UI, etc.)
}

// ----------------------------------------------------
// Non-blocking update method
// ----------------------------------------------------
void updateAirQuality() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    int mq135Val = analogRead(MQ135_A);
    int mq2Val   = analogRead(MQ2_A);

    // Normalize both sensor values between 0–1
    float norm135 = constrain((float)(mq135Val - MQ_BASELINE) / (MQ_MAX - MQ_BASELINE), 0.0, 1.0);
    float norm2   = constrain((float)(mq2Val   - MQ_BASELINE) / (MQ_MAX - MQ_BASELINE), 0.0, 1.0);

    // Simple combined air quality index (weighted average)
    airQualityIndex = (norm135 * 0.6 + norm2 * 0.4) * 100.0; // 0–100 scale

    // Determine fan speed (linearly mapped)
    fanPWM = map((int)airQualityIndex, 0, 100, 80, 255); // baseline fan speed: 80

    // LED indication
    if (airQualityIndex < 70.0) { // good air
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
    } else { // poor air
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
    }

    // Dynamic update interval adjustment
    // Shorter interval = more frequent checks when air quality is bad
    // Combine AQI and occupancy into a scaling factor
    float aqFactor = constrain(airQualityIndex / 100.0, 0.0, 1.0);
    float occFactor = constrain((float)occupancy / 10.0, 0.0, 1.0); // assuming max occupancy ~10
    float urgency = (WEIGHT_AQ * aqFactor + WEIGHT_OCC * occFactor);

    // Map urgency → update frequency (between 10s and 60s)
    updateInterval = map(urgency * 100, 0, 100, 2000, 1000);

    // Apply fan PWM
    analogWrite(FAN_PWM, fanPWM);

    // Debug print
    Serial.print("MQ135: "); Serial.print(mq135Val);
    Serial.print("  MQ2: "); Serial.print(mq2Val);
    Serial.print("  AQI: "); Serial.print(airQualityIndex, 1);
    Serial.print("  FanPWM: "); Serial.print(fanPWM);
    Serial.print("  UpdateInterval(ms): "); Serial.println(updateInterval);
  }
}
