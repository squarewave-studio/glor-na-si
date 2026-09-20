# Glór na Sí

An instrument built on the archaeoacoustics of Brú na Bóinne. Made for the [Synthux Academy](https://www.synthux.academy/) residency, 2026.

## About

Glór na Sí is an instrument built on the archaeoacoustics of Brú na Bóinne. I live close enough to the site that this felt like the obvious thing to make. The passage tombs of the Boyne Valley have been studied for their sound as much as their architecture; the chambers ring. They hold onto some frequencies and let the rest fall away, and people have noticed this for as long as anyone has stood inside them.

Archaeoacoustics asks whether any of that was deliberate. I don't know, and the instrument doesn't need an answer.

So it treats a chamber the way you'd treat a bell or a length of pipe. Something goes in, the space decides what comes back out. I'm keeping whatever excites it fairly plain, because the interesting part should be the resonance and not the source. That thinking comes from Rings and Elements, and further back from Karplus-Strong and the VL1. Nothing is sampled either. The space is the instrument.

The one thing I keep returning to is the roof-box at Newgrange. For a few minutes around the winter solstice, light comes through a gap above the entrance and travels the whole length of the passage. A narrow opening that reaches all the way in, but only under the right conditions. That sits at the centre of how the instrument is played. One movement takes you from outside the entrance to the back of the chamber.

## Firmware

v0.2 runs on a Daisy Seed in a Synthux Simple Touch. Two sounds and one gesture: strike a drum or hold a chant, then walk the passage and hear the chamber answer back.

The firmware is a fork of Synthux Academy's [TouchString](https://github.com/Synthux-Academy/TouchString), with the string engine swapped for a drum and a chamber. See [CREDITS.md](CREDITS.md).

### Controls

<img src="touch.jpeg" width="300"/>

The board picture is Synthux Academy's, from the [TouchString](https://github.com/Synthux-Academy/TouchString) repository.

#### Switches
- **S07/S08** (right)
  - **Up**: Voice
  - **Centre** or **Down**: Drum
- **S09/S10** (left)
  - **Up**: Latch. Voices keep singing after their pads come up, and carry across to Drum so you can play the drum over them
  - **Centre** or **Down**: off

#### Pads
- Drum: the front row P03 to P07 strike, rising left to right, and P09 below gives the top note
- Drum loop: hold P10, play the drum pads, let go and it loops what you played from the first hit. Hold for the length of the bar, or let go late and it still comes round on time. P11 clears it. The loop keeps going when you switch to Voice, so you can sing over it
- Voice: two voices. Each pad is a vowel at an interval above the pitch knob
  - P03 mmm, P04 ooh, P05 oh, P06 aah, P07 ah, at the root
  - P08 ooh, P09 aah, a fifth up
  - P00 ooh, P01 aah, an octave up. P02 overtone, a fifth up
  - A pad takes a silent voice first, then a latched one, then the older of the two. Let go and it closes unless latched

The drum is tuned under 110 Hz. Acoustic surveys of passage tombs in Ireland and Britain (Jahn, Devereux and Ibison, 1996) found the chambers resonate between roughly 95 and 120 Hz, so I use 110 Hz for Newgrange, and the room carries that note rather than the hit. The pads sit below it, where a strike sounds like skin and wood rather than a bell: P03 at 36.7 Hz, P04 at 41.25 Hz, P05 at 45.8 Hz, P06 at 55 Hz, P07 at 61.9 Hz and P09 at 68.75 Hz. Hit them all at once and you get one chord ringing the chamber, not six notes.

Each voice is two chants a few cents apart, built from 24 harmonics measured off a recorded chant at C2. Holding a pad opens a voice from a closed hum into that pad's vowel as it swells. Letting go closes it back down over a little longer.

#### Knobs (clockwise)
- S31 **Pitch** | Voice pitch, C2 at centre, an octave either way, not quantised, live on every voice. Works in either mode
- S32 **Tone** | Drum body, from a woody thud to a metallic ring. Lands on the next strike. In Voice it is the **Spread**, how far apart the two singers sit, from one voice to 40 cents
- S33 **Decay** | Drum ring time, clockwise is longer. Lands on the next strike. In Voice it is the **Swell**, how long a note takes to open, 0.4 s to 3 s
- S34 **Unsteadiness** | How much each harmonic wanders and the pitch drifts. Centre is the chant as measured, left is still, right is twice as loose. Works in either mode
- S35 **Level** | Drum level in Drum, voice level in Voice. Centre is the level as built, right is 6 dB up
- S30 - unused

S32, S33 and S35 change job with the mode. After a switch, each knob keeps the setting that mode last had until you turn it back through that point, so flipping the switch never jumps a sound.

#### Faders
- S37 (right) **Walk** | Position along the passage. Down is the entrance, up is the chamber. Push it up and the tail gets longer and darker until the room is louder than the drum
- S36 (left) - read but unused, reserved for the light box

#### LED
The onboard LED flashes on every strike in Drum. While a loop is recording it stays on and blinks off for every strike it takes. In Voice it stays on while a voice sings.

### Project Structure
```
glor-na-si/
├── GlorNaSi.cpp         # Main application entry point
├── Makefile             # Build configuration
├── common/              # Configuration and utilities
├── mound/               # Instrument core (drum, chant engine, chamber, walk)
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

For the [Daisy web programmer](https://electro-smith.github.io/Programmer/), flash `build/GlorNaSi.bin`, or a build from the [releases page](https://github.com/squarewave-studio/glor-na-si/releases) if you would rather skip the toolchain. [CHANGELOG.md](CHANGELOG.md) says what each one does.

### Configuration
Edit [config.h](common/config.h) to retune the pads, the drum ranges and the walk.
