#include <Adafruit_NeoPixel.h>
#include <NewPing.h>

#define LED_PIN        A0
#define NUM_LEDS       8
#define TRIGGER_PIN    9
#define ECHO_PIN       10

#define MAX_DIST_CM    400
#define SAMPLES        3

int CAR_NOSE_OFFSET_CM = 120;  // The distance from the point that the sensor is to the front bumber of your car

int TARGET_STOP_DISTANCE_CM = 50;  // The desired distance from the front of your car to the next car

#define CRITICAL_FLASH_SPEED_MS 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST_CM);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned int readDistanceRaw() {
  unsigned int vals[SAMPLES];
  for (int i = 0; i < SAMPLES; i++) {
    unsigned int d = sonar.ping_cm();
    if (d == 0) d = MAX_DIST_CM + 1;
    vals[i] = d;
    delay(10);
  }

  for (int i = 0; i < SAMPLES - 1; i++)
    for (int j = i + 1; j < SAMPLES; j++)
      if (vals[j] < vals[i]) { unsigned int t = vals[i]; vals[i] = vals[j]; vals[j] = t; }

  unsigned int sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += vals[i];
  unsigned int avg = sum / SAMPLES;

  if (avg > MAX_DIST_CM) return 0;
  return avg;
}

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

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  
  unsigned int distRaw = readDistanceRaw();  // cm

  int bumperDistance;
  if (distRaw == 0) bumperDistance = -999; 
  else bumperDistance = (int)distRaw - CAR_NOSE_OFFSET_CM;


  if (bumperDistance <= 0) {

    flashCritical();
    delay(80);
    return;
  }

  int FAR_LIMIT = TARGET_STOP_DISTANCE_CM + 150; 
  int NEAR_LIMIT = TARGET_STOP_DISTANCE_CM;       

  bumperDistance = constrain(bumperDistance, NEAR_LIMIT, FAR_LIMIT);


  int ledsOn = map(bumperDistance, FAR_LIMIT, NEAR_LIMIT, 1, NUM_LEDS);
  ledsOn = constrain(ledsOn, 1, NUM_LEDS);


  uint32_t color;
  if (bumperDistance > TARGET_STOP_DISTANCE_CM + 80)       color = strip.Color(0, 120, 0);     // green
  else if (bumperDistance > TARGET_STOP_DISTANCE_CM + 30)  color = strip.Color(180, 150, 0);   // yellow
  else if (bumperDistance > TARGET_STOP_DISTANCE_CM)       color = strip.Color(180, 70, 0);    // orange
  else                                                     color = strip.Color(180, 0, 0);     // red


  if (bumperDistance <= TARGET_STOP_DISTANCE_CM) {
    flashCritical();
  } else {
    drawBar(ledsOn, color);
  }

  delay(80);
}
