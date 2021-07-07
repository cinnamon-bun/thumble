<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Contents**

- [Thumble](#thumble)
  - [How it's designed musically; how to play it](#how-its-designed-musically-how-to-play-it)
    - [Musical notes](#musical-notes)
    - [Special keys](#special-keys)
    - [Joystick](#joystick)
  - [Shepard Tones](#shepard-tones)
  - [Hardware](#hardware)
    - [Alternative hardware](#alternative-hardware)
  - [Case](#case)
  - [Assembly sequence](#assembly-sequence)
  - [Arduino setup](#arduino-setup)
  - [The firmware code](#the-firmware-code)
  - [Soldering](#soldering)
- [Build photos](#build-photos)
  - [Laser cutting](#laser-cutting)
  - [Case Assembly](#case-assembly)
  - [Electronics](#electronics)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Thumble

![](beautyshot.png)

![](build_photos/photo_2021-07-07_08-24-30.jpg)

A MIDI controller in the shape of a video game controller (e.g. xbox), but using mechanical-keyboard buttons.

## How it's designed musically; how to play it

Hold it like an Xbox controller.  Play it like a kalimba, with your thumbs.

The diagonal keys are notes; the 3 keys in the middle are extra function keys.  The joystick is for pitch bends.

### Musical notes

Unlike a kalimba, the notes are arranged piano style.  There's one octave starting at B on the left and ascending to Bb on the right.  Like a piano, white keys are on the bottom row and black keys are on the top row (except for B).

Because we use Shepard Tones (more below), you can play a melody so that it wraps off one side of the keyboard and back onto the other side, without noticing a jump in octaves.

### Special keys

The middle keys S, T, and U are for special purposes.  Currently:

* S is volume down (like a multimedia keyboard -- controlling the computer volume)
* T is volume up
* U is a SHIFT key
    * SHIFT - B toggles between MIDI channel 0 and 1
    * SHIFT + a white key tranposes the keyboard to different octaves.  F gets you the default octave.

...but check the [source code](https://github.com/cinnamon-bun/thumble/blob/main/thumble_001_arduino/thumble_001_arduino.ino#L97-L102) for the latest changes to the special keys.

The middle keys could possibly do other things like:
* play specific MIDI pitches where drum sounds can be set up, that would ignore the transpose settings and pitch bends.

### Joystick

The joystick is not programmed yet (TODO), but it will do pitch bends.  Left/right will bend by a half step, and up/down will bend by a whole step.

It could also act as a mod wheel on one axis and a pitch bend on the other axis.

## Shepard Tones

There's only room for 12 notes, but wouldn't it be nice if you could wrap around the edge when playing a melody without jumping an octave?

We use an auditory illusion called [Shepard Tones](https://en.wikipedia.org/wiki/Shepard_tone) to make this possible.  Each key actually plays several octaves of that note at the same time, but with louder notes in the middle octaves, to maintain the loudest note in the same octave despite the illusion of constantly ascending tones.

As you go up, the higher octaves fade away and the lower octaves fade in to take their place, until you wrap around seamlessly.

For example:
* C plays C1 at 50% and C2 at 50% volume.
* D plays D1 at 58% and D2 at 42%.
* ...etc, with the upper octave fading away until we get to the middle of the scale and the upper octave is gone.  Then we start bringing in the octave below...
* F plays F1 at 92% and F2 at 8%.
* F# plays F#1 at 100%
* G plays G0 at 8% and G1 at 92%
* and eventually we end up back where we started:
* B plays B0 at 42% and B1 at 58%

## Hardware

1. Arduino / microcontroller: [Feather 32u4 basic proto](https://www.adafruit.com/product/2771), from Adafruit.  If these are out of stock, look for the exact same thing on Amazon (sold by Adafruit).
1. Cherry MX compatible keyswitches (x 15)
    * I used [Gateron Silent Clear](https://novelkeys.xyz/products/gateron-silent-switches?_pos=2&_sid=fd79d8abc&_ss=r) since they have the weakest springs available and no tactile bump
1. Keycaps (x 15)
    * I used these [XDA profile](https://www.amazon.com/gp/product/B092H93KP6/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1) keycaps because they're pretty flat on top so it's easy to slide your thumbs around
1. This [joystick](https://www.adafruit.com/product/245) from Adafruit
1. [Magnetic USB cables](https://www.amazon.com/gp/product/B07GWLF4GR/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1) to avoid strain on the USB port.
1. Some wires
1. Soldering equipment

### Alternative hardware

The [Elite-C](https://keeb.io/products/elite-c-low-profile-version-usb-c-pro-micro-replacement-atmega32u4) uses the same chip as the Feather, but the physical hardware is better.   It's smaller, and it has a strong USB-C port, while the Feather's USB-Micro port tends to rip off if the cord is pulled.

However the Elite-C can't be flashed from the Arduino IDE (because it uses the DFU bootloader instead of the Caterina bootloader that's normal for Arduino boards.)  You have to use the command line to flash it, and I'm not sure how to compile the code for it or make sure it supports MIDI-over-USB.

If you know how to solve that problem, you can use the Elite-C instead.  Note though that the case is designed for the Feather, which is larger.

## Case

The case is laser cut from 7 layers of 1/8 inch material (3mm).  I used plywood and glued the layers, or you could use acrylic and figure out some screw holes.  The SVG files are in the `case` folder.

3mm plywood is too thick for the keyswitches to grab onto with their little latches.  You'll have to glue them into place (TODO: with what glue?).

The joystick and microcontroller are also a bit loose in the case; you'll have to improvise a way to keep them stable.  Maybe a spare piece of wood to pack the space below them.

The top layer SVG is flipped so that the laser smoke marks end up inside the case instead of outside.  Case level 4 is hollow, make three of those to stack up enough interior space for wiring.

## Assembly sequence

1. Software setup:
    1. Install the Arduino IDE and set it up as described below.
    1. Flash software to the microcontroller before you do anything else, just to make sure you can.  You won't be able to test it though.
1. Case:
    1. Laser-cut case layers from plywood.
    1. Align plywood layers, test-fitting keyswitches to ensure precise alignment.  (The keyswitches will not clip in securely; we glue them in a later step.)
    1. Glue all plywood layers except the back panel.
    1. Sand and finish the plywood.  Round off the edges of the handle to make it more comfortable to hold and to remove the burned part.
1. Assembly:
    1. Glue in the keyswitches, being carful not to get glue on the metal contacts.
    1. Put in the joystick and microcontroller and affix them somehow.
    1. Soldering time.
1. Testing:
    1. Test device functionality, press buttons, connect to MIDI software and see if it works.  Try using a "midi spy" program like Midi Monitor to observe the MIDI events it's sending.
1. Final packaging:
    1. Add more supports around the back of the joystick and microcontroller to hold them steady in their little caves when squeezed by the back panel.  Maybe glue some scrap wood?
    1. Also add some scrap wood to hold the USB port tight when the back cover is on.  The USB port tends to get ripped out when the cable is yanked.  Some superglue might help here if you're careful.
    1. Put the back panel on with wood screws, or use M2.5 machine screws in the provided holes.  You might need to open it again later.
    1. Put keycaps on the keyswitches.

## Arduino setup

1. Install the official [Arduino IDE](https://www.arduino.cc/en/software).  The "hourly build" is free, the others cost money.
2. In the Arduino IDE, install the Adafruit board profiles using [instructions from Adafruit](https://learn.adafruit.com/adafruit-feather-32u4-basic-proto/arduino-ide-setup).  Specifically, the Adafruit board manager URL to add is `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
3. Set board: Adafruit Feather 32u4 basic proto
4. Add libraries: MIDIUSB

Open the arduino code in `thumble_001_arduino`.  Hit the checkmark button to see if it compiles with no problems.

To send the code to the board, press the button with the right arrow.  You do not need to press the reset button on the board when flashing it.

## The firmware code

This code will turn the microcontroller into a USB MIDI input device, like a piano keyboard, that can be used with any other hardware (ipad, laptop, etc).

This is not based on any existing mechanical keyboard firmware because it's fairly simple.

## Soldering

We can wire each key directly between GND and an input pin, since we don't have many keys.  (Otherwise in a bigger keyboard we'd build a key matrix using diodes.)

For simplicity, a few GND wires can run along tracks in the case and connect to every key.  Then attach a signal wire from each key to a unique pin on the microcontroller.

We use 15 digital input pins for keys.

The joystick uses up two analog inputs and needs a ground and 3v wire too.

Wiring diagram ([click to enlarge](https://raw.githubusercontent.com/cinnamon-bun/thumble/main/final_v006-wiring.png)):

![](final_v006-wiring.png)

You can adjust the key-to-pin assignments in the code if you soldered things a bit differently.  Look for the `#define KEY00_PIN` lines around line 90.

# Build photos

## Laser cutting

Laser cutting the case layers from 1/8 inch plywood
![](build_photos/PXL_20210626_213331578.jpg)

## Case Assembly

Gluing the case layers
![](build_photos/PXL_20210628_180228637.jpg)

Test fitting the switches for layer alignment
![](build_photos/PXL_20210628_180246104.jpg)

Gluing
![](build_photos/PXL_20210628_180300707.jpg)

We used TiteBondIII Ultimate Wood Glue
![](build_photos/PXL_20210628_180313621.jpg)

Clamping & gluing
![](build_photos/messages_0(12).jpeg)
![](build_photos/messages_0(13).jpeg)

The layer stack showing the USB area
![](build_photos/messages_0(11).jpeg)

Counterset Torx screws to hold the back over
![](build_photos/messages_0(10).jpeg)
![](build_photos/messages_0(8).jpeg)

Sanding round edges
![](build_photos/messages_0(1).jpeg)
![](build_photos/messages_0(2).jpeg)
![](build_photos/messages_0(3).jpeg)

## Electronics

Fitting the electronics.  We had to cut off a corner of the case to make room for the USB cord.
![](build_photos/messages_0(5).jpeg)
![](build_photos/messages_0(6).jpeg)

Improvised way to hold the Feather board steady
![](build_photos/messages_0(7).jpeg)

Final case assembly
![](build_photos/messages_0.jpeg)
![](build_photos/photo_2021-07-07_08-25-05.jpg)

Soldering
![](build_photos/photo_2021-07-07_08-24-45.jpg)

Keycap layout
![](build_photos/PXL_20210628_162834118.jpg)

(Not pictured: flashing the firmware.)

Finished!
![](build_photos/photo_2021-07-07_08-24-30.jpg)