#include "MIDIUSB.h"

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

//------------------------------------------------------
// VARS

#define NUM_KEYS 15

//  my number - arduino number - meaning
//  (my number is midi offset from middle c)
#define KEY11 18  // B
#define KEY00 19  // C
#define KEY01 20  // c#
#define KEY02 21  // D
#define KEY03 22  // d#
#define KEY04 23  // E
#define KEY05 15  // F
#define KEY06 16  // f#
#define KEY07 14  // G
#define KEY08  0  // g#
#define KEY09  1  // A
#define KEY10 12  // a#

#define KEY12 11  // ?
#define KEY13  5  // shift
#define KEY14  3  // ??

#define JOY_X A10  // joystick x axis (arduino pin 10)
#define JOY_Y  A9  // joystick y axis (arduino pin 9)

#define LED   13  // onboard red led

#define KEY_COOLDOWN 1000  // for debouncing.  max 32000
#define READ_JOY_EVERY 200  // to save time on analog reads.

int keyToPin[NUM_KEYS] = {
    KEY00,  // C
    KEY01,
    KEY02,  // D
    KEY03,
    KEY04,
    KEY05,
    KEY06,
    KEY07,
    KEY08,
    KEY09,
    KEY10,
    KEY11,  // B

    KEY12,  // ?
    KEY13,  // shift
    KEY14,  // ??
};

// keys: arrays for debouncing & state tracking
bool keyIsDown[NUM_KEYS];
bool keyWasDown[NUM_KEYS];
int coolDown[NUM_KEYS];

// joystick
float joy_x = 0.0;
float joy_y = 0.0;
int joyCooldown = 0;

//------------------------------------------------------
// SETUP

void setup() {
    pinMode(LED, OUTPUT);
    for (int ii = 0; ii < NUM_KEYS; ii++) {
        pinMode(keyToPin[ii], INPUT);
        keyIsDown[ii] = false;
        keyWasDown[ii] = false;
        coolDown[ii] = 0;
    }
    // don't need to set up the analog input pins
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
        joy_x = ((float)analogRead(JOY_X) / 1023.0 - 0.5) * 2.0;
        joy_y = ((float)analogRead(JOY_Y) / 1023.0 - 0.5) * 2.0;
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

void onKeyDown(int key) {
    if (key == 11) { key = -1; }
    noteOn(0, 60 + key, 100);
}
void onKeyUp(int key) {
    noteOff(0, 60 + key, 100);
}
void onKeyStillDown(int key) {
}
