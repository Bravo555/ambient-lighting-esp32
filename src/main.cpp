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
        while(Serial.available()) {
            uint8_t data = Serial.read();
            if(data == 0x00) {
                // set brightness
                int brightness = Serial.read();
                if(brightness == -1) break;
                if(Serial.availableForWrite()) Serial.printf("setting brightness at: %d\n", brightness);
                pixels.setBrightness(brightness);
            }

            if(data == 0x01) { // incoming colour data
                byte colours[3] = {0x12, 0x34, 0x56};
                Serial.readBytes(colours, 3);

                Serial.write(colours[0]);
                Serial.write(colours[1]);
                Serial.write(colours[2]);

                for(int i = 0; i < NUMPIXELS; i++) {
                    pixels.setPixelColor(i, pixels.Color(colours[0], colours[1], colours[2]));
                }
                pixels.show();
            }

            if((char)data == 'A') {
                Serial.println("fuck off");
            }
        }

        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255

        //pixels.show();   // Send the updated pixel colors to the hardware.
}
