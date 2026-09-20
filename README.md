# Glór na Sí

An instrument built on the archaeoacoustics of Brú na Bóinne. Made for the [Synthux Academy](https://www.synthux.academy/) residency, 2026.

## About

Glór na Sí is an instrument built on the archaeoacoustics of Brú na Bóinne. I live close enough to the site that this felt like the obvious thing to make. The passage tombs of the Boyne Valley have been studied for their sound as much as their architecture; the chambers ring. They hold onto some frequencies and let the rest fall away, and people have noticed this for as long as anyone has stood inside them.

Archaeoacoustics asks whether any of that was deliberate. I don't know, and the instrument doesn't need an answer.

So it treats a chamber the way you'd treat a bell or a length of pipe. Something goes in, the space decides what comes back out. I'm keeping whatever excites it fairly plain, because the interesting part should be the resonance and not the source. That thinking comes from Rings and Elements, and further back from Karplus-Strong and the VL1. Nothing is sampled either. The space is the instrument.

The one thing I keep returning to is the roof-box at Newgrange. For a few minutes around the winter solstice, light comes through a gap above the entrance and travels the whole length of the passage. A narrow opening that reaches all the way in, but only under the right conditions. That sits at the centre of how the instrument is played. One movement takes you from outside the entrance to the back of the chamber.

## Firmware

v0.5 runs on a Daisy Seed in a Synthux Simple Touch. Two sounds and one gesture: strike a skin or hold a voice where you stand, then walk the passage and hear how the mound carries it.

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
- Drum: one skin. The front row P03 to P07 is a line across it, P05 the centre and the others out towards the rim, and P08 and P09 below are the stick near the rim. The top row P00 to P02 is three stones
- Loops: two of them, one for the drum and one for the voices, each with its own bar, and both keep playing whichever mode you are in. Tap P10 and play, then tap it again, and what you played comes round from the first thing you played, each hit or vowel where you stood, so a loop laid down while walking is spread along the passage. The second tap is the bar, rests and all. Tap P10 again while that loop runs and the next take replaces it, bar and all. Leave a take twenty seconds without playing and it ends by itself, trimmed to the last thing you played. P11 clears the loop of the mode you are in
- A vowel still down when the take ends is the voice loop's from then on and keeps singing after the pad comes up, a drone under whatever you play next. A few vowels in turn come round as a phrase, and a beat of a different length underneath drifts against it
- Voice: two voices. The front row is five sounds at the pitch knob's note, and the other pads sing whatever the front row sang last
  - P03 hmm, P04 ooh, P05 aah, P06 ho, P07 aww, at the root
  - P00, P01, P02: the last sound a fourth, a fifth and an octave up
  - P08: the last sound with the voice breaking into an undertone, an octave below the note. P09: the last sound with one overtone picked out and held, the ninth harmonic of the note
  - A pad takes a silent voice first, then one the loop holds, then the older of the two under your fingers. Let go and it closes, unless the loop holds it

The drum is a circular skin, built from the physics of a membrane rather than from a recording. A skin has a fixed set of modes at fixed ratios, and where you strike decides how much each one rings: at the centre only the round modes sound, deep and pure, and out towards the rim the others come in and the hit gets thinner and brighter. Nothing is retuned between pads. It is one drum, played across its face, the way a bodhrán is. The fundamental sits at 55 Hz with S31 at centre, under the chamber's 110 Hz, and each hit starts a little sharp and settles in the first tenth of a second as the tension gives. The strike is a mallet pulse, the same push however hard, so hardness changes the sound and not the loudness, and no two hits are quite the same. Acoustic surveys of passage tombs in Ireland and Britain (Jahn, Devereux and Ibison, 1996) found the chambers resonate between roughly 95 and 120 Hz, so the chamber here rings at 110 Hz.

The stones are the same stick on stone: the strike into a few short bright modes and no skin at all. They have no note of their own to speak of, which makes them the plainest way to hear what the chamber does.

Each voice is two singers a few cents apart, and each singer is a glottis and a mouth. The glottis is a model of the vocal folds, the Liljencrants-Fant pulse, with breath through it. The mouth is five resonances at the frequencies a low male voice puts them for each sound, so the sound stays itself when the pitch moves, as a real mouth does. Every note is sung the way a person sings it: it scoops in from below, or now and then flicks in from above, the vibrato arrives a moment after the onset, the pitch drifts a little, and the note settles as it stops. hmm and ho open with a soft breath before the voice. Nothing is sampled.

#### Knobs (clockwise)
- S31 **Pitch** | Voice pitch, C2 at centre, an octave either way, not quantised, live on every voice. Nudged down to A, the octave pad lands on the chamber's ring. In Drum it is the **Size**, the skin as built at centre, half as big to the right and twice as big to the left, landing on the next strike
- S32 **Stiffness** | How much the skin behaves like a hide rather than an ideal membrane, stretching its upper modes sharp. Lands on the next strike. In Voice it is the **Spread**, how far apart the two singers sit, from one voice to 40 cents
- S33 **Decay** | Skin ring time, 1.2 s to 4 s, clockwise is longer. Lands on the next strike. In Voice it is the **Swell**, how long a note takes to open, 40 ms to 2 s
- S34 **Unsteadiness** | Fully left the singers hold a note as steady as a machine. Centre is a person: the scoop, the vibrato, the drift. To the right the voice comes apart, wider and slower vibrato, the pitch wandering, breath in the tone, rasp and creak, the mouth moving on its own. A drum hit knocks the singers further apart, more the nearer it lands, and they settle back in half a second. In Drum it is the **Hardness**, from a soft mallet to a hard stick, landing on the next strike. A harder hit is a shorter contact, so it is brighter and has more of the stick in it, not louder
- S35 **Level** | Drum level in Drum, voice level in Voice. Centre is the level as built, right is 6 dB up
- S30 **Speed** | How fast the loop runs. As played at centre, a quarter speed to the left, four times to the right. Nothing changes pitch. Works in either mode

S31 to S35 change job with the mode. After a switch, each knob keeps the setting that mode last had until you turn it back through that point, so flipping the switch never jumps a sound. The mode that is on at power-up follows the knobs. The other starts from its own settings and its knobs pick up from there. For Voice that is pitch at C2, a small spread, a swell of about a quarter of a second, unsteadiness and level at centre. For Drum it is the skin as built, no stiffness, decay and level at centre and a fairly soft mallet.

#### Faders
- S36 (left) **Walk** | Where you stand. Down is a few metres outside the entrance, up is the back of the chamber. The fader is where you are going, and you walk there at about five metres a second. A drum hit or a voice starts where you stand and stays there. Walk away and it goes distant, late and dull, and the room takes over. Walk back and it comes forward again
- S37 (right) - read but unused, reserved for the light box

#### LED
The onboard LED flashes on everything you play, a hit or a vowel, in either mode. While a loop is recording it stays on and blinks off for everything it takes.

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
├── mound/               # Instrument core (drum, voice, passage)
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
Edit [config.h](common/config.h) to retune the pads, the drum ranges, the voice's sounds and the passage: its lengths, its losses, the ring and the walking speed.
