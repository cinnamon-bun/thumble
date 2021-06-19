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
// MAIN

//      my number - arduino number - meaning
#define KEY01 18  // B
#define KEY02 19  // C
#define KEY03 20  // c#
#define KEY04 21  // D
#define KEY05 22
#define KEY06 23  // E
#define KEY07 15  // F
#define KEY08 16
#define KEY09 14  // G
#define KEY10  0
#define KEY11  1  // A
#define KEY12 12  // a#

#define KEY13 11  // extra key
#define KEY14  5  // extra key
#define KEY15  3  // extra key

#define JOY_X A10  // joystick x axis (arduino pin 10)
#define JOY_Y  A9  // joystick y axis (arduino pin 9)

#define LED   13  // onboard red led

void setup() {
    pinMode(KEY01, INPUT);
    pinMode(KEY02, INPUT);
    pinMode(KEY03, INPUT);
    pinMode(KEY04, INPUT);
    pinMode(KEY05, INPUT);
    pinMode(KEY06, INPUT);
    pinMode(KEY07, INPUT);
    pinMode(KEY08, INPUT);
    pinMode(KEY09, INPUT);
    pinMode(KEY10, INPUT);
    pinMode(KEY11, INPUT);
    pinMode(KEY12, INPUT);

    pinMode(KEY13, INPUT);
    pinMode(KEY14, INPUT);
    pinMode(KEY15, INPUT);

    pinMode(LED, OUTPUT);

    // don't need to set up the analog input pins
}

void loop() {

    //analogRead(JOY_X);  // returns 0 - 1023, meaning 0 to 3.3 volts
    //analogRead(JOY_Y);

    digitalWrite(13, HIGH);
    MidiUSB.flush();
    noteOn(0, 60, 100);
    delay(200);
    digitalWrite(13, LOW);
    noteOff(0, 60, 100);
    delay(2000);
}
