# Glór na Sí

An instrument built on the archaeoacoustics of Brú na Bóinne. Made for the [Synthux Academy](https://www.synthux.academy/) residency, 2026.

## About

Glór na Sí is an instrument built on the archaeoacoustics of Brú na Bóinne. I live close enough to the site that this felt like the obvious thing to make. The passage tombs of the Boyne Valley have been studied for their sound as much as their architecture; the chambers ring. They hold onto some frequencies and let the rest fall away, and people have noticed this for as long as anyone has stood inside them.

Archaeoacoustics asks whether any of that was deliberate. I don't know, and the instrument doesn't need an answer.

So it treats a chamber the way you'd treat a bell or a length of pipe. Something goes in, the space decides what comes back out. I'm keeping whatever excites it fairly plain, because the interesting part should be the resonance and not the source. That thinking comes from Rings and Elements, and further back from Karplus-Strong and the VL1. Nothing is sampled either. The space is the instrument.

The one thing I keep returning to is the roof-box at Newgrange. For a few minutes around the winter solstice, light comes through a gap above the entrance and travels the whole length of the passage. A narrow opening that reaches all the way in, but only under the right conditions. That sits at the centre of how the instrument is played. One movement takes you from outside the entrance to the back of the chamber.

## Firmware

v0.1 runs on a Daisy Seed in a Synthux Simple Touch. One sound and one gesture: strike a drum, then walk the passage and hear the chamber answer back.

The firmware is a fork of Synthux Academy's [TouchString](https://github.com/Synthux-Academy/TouchString), with the string engine swapped for a drum and a chamber. See [CREDITS.md](CREDITS.md).

### Controls

#### Pads
- P00, P03, P04, P08, P01, P05 - drum strikes
- P02, P06, P07, P09, P10, P11 - unused

The pads are tuned to 110 Hz and below. Acoustic surveys of passage tombs in Ireland and Britain (Jahn, Devereux and Ibison, 1996) found the chambers resonate between roughly 95 and 120 Hz, so I use 110 Hz for Newgrange. P05 sits on that note. The rest sit under it, where a hit sounds like skin and wood rather than a bell: P01 at 82.5 Hz, P08 at 68.75 Hz, P00 at 55 Hz, P04 at 41.25 Hz and P03 at 36.7 Hz. Hit them all at once and you get one chord ringing the chamber, not six notes.

#### Knobs
- S32 **Tone** | Drum body, from a woody thud to a metallic ring. Lands on the next strike
- S33 **Decay** | Drum ring time, clockwise is longer. Lands on the next strike
- S30, S31, S34, S35 - unused

#### Faders
- S37 (right) **Walk** | Position along the passage. Down is the entrance, up is the chamber. Push it up and the tail gets longer and darker until the room is louder than the drum
- S36 (left) - read but unused, reserved for the light box

#### Switches
Unused.

#### LED
The onboard LED flashes on every strike.

### Project Structure
```
glor-na-si/
├── GlorNaSi.cpp         # Main application entry point
├── Makefile             # Build configuration
├── common/              # Configuration and utilities
├── tomb/                # Instrument core (drum, chamber, walk)
├── touch/               # Simple Touch wrapper (pads, knobs, switches)
└── ui/                  # UI connecting instrument core with touch wrapper
```

### Project Setup
The firmware builds against the libDaisy and DaisySP checkouts inside a Synthux TouchString clone, which the Makefile expects at `~/Development/Daisy/TouchString`. Set `TOUCHSTRING_DIR` to use a clone somewhere else.

```shell
$ git clone --recurse-submodules https://github.com/Synthux-Academy/TouchString.git ~/Development/Daisy/TouchString
$ make -C ~/Development/Daisy/TouchString libs -j8
$ make clean; make -j8
$ make program-dfu
```

For the [Daisy web programmer](https://electro-smith.github.io/Programmer/), flash `build/GlorNaSi.bin`.

### Configuration
Edit [config.h](common/config.h) to retune the pads, the drum ranges and the walk.
