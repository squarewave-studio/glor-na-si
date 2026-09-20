# Glór na Sí

An instrument built on the archaeoacoustics of Brú na Bóinne. Made for the [Synthux Academy](https://www.synthux.academy/) residency, 2026.

## About

Glór na Sí is an instrument built on the archaeoacoustics of Brú na Bóinne. I live close enough to the site that this felt like the obvious thing to make. The passage tombs of the Boyne Valley have been studied for their sound as much as their architecture; the chambers ring. They hold onto some frequencies and let the rest fall away, and people have noticed this for as long as anyone has stood inside them.

Archaeoacoustics asks whether any of that was deliberate. I don't know, and the instrument doesn't need an answer.

So it treats a chamber the way you'd treat a bell or a length of pipe. Something goes in, the space decides what comes back out. I'm keeping whatever excites it fairly plain, because the interesting part should be the resonance and not the source. That thinking comes from Rings and Elements, and further back from Karplus-Strong and the VL1. Nothing is sampled either. The space is the instrument.

The one thing I keep returning to is the roof-box at Newgrange. For a few minutes around the winter solstice, light comes through a gap above the entrance and travels the whole length of the passage. A narrow opening that reaches all the way in, but only under the right conditions. That sits at the centre of how the instrument is played. One movement takes you from outside the entrance to the back of the chamber.

## Firmware

v0.3 runs on a Daisy Seed in a Synthux Simple Touch. Two sounds and one gesture: strike a drum or hold a chant where you stand, then walk the passage and hear how the mound carries it.

The firmware is a fork of Synthux Academy's [TouchString](https://github.com/Synthux-Academy/TouchString), with the string engine swapped for a drum and a chamber. See [CREDITS.md](CREDITS.md).

### Controls

<img src="touch.jpeg" width="300"/>

The board picture is Synthux Academy's, from the [TouchString](https://github.com/Synthux-Academy/TouchString) repository.

#### Switches
- **S07/S08** (right)
  - **Up**: Voice
  - **Centre** or **Down**: Drum
- **S09/S10** (left) - unused

#### Pads
- Drum: the front row P03 to P07 strike, rising left to right, and P09 below gives the top note
- Loop: tap P10 and play, in either mode, then tap it again and it comes round from the first thing you played. Drum hits, and vowels, each where you stood, so a loop laid down while walking is spread along the passage. The second tap is the bar, rests and all. Leave it twenty seconds without playing and the take ends by itself, trimmed to the last thing you played. P11 clears it
- The loop keeps going when you switch modes, so you can sing over a beat or drum over a phrase. A vowel still down when the take ends is the loop's from then on and keeps singing after the pad comes up, a drone under whatever you play next. A few vowels in turn come round as a phrase
- Voice: two voices. Each pad is a vowel at an interval above the pitch knob
  - P03 mmm, P04 ooh, P05 oh, P06 aah, P07 ah, at the root
  - P08 ooh, P09 aah, a fifth up
  - P00 ooh, P01 aah, an octave up. P02 overtone, a fifth up
  - A pad takes a silent voice first, then one the loop holds, then the older of the two under your fingers. Let go and it closes, unless the loop holds it

The drum is tuned under 110 Hz. Acoustic surveys of passage tombs in Ireland and Britain (Jahn, Devereux and Ibison, 1996) found the chambers resonate between roughly 95 and 120 Hz, so the chamber here rings at 110 Hz and the drum sits under it. The pads sit below it, where a strike sounds like skin and wood rather than a bell: P03 at 36.7 Hz, P04 at 41.25 Hz, P05 at 45.8 Hz, P06 at 55 Hz, P07 at 61.9 Hz and P09 at 68.75 Hz. Hit them all at once and you get one chord ringing the chamber, not six notes. Each hit starts a little sharp and settles in the first tenth of a second, as a skin does, more for a harder hit.

Each voice is two chants a few cents apart, built from 24 harmonics measured off a recorded chant at C2. Holding a pad opens a voice from a closed hum into that pad's vowel as it swells. Letting go closes it back down over a little longer.

#### Knobs (clockwise)
- S31 **Pitch** | Voice pitch, C2 at centre, an octave either way, not quantised, live on every voice. In Drum it is the **Tune**, the pads as built at centre and an octave either way, landing on the next strike
- S32 **Tone** | Drum body, from a woody thud to a metallic ring. Lands on the next strike. In Voice it is the **Spread**, how far apart the two singers sit, from one voice to 40 cents
- S33 **Decay** | Drum ring time, clockwise is longer. Lands on the next strike. In Voice it is the **Swell**, how long a note takes to open, 0.15 s to 3 s
- S34 **Unsteadiness** | How much each harmonic wanders and the pitch drifts. Centre is the chant as measured, left is still, right is twice as loose. A drum hit knocks the singers, more the nearer it lands and the less steady they are, and they settle back in half a second. In Drum it is the **Hardness**, from a soft mallet to a hard stick, landing on the next strike
- S35 **Level** | Drum level in Drum, voice level in Voice. Centre is the level as built, right is 6 dB up
- S30 **Speed** | How fast the loop runs. As played at centre, a quarter speed to the left, four times to the right. Nothing changes pitch. Works in either mode

S31 to S35 change job with the mode. After a switch, each knob keeps the setting that mode last had until you turn it back through that point, so flipping the switch never jumps a sound.

#### Faders
- S36 (left) **Walk** | Where you stand. Down is a few metres outside the entrance, up is the back of the chamber. The fader is where you are going, and you walk there at about five metres a second. A drum hit or a voice starts where you stand and stays there. Walk away and it goes distant, late and dull, and the room takes over. Walk back and it comes forward again
- S37 (right) - read but unused, reserved for the light box

#### LED
The onboard LED flashes on every strike in Drum. While a loop is recording it stays on and blinks off for every strike it takes. In Voice it stays on while a voice sings.

### The passage

The instrument is a line, measured in metres along the axis of Newgrange from O'Kelly's excavation plan. The entrance stone is 0. The passage runs 19 m to the chamber, which goes on for another 6 m, and the bottom of the walk fader stands 3 m outside. Sound travels along the line at 343 m/s, so a strike at the back of the chamber reaches the entrance about 65 ms later, and it loses level and top for every metre of stone it passes.

Each drum hit and each voice is pinned where you stood when it started. What you hear from it is the direct sound over the distance between you and the chamber's answer coming back down the passage, a reverb and a ring. The open mouth adds a dull inverted slap. Stand on top of a sound and it is dry and close. Walk away and the room takes over, then the whole thing recedes until outside there is only a murmur from the opening. The passage also colours what passes through it. Its walls are a metre apart, so everything rings faintly between them, and its height, low at the entrance and high by the chamber, gives it a note that slides as you walk.

Jahn, Devereux and Ibison (1996) measured the chambers of Irish and British passage tombs resonating between 95 and 120 Hz. The chamber here rings at 110 Hz, and more quietly at 86 Hz for the east recess, so a voice tuned onto the ring comes back louder than one that is not. The lengths, the losses and the ring are all in [config.h](common/config.h).

### Project Structure
```
glor-na-si/
├── GlorNaSi.cpp         # Main application entry point
├── Makefile             # Build configuration
├── common/              # Configuration and utilities
├── mound/               # Instrument core (drum, chant engine, passage)
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
Edit [config.h](common/config.h) to retune the pads, the drum ranges and the passage: its lengths, its losses, the ring and the walking speed.
