#include "MIDIUSB.h"
#include "Keyboard.h"

// alternate midi setup
// https://learn.adafruit.com/adafruit-feather-32u4-bluefruit-le?view=all
// https://github.com/rkistner/arcore

//------------------------------------------------------
// MIDI UTILS
// https://www.arduino.cc/en/Tutorial/MidiDevice
// https://learn.adafruit.com/web-ble-midi/simple-midi-controller

void controlChange(byte channel, byte control, byte value) {
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
}
void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = { 0x09, 0x90 | channel, pitch, velocity };
    MidiUSB.sendMIDI(noteOn);
}
void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = { 0x08, 0x80 | channel, pitch, velocity };
    MidiUSB.sendMIDI(noteOn);
}
void allNotesOff(byte channel) {
    for (byte pitch = 0; pitch <= 127; pitch++) {
        noteOff(channel, pitch, 100);
    }
}

//------------------------------------------------------
// KEYBOARD MEDIA KEYS

void volumeMute() {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_F10);
    delay(75);
    Keyboard.release(KEY_F10);
    delay(75);
    Keyboard.release(KEY_LEFT_CTRL);
}
void volumeDown() {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_F11);
    delay(75);
    Keyboard.release(KEY_F11);
    delay(75);
    Keyboard.release(KEY_LEFT_CTRL);
}
void volumeUp() {
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(KEY_F12);
    delay(75);
    Keyboard.release(KEY_F12);
    delay(75);
    Keyboard.release(KEY_LEFT_CTRL);
}

//------------------------------------------------------
// KEY AND PIN SETUP

#define NUM_KEYS 15

//  my number - arduino number - meaning
//  (my number is midi offset from b-below-middle-c)
#define KEY00_PIN 18  // B
#define KEY01_PIN 19  // C
#define KEY02_PIN 20  // c#
#define KEY03_PIN 21  // D
#define KEY04_PIN 22  // d#
#define KEY05_PIN 23  // E

#define KEY06_PIN 15  // F
#define KEY07_PIN 16  // f#
#define KEY08_PIN 14  // G
#define KEY09_PIN  0  // g#
#define KEY10_PIN  1  // A
#define KEY11_PIN 12  // a#

#define KEY12_PIN 11  // ?
#define KEY13_PIN  5  // shift
#define KEY14_PIN  3  // ??

#define JOY_X_ANALOG_PIN A10  // joystick x axis (arduino pin 10)
#define JOY_Y_ANALOG_PIN  A9  // joystick y axis (arduino pin 9)

#define LED_PIN 13  // onboard red led

// unused pins
// 6 -- can be used for neopixels
// 2

#define SHIFT_KEY 13   // as a key-number, not a pin number

int keyToPin[NUM_KEYS] = {
    KEY00_PIN,  // B
    KEY01_PIN,  // C
    KEY02_PIN,
    KEY03_PIN,  // D
    KEY04_PIN,
    KEY05_PIN,  // E

    KEY06_PIN,  // F
    KEY07_PIN,
    KEY08_PIN,  // G
    KEY09_PIN,
    KEY10_PIN,  // A
    KEY11_PIN,  // a#

    KEY12_PIN,  // ?
    KEY13_PIN,  // shift
    KEY14_PIN,  // ??
};

//------------------------------------------------------
// STATE

#define KEY_COOLDOWN 1000  // for debouncing.  max 32000
#define READ_JOY_EVERY 200  // to save time on analog reads.

// keys: arrays for debouncing & state tracking
bool keyIsDown[NUM_KEYS];
bool keyWasDown[NUM_KEYS];
int coolDown[NUM_KEYS];

// midi
int baseTranspose = 60;  // what midi note the C key should play
int deltaTranspose = 0;  // shift by this much when transposing
int octaveTranspose = 0; // number of octaves to shift up or down.
byte midiChannel = 0;

// joystick
float joy_x = 0.0;
float joy_y = 0.0;
int joyCooldown = 0;

//------------------------------------------------------
// SETUP

void setup() {
    pinMode(LED_PIN, OUTPUT);
    for (int ii = 0; ii < NUM_KEYS; ii++) {
        pinMode(keyToPin[ii], INPUT);
        keyIsDown[ii] = false;
        keyWasDown[ii] = false;
        coolDown[ii] = 0;
    }
    // don't need to set up the analog input pins

    Keyboard.begin();
}

//------------------------------------------------------
// MAIN

void loop() {
    MidiUSB.flush();

    // read joystick every so often
    joyCooldown -= 1;
    if (joyCooldown <= 0) {
        // analogRead returns an int 0 - 1023, meaning 0 to 3.3 volts
        // remap to a float -1 to 1
        joy_x = ((float)analogRead(JOY_X_ANALOG_PIN) / 1023.0 - 0.5) * 2.0;
        joy_y = ((float)analogRead(JOY_Y_ANALOG_PIN) / 1023.0 - 0.5) * 2.0;
        joyCooldown = READ_JOY_EVERY;
    }

    // read keys
    for (int ii = 0; ii < NUM_KEYS; ii++) {
        if (coolDown[ii] > 0) {
            // completely ignore a key until it's cooled down.
            // apply cooldown here.
            coolDown[ii] -= 1;
        } else {
            // record new state.
            keyWasDown[ii] = keyIsDown[ii];
            keyIsDown[ii] = digitalRead(keyToPin[ii]);
        }
    }

    // react to key events
    for (int ii = 0; ii < NUM_KEYS; ii++) {
        // ignore keys that are not cooled down
        if (coolDown[ii] != 0) {
            continue;
        }
        // when a key is pressed, send event and restart the cooldown timer
        if (keyWasDown[ii] && ! keyIsDown[ii]) {
            onKeyUp(ii);
            coolDown[ii] = KEY_COOLDOWN;
        } else if (! keyWasDown[ii] && keyIsDown[ii]) {
            onKeyDown(ii);
            coolDown[ii] = KEY_COOLDOWN;
        } else if (keyIsDown[ii]) {
            onKeyStillDown(ii);
        }
    }

    // blink LED
    //digitalWrite(13, HIGH);
    //MidiUSB.flush();
    //noteOn(0, 60, 100);
    //delay(200);
    //digitalWrite(13, LOW);
    //noteOff(0, 60, 100);
    //delay(2000);
}

int keyToMidiNote(int key) {
    // key 0 is B-below-middleC, midi 59
    return key - 1 + baseTranspose + deltaTranspose + octaveTranspose * 12;
}

void onKeyDown(int key) {
    if (key >= 0 && key <= 11) {
        // midi keys
        noteOn(midiChannel, keyToMidiNote(key), 100);
    } else if (key == 12) {
        volumeDown();
    } else if (key == SHIFT_KEY) {
        // shift
    } else if (key == 14) {
        volumeUp();
    }
}
void onKeyUp(int key) {
    if (key >= 0 && key <= 11) {
        // midi keys
        noteOff(midiChannel, keyToMidiNote(key), 100);
    }
}
void onKeyStillDown(int key) {
}
