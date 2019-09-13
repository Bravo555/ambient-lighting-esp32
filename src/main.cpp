// TODO:
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int PIN = 2;
const int NUMPIXELS = 60;
const int INTERVAL = 250;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

enum {
    CAPTURING,
    AMBIENT,
    OFF,
} currentMode;

struct {
    unsigned long lastMillis;
    uint16_t hue;
    uint16_t ledIndex;
} ambientState;

void setup() {
    Serial.begin(9600);
    pixels.begin();
    currentMode = AMBIENT;
    ambientState.lastMillis = millis();
    ambientState.ledIndex = 0;
    ambientState.hue = 0;
    pixels.show();
}

// state used by AMBIENT mode
// lastMillis needs to be set to current time when switching mode to AMBIENT

void loop() {
    // step 1: process messages that may/may not be irrelevant for the current mode
    // overall, we want to support the following messages:
    //      1. enable/disable
    //      2. set brightness
    //      3. process screen info payload (only when in CAPTURING mode)
    //      4. change modes (between AMBIENT and CAPTURING)
    // if we're disabled, we only care whether or not to enable
    // we're kinda emulating esp32's turning off feature, that i dont know how to implement yet
    // later, this piece of code will be replaced by proper power management
    if(currentMode == OFF) {
        while(Serial.available()) {
            byte command = Serial.read();
            if(command == 0x01) {
                byte enabled = Serial.read();
                if(enabled == 0xff) {
                    currentMode = AMBIENT;
                }
            }
        }
    }
    else {
        while(Serial.available()) {
            byte command = Serial.read();
            switch(command) {
                case 0x01: { // enable/disable
                    byte enabled = Serial.read();
                    if(enabled == 0x00) {
                        for(int i = 0; i < NUMPIXELS; i++) {
                            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
                        }
                        pixels.show();
                        currentMode = OFF;
                    }
                }
                    break;
                case 0x02: { // set brightness
                    int brightness = Serial.read();
                    if(brightness == -1) break;
                    if(Serial.availableForWrite()) Serial.printf("setting brightness at: %d\n", brightness);
                    pixels.setBrightness(brightness);
                }
                    break;
                case 0x03: { // incoming colour data
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
                    break;
            }
        }

        // after we're done with messages, we update the LEDs
        switch(currentMode) {
            case AMBIENT: {
                unsigned long currentMillis = millis();
                if(currentMillis > ambientState.lastMillis + INTERVAL) {
                    ambientState.lastMillis = currentMillis;
                    // first nice pattern would be to cycle HSV in a forwards-backwards-forwards... manner
                    pixels.setPixelColor(ambientState.ledIndex % 60, pixels.ColorHSV(ambientState.hue, 0xff, 0xff));
                    pixels.show();

                    ambientState.ledIndex += 1;
                    ambientState.hue = (ambientState.hue + 100) % 0xffff;
                    Serial.printf("%u\n", ambientState.hue);
                }
            }
                break;
            case CAPTURING:
                // here we'd ordinarily process the screen payload, but we did it in message processing stage
                break;
        }
    }

    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255

    //pixels.show();   // Send the updated pixel colors to the hardware.
}
