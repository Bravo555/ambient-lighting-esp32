// TODO:
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - [x] Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - [ ] MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - [x] NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int PIN = 2;
const int NUMPIXELS = 60;
const int INTERVAL = 200;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

enum {
    OFF = 0,
    AMBIENT = 1,
    CAPTURING = 2,
} currentMode;

// state used by AMBIENT mode
// lastMillis needs to be set to current time when switching mode to AMBIENT
struct {
    unsigned long lastMillis;
    uint16_t hueOffset;
} ambientState;

void setup() {
    Serial.begin(115200);
    pixels.begin();
    currentMode = AMBIENT;
    ambientState.lastMillis = millis();
    ambientState.hueOffset = 0;
    pixels.show();
}


byte lastColour[3] = {0, 0, 0};

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
                    pixels.show();
                }
                    break;
                case 0x03: { // incoming colour data
                    if(currentMode == CAPTURING) {
                        byte colours[3] = {0xff, 0x00, 0x00};   // that way we can easily see "missed frames"
                        Serial.readBytes(colours, 3);
                        Serial.printf("read: %hhu %hhu %hhu\n", colours[0], colours[1], colours[2]);

                        if(colours[0] == lastColour[0] &&
                            colours[1] == lastColour[1] &&
                            colours[2] == lastColour[2]) {
                            break;
                        }

                        Serial.println("printing new colour");
                        for(int i = 0; i < NUMPIXELS; i++) {
                            pixels.setPixelColor(i, pixels.Color(colours[0], colours[1], colours[2]));
                        }
                        pixels.show();
                        lastColour[0] = colours[0];
                        lastColour[1] = colours[1];
                        lastColour[2] = colours[2];
                    }
                }
                    break;

                case 0x04: { // enable/disable capture
                    byte captureEnable = Serial.read();
                    if(captureEnable == 0xff) {
                        currentMode = CAPTURING;
                    } else if(captureEnable == 0x00) {
                        currentMode = AMBIENT;
                    }
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
                    for(unsigned int i = 0; i < NUMPIXELS; i++) {
                        uint16_t currentHue = (i * (0xffff / NUMPIXELS) + ambientState.hueOffset) % 0xffff;
                        pixels.setPixelColor(i, pixels.ColorHSV(currentHue, 0xff, 0xff));
                    }
                    pixels.show();

                    ambientState.hueOffset = (ambientState.hueOffset + 250) % 0xffff;
                    Serial.printf("%u\n", ambientState.hueOffset);
                }
            }
                break;
            case CAPTURING:
                // here we'd ordinarily process the screen payload, but we did it in message processing stage
                break;
        }
    }
    delay(10);
}
