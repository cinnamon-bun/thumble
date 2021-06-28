#include "MIDIUSB.h"
#include "Keyboard.h"

// alternate midi setup
// https://learn.adafruit.com/adafruit-feather-32u4-bluefruit-le?view=all
// https://github.com/rkistner/arcore


//------------------------------------------------------
// MATH UTILS

float signFloat(float x) {
    if (x > 0.0) { return 1.0; }
    if (x < 0.0) { return -1.0; }
    return 0.0;
}

float remapFloat(float x, float oldMin, float oldMax, float newMin, float newMax, boolean clamp) {
    float pct = (x - oldMin) / (oldMax - oldMin);
    if (clamp) {
        if (pct < 0.0) { pct = 0.0; }
        if (pct > 1.0) { pct = 1.0; }
    }
    return pct * (newMax - newMin) + newMin;
}

int clampInt(int x, int lo, int hi) {
    return max(lo, min(hi, x));
}

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

// TODO: test this

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

// I've given "key numbers" to each key which are different
// than the pin numbers.
//
//  KEY NUMBERS
//
//   00           12    13           11
//    01  02         14         09  10
//         03  04          07  08
//              05        06
//
// Current special behaviors:
//   12 and 13 are multimedia volume keys
//   14 is SHIFT
//   SHIFT + 00 (SHIFT + B) toggles between MIDI channel 0 and 1.
//   SHIFT + a white key tranposes the keyboard to different octaves.
//      F (06) gets you the default octave.
//
//   This is all implemented in the onKeyDown() function.
//

#define NUM_KEYS 15

//  my key number - arduino number - meaning
//  (my key number is in midi order)
#define KEY00_PIN  1  // B   // right side
#define KEY01_PIN 14  // C
#define KEY02_PIN  0  // c#
#define KEY03_PIN 16  // D
#define KEY04_PIN 15  // d#
#define KEY05_PIN 23  // E
#define KEY06_PIN  3  // F   // left side
#define KEY07_PIN  5  // f#
#define KEY08_PIN  9  // G
#define KEY09_PIN 10  // g#
#define KEY10_PIN 11  // A
#define KEY11_PIN 12  // a#

#define KEY12_PIN 22  // S (L)   // center keys
#define KEY13_PIN 20  // T (R)
#define KEY14_PIN 21  // U (SHIFT)

#define JOY_X_ANALOG_PIN A10  // joystick x axis (arduino pin 10)
#define JOY_Y_ANALOG_PIN  A9  // joystick y axis (arduino pin 9)

#define LED_PIN 13  // onboard red led

// unused pins
// 6 -- can be used for neopixels
// 2

// note name to key number (not pin number)
#define B_KEY   0
#define C_KEY   1
#define CS_KEY  2
#define D_KEY   3
#define DS_KEY  4
#define E_KEY   5
#define F_KEY   6
#define FS_KEY  7
#define G_KEY   8
#define GS_KEY  9
#define A_KEY  10
#define AS_KEY 11

#define L_KEY     12   // S
#define R_KEY     13   // T
#define SHIFT_KEY 14   // U

// key number to pin number
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

    KEY12_PIN,  // S (L)
    KEY13_PIN,  // T (R)
    KEY14_PIN,  // U (SHIFT)
};

//------------------------------------------------------
// STATE

// debounce keyswitches.  max 32000.
// when a keyswitch changes state, ignore it for this many ticks
// before trying to read it again.
#define KEY_COOLDOWN 1000

// do analog read of joystick every N ticks, because it's slow
// and we don't want to spam MIDI pitchbend signals too fast
#define READ_JOY_EVERY 100

// keys: arrays for debouncing & state tracking
bool keyIsDown[NUM_KEYS];
bool keyWasDown[NUM_KEYS];
int coolDown[NUM_KEYS];  // ticks until next read, if recently changed (for debouncing)

// midi
int baseTranspose = 60;  // what midi note the C key should play
int octaveTranspose = 0; // also shift this much (multiples of 12, like -24)
byte midiChannel = 0;  // this can be toggled between 0 and 1

byte keyToMidiNote(int key) {
    // key 0 is B-below-middleC, midi 59
    int result = key - 1 + baseTranspose + octaveTranspose;
    return (byte)clampInt(result, 0, 127);
}

// joystick
// range is a float -1.0 to 1.0
// positive is up-right.
// the middle deadzone (+/- joyDeadzone) is snipped out of the middle and
// the remaining range is scaled to fill that space.
// similarly the outer max zone (+/- joyMax) is snipped and the remaining
// range is stretched to fill that space.
float joyX = 0.0;
float joyY = 0.0;
float joyDeadzone = 0.1;  // between +/- this value, the joystick can move with no effect
float joyMax = 0.8;  // past +/- this value, the joystick is maxed out and has no further effect

int joyCooldown = 0;  // keep track of how long until next joystick read (see READ_JOY_EVERY)

#define DIR_NONE 0
#define DIR_N  1
#define DIR_E  2
#define DIR_S  3
#define DIR_W  4
byte joyDirection = DIR_NONE;

float computeJoyAxis(int rawJoy) {
    // rawJoy is the analog value of the X or Y axis, 0 to 1023
    // output is a float with dead zone and outer max zone removed, scaled to -1 to 1
    float joy = remapFloat((float)rawJoy, 0.0, 1023.0, -1.0, 1.0, true);
    float sg = signFloat(joy);
    joy = abs(joy);
    joy = remapFloat(joy, joyDeadzone, joyMax, 0, 1, true) * sg;
    return joy;
}

byte getJoyDirection(float joyX, float joyY) {
    if (joyX == 0.0 && joyY == 0.0) { return DIR_NONE; }
    if (abs(joyX) < abs(joyY)) {
        // vertical
        if (joyY > 0.0) { return DIR_N; }
        return DIR_S;
    } else {
        // horizontal
        if (joyX > 0.0) { return DIR_E; }
        return DIR_W;
    }
    return DIR_NONE;
}

//------------------------------------------------------
// SETUP

void setup() {
    pinMode(LED_PIN, OUTPUT);

    // keyswitch pins: input
    for (int ii = 0; ii < NUM_KEYS; ii++) {
        pinMode(keyToPin[ii], INPUT);
        keyIsDown[ii] = false;
        keyWasDown[ii] = false;
        coolDown[ii] = 0;
    }

    // don't need to set up the analog input pins

    // get ready to send multimedia volume keys
    Keyboard.begin();
}

//------------------------------------------------------
// MAIN

void loop() {
    // send MIDI data over USB, if there is anything queued up
    MidiUSB.flush();

    // read joystick every so often
    joyCooldown -= 1;
    if (joyCooldown <= 0) {
        // analogRead returns an int 0 - 1023, meaning 0 to 3.3 volts
        // remap to a float -1 to 1
        joyX = computeJoyAxis(analogRead(JOY_X_ANALOG_PIN));
        joyY = computeJoyAxis(analogRead(JOY_Y_ANALOG_PIN));
        //joyX = ((float)analogRead(JOY_X_ANALOG_PIN) / 1023.0 - 0.5) * 2.0;
        //joyY = ((float)analogRead(JOY_Y_ANALOG_PIN) / 1023.0 - 0.5) * 2.0;
        joyCooldown = READ_JOY_EVERY;

        // TODO: handle joystick motion, make pitch bends

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

    // blink LED for testing and send some midi notes.
    // this is incompatible with the rest of the code
    // because of the delay() calls
    //digitalWrite(13, HIGH);
    //MidiUSB.flush();
    //noteOn(0, 60, 100);
    //delay(200);
    //digitalWrite(13, LOW);
    //noteOff(0, 60, 100);
    //delay(2000);
}

//------------------------------------------------------
// KEY HANDLERS

void onKeyDown(int key) {
    if (keyIsDown[SHIFT_KEY]) {
        // special shift key behaviors

        // B key toggles MIDI channel between 0 and 1
        if (key == B_KEY) {
            allNotesOff(midiChannel);  // don't leave dangling notes in the old channel
            midiChannel = 1 - midiChannel;
        }

        // other white keys transpose by octaves
        if (key == C_KEY) { octaveTranspose = -36; }
        if (key == D_KEY) { octaveTranspose = -24; }
        if (key == E_KEY) { octaveTranspose = -12; }
        if (key == F_KEY) { octaveTranspose =  0; }  // default transpose
        if (key == G_KEY) { octaveTranspose =  12; }
        if (key == A_KEY) { octaveTranspose =  24; }
    } else {
        if (key >= 0 && key <= 11) {
            // midi keys send notes
            // TODO: shepard tones
            noteOn(midiChannel, keyToMidiNote(key), 100);
        } else if (key == L_KEY) {
            // multimedia volume keys
            volumeDown();
        } else if (key == SHIFT_KEY) {
            // shift key pressed; do nothing here
        } else if (key == R_KEY) {
            // multimedia volume keys
            volumeUp();
        }
    }
}
void onKeyUp(int key) {
    if (key >= 0 && key <= 11) {
        // midi keys end their notes
        noteOff(midiChannel, keyToMidiNote(key), 100);
    }
}
void onKeyStillDown(int key) {
}
