#include <Adafruit_NeoPixel.h>
#include <NewPing.h>


#define LED_PIN        A0 
#define NUM_LEDS       8

#define TRIGGER_PIN    9
#define ECHO_PIN       10

#define MAX_DIST_CM    400
#define CRITICAL_CM    30
#define SAMPLES        3

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST_CM);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned int readDistance() {
  unsigned int vals[SAMPLES];

  for (int i = 0; i < SAMPLES; i++) {
    unsigned int d = sonar.ping_cm();
    if (d == 0) d = MAX_DIST_CM + 1;
    vals[i] = d;
    delay(10);
  }

  for (int i = 0; i < SAMPLES - 1; i++)
    for (int j = i + 1; j < SAMPLES; j++)
      if (vals[j] < vals[i]) {
        unsigned int t = vals[i];
        vals[i] = vals[j];
        vals[j] = t;
      }

  unsigned int sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += vals[i];
  unsigned int avg = sum / SAMPLES;

  if (avg > MAX_DIST_CM) return 0;
  return avg;
}

void setBar(int count, unsigned int dist) {
  strip.clear();

  for (int i = 0; i < count; i++) {
    uint32_t c;

    if (dist == 0 || dist > 200)       c = strip.Color(0,120,0);     // green
    else if (dist > 100)               c = strip.Color(80,160,0);    // green/yellow
    else if (dist > CRITICAL_CM)       c = strip.Color(180,90,0);    // orange
    else                                c = strip.Color(150,0,0);    // red

    strip.setPixelColor(i, c);
  }
  strip.show();
}

void flashCritical() {
  static unsigned long t = 0;
  static bool on = false;
  unsigned long now = millis();

  if (now - t >= 200) {
    t = now;
    on = !on;
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (on) strip.setPixelColor(i, strip.Color(180,0,0));
    else    strip.setPixelColor(i, 0);
  }
  strip.show();
}

void setup() {
  strip.begin();
  strip.show();
}

void loop() {
  unsigned int dist = readDistance();

  int ledsOn;
  if (dist == 0) {
    ledsOn = 1;
  } else {
    int d = constrain((int)dist, 5, 250);
    ledsOn = map(d, 250, 5, 1, NUM_LEDS);
    ledsOn = constrain(ledsOn, 1, NUM_LEDS);
  }

  if (dist > 0 && dist <= CRITICAL_CM) {
    flashCritical();
  } else {
    setBar(ledsOn, dist);
  }

  delay(80);
}
