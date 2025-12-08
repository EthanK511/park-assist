Park-Assist
Overview

Park-Assist is a simple DIY parking aid built around an ultrasonic sensor, an Arduino Nano, and an LED strip. As you approach an obstacle, the LEDs progressively light up — turning more numerous and redder — to give you intuitive visual feedback on how close you are to the object ahead.

Features

Ultrasonic distance sensing to detect how close the front of your car is to an obstacle.

Gradual LED brightness/color change: more lights, shifting toward red as distance decreases.

Configurable distance threshold (so you can calibrate how close you want to get before warning).

Compact housing design — sensor mounted such that it sees over the hood/windshield rim for reliable measurement.

What’s Inside

Hardware: HC‑SR04 Ultrasonic Sensor for range detection, WS2812 LED Strip for visual output, Arduino Nano microcontroller, plus wiring and a custom 3D-printed houseing.

Software: Arduino C++ code that reads distance, maps it to LED count & color, and outputs to the LED strip.

Setup & Usage

When approaching a wall/car while parking, watch LED strip: more lights + red color → closer distance → stop when you see full red.
