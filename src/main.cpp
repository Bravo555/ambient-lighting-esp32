#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int PIN = 2;
const int NUMPIXELS = 60;
const int DELAYVAL = 200;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(9600);
    pixels.begin();
}

void loop() {
    for(int i=0; i<NUMPIXELS * 4; i++) {
        while(Serial.available()) {
            uint8_t data = Serial.read();
            if(data == 0x00) {
                // set brightness
                int brightness = Serial.read();
                if(brightness == -1) break;
                if(Serial.availableForWrite()) Serial.printf("setting brightness at: %d\n", brightness);
                pixels.setBrightness(brightness);
            }
        }

        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i % 60, pixels.Color((i * 10)%250, 150, (i-10)%250));

        pixels.show();   // Send the updated pixel colors to the hardware.

        delay(DELAYVAL); // Pause before next pass through loop
                         // need to get rid of this
    }
}
