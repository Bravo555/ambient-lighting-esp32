#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int PIN = 2;
const int NUMPIXELS = 60;
const int DELAYVAL = 200;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    pixels.begin();
}

void loop() {
    for(int i=0; i<NUMPIXELS * 4; i++) { // For each pixel...

        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i % 60, pixels.Color((i * 10)%250, 150, (i-10)%250));

        pixels.show();   // Send the updated pixel colors to the hardware.

        delay(DELAYVAL); // Pause before next pass through loop
    }
}
