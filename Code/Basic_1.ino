#include <Adafruit_NeoPixel.h>
#include <NewPing.h>

// ---------------- CONFIG ----------------

// Pin assignments
#define LED_PIN        A0
#define NUM_LEDS       8
#define TRIGGER_PIN    9
#define ECHO_PIN       10

// Sensor configuration
#define MAX_DIST_CM    400
#define SAMPLES        3

// ---- New configuration variables ----
// Distance from sensor (dash) to the front-most point of the car.
int CAR_NOSE_OFFSET_CM = 70;      // adjust to your vehicle, typical: 60–90 cm

// Desired safe distance from car front to the wall / another car
int TARGET_STOP_DISTANCE_CM = 50; // adjust to preference

// LED behavior
#define CRITICAL_FLASH_SPEED_MS 200

// ---------------- GLOBALS ----------------
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST_CM);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------------- DISTANCE READ ----------------
unsigned int readDistanceRaw() {
  unsigned int vals[SAMPLES];
  for (int i = 0; i < SAMPLES; i++) {
    unsigned int d = sonar.ping_cm();
    if (d == 0) d = MAX_DIST_CM + 1;
    vals[i] = d;
    delay(10);
  }

  // sort small array
  for (int i = 0; i < SAMPLES - 1; i++)
    for (int j = i + 1; j < SAMPLES; j++)
      if (vals[j] < vals[i]) { unsigned int t = vals[i]; vals[i] = vals[j]; vals[j] = t; }

  // average
  unsigned int sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += vals[i];
  unsigned int avg = sum / SAMPLES;

  if (avg > MAX_DIST_CM) return 0;
  return avg;
}

// ---------------- LED BAR ----------------
void drawBar(int count, uint32_t color) {
  strip.clear();
  for (int i = 0; i < count; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void flashCritical() {
  static unsigned long last = 0;
  static bool on = false;
  unsigned long now = millis();
  if (now - last >= CRITICAL_FLASH_SPEED_MS) {
    last = now;
    on = !on;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, on ? strip.Color(180,0,0) : 0);
  }
  strip.show();
}

// ---------------- MAIN ----------------
void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  // STEP 1 — Raw reading (sensor to obstacle)
  unsigned int distRaw = readDistanceRaw();  // cm

  // STEP 2 — Convert reading to distance FROM BUMPER
  // bumperDistance = distance sensor→wall minus offset sensor→bumper
  int bumperDistance;
  if (distRaw == 0) bumperDistance = -999;  // no reading
  else bumperDistance = (int)distRaw - CAR_NOSE_OFFSET_CM;

  // STEP 3 — LED logic
  if (bumperDistance <= 0) {
    // Object is at or inside bumper offset (too close)
    flashCritical();
    delay(80);
    return;
  }

  // Scaling logic
  // Define the span from “far enough not to care” down to “target stop distance”
  int FAR_LIMIT = TARGET_STOP_DISTANCE_CM + 150;  // 150 cm behind the target distance
  int NEAR_LIMIT = TARGET_STOP_DISTANCE_CM;       // desired stopping point

  bumperDistance = constrain(bumperDistance, NEAR_LIMIT, FAR_LIMIT);

  // Map to LEDs
  int ledsOn = map(bumperDistance, FAR_LIMIT, NEAR_LIMIT, 1, NUM_LEDS);
  ledsOn = constrain(ledsOn, 1, NUM_LEDS);

  // Color selection
  uint32_t color;
  if (bumperDistance > TARGET_STOP_DISTANCE_CM + 80)       color = strip.Color(0, 120, 0);     // green
  else if (bumperDistance > TARGET_STOP_DISTANCE_CM + 30)  color = strip.Color(180, 150, 0);   // yellow
  else if (bumperDistance > TARGET_STOP_DISTANCE_CM)       color = strip.Color(180, 70, 0);    // orange
  else                                                     color = strip.Color(180, 0, 0);     // red

  // If inside the target stop distance → critical
  if (bumperDistance <= TARGET_STOP_DISTANCE_CM) {
    flashCritical();
  } else {
    drawBar(ledsOn, color);
  }

  delay(80);
}
