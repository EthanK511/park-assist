#include <Adafruit_NeoPixel.h>

#define LED_PIN A0
#define NUM_LEDS 8
#define TRIG 9
#define ECHO 8
#define BUTTON 4

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

float dashboard_offset_cm = 0;    
float desired_stop_cm = 35;       


unsigned long buttonDownTime = 0;
bool buttonWasDown = false;

float getDistanceCM() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  if (duration == 0) return 999; 

  return duration * 0.0343 / 2;
}

void setAll(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, r, g, b);
  strip.show();
}

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUTTON, INPUT);

  strip.begin();
  strip.show();

  setAll(0, 20, 0);
  delay(500);
  setAll(0, 0, 0);
}

void handleCalibration() {
  bool pressed = digitalRead(BUTTON);

  if (pressed && !buttonWasDown) {
    buttonDownTime = millis();
  }

  if (!pressed && buttonWasDown) {
    unsigned long delta = millis() - buttonDownTime;

    if (delta < 600) {
  
      dashboard_offset_cm = getDistanceCM();
      setAll(0, 50, 0); 
      delay(400);
      setAll(0, 0, 0);
    } 
  }

  if (pressed && (millis() - buttonDownTime > 1200)) {

    desired_stop_cm = getDistanceCM() - dashboard_offset_cm;
    if (desired_stop_cm < 5) desired_stop_cm = 5;

    setAll(50, 50, 0);  
    delay(700);
    setAll(0, 0, 0);
  }

  buttonWasDown = pressed;
}

void displayDistance(float dist) {

  if (dist > 150) {
    setAll(0, 0, 20);
    return;
  }

  float percent = dist / desired_stop_cm;
  if (percent < 0) percent = 0;
  if (percent > 1) percent = 1;

  int leds_to_light = round(percent * NUM_LEDS);

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < leds_to_light) {
      
      float t = float(i) / NUM_LEDS;
      uint8_t r = t * 255;
      uint8_t g = (1.0 - t) * 255;
      strip.setPixelColor(i, r, g, 0);
    } else {
      strip.setPixelColor(i, 0, 0, 0);
    }
  }


  if (dist < 15) {
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, 255, 0, 0);
  }


  if (dist <= 0) {
    for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, 255, 0, 0);
    strip.show();
    delay(100);
    setAll(0, 0, 0);
    delay(100);
    return;
  }

  strip.show();
}

void loop() {
  handleCalibration();

  float raw = getDistanceCM();
  float front_distance = raw - dashboard_offset_cm;
  float dist_to_stop = front_distance - desired_stop_cm;

  displayDistance(dist_to_stop);

  delay(60);
}
