# Thumble

A MIDI controller in the shape of a video game controller (e.g. xbox), but using mechanical-keyboard hardware.

## How it works musically; how to play it

Use your thumbs!

## Hardware

1. [Feather 32u4 basic proto](https://www.adafruit.com/product/2771), from Adafruit
2. Cherry MX compatible keyswitches (x 15)
    * I used [Gateron Silent Clear](https://novelkeys.xyz/products/gateron-silent-switches?_pos=2&_sid=fd79d8abc&_ss=r) since they have the weakest springs available and no tactile bump
3. Keycaps (x 15)
    * I used these [XDA profile](https://www.amazon.com/gp/product/B092H93KP6/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1) keycaps because they're pretty flat on top so it's easy to slide your thumbs around
4. This [joystick](https://www.adafruit.com/product/245) from Adafruit
4. Some wires
5. Soldering equipment

## Case

I used a laser cutter to cut the enclosure.  I cut it from 3mm plywood (1/8 inch), but you could also use acrylic if you added more screw holes.  See the SVG files in the `case` folder.

The case is made of several layers stacked up.  With wood you can glue them; with acrylic you'll have to figure out screws or something.

## Soldering

TODO

## Arduino setup

Install the Adafruit board profiles using instructions from their site.

set board: Adafruit Feather 32u4 basic proto

add libraries: MIDIUSB




