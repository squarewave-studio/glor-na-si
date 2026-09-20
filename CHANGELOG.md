# Changelog

Each release has its built binary attached on the [releases page](https://github.com/squarewave-studio/glor-na-si/releases). Flash one with the [Daisy web programmer](https://electro-smith.github.io/Programmer/), or put the Seed in DFU mode and run

    dfu-util -a 0 -s 0x08000000:leave -D glor-na-si-<version>.bin -d ,0483:df11

## v0.5, 20 September 2026

[glor-na-si-v0.5.bin](https://github.com/squarewave-studio/glor-na-si/releases/tag/v0.5)

- A new voice. The chant engine and its measured harmonics are gone. Each singer is now a glottis and a mouth: a model of the vocal folds with breath through it, into five resonances placed where a low male voice puts them for each sound. The sound stays itself when the pitch moves, as a mouth does, and there is top and breath where the old voice had neither
- Five sounds on the front row: hmm, ooh, aah, ho, aww. hmm and ho open with a soft breath
- The other pads sing whatever the front row sang last. P00, P01 and P02 take it a fourth, a fifth and an octave up. P08 breaks the voice into an undertone an octave below. P09 picks one overtone out of it and holds it
- Every note is sung like a person sings it: a scoop in from below, or now and then a flick from above, vibrato that arrives a moment after the onset, a little drift, and a settle as the note stops
- S34 runs from a machine-steady note on the left, through a person at centre, to a voice coming apart on the right
- The swell runs 40 ms to 2 s, so a note can start like a syllable
- The loop remembers which sound each vowel was sung with, so the top and bottom pads come round as they were played

## v0.4, 20 September 2026

[glor-na-si-v0.4.bin](https://github.com/squarewave-studio/glor-na-si/releases/tag/v0.4)

- The drum is a skin. Eight modes of a circular membrane at the Bessel ratios, and where you strike decides how much each rings: the centre deep and pure, the rim thin and bright. The pads are places on one skin, not notes. The front row is a line across it, P05 the centre, and P08 and P09 are the stick near the rim
- The strike is a mallet pulse, the same push however hard, so hardness is brightness and the stick, not loudness. No two hits are quite the same
- S31 in Drum is the size, half to double. S32 is the stiffness, an ideal membrane to a hide. S33 the decay, S34 the hardness
- Three stones on the top row in Drum, the same stick into a few short bright modes and no skin
- The modal body from DaisySP is gone, and with it most of the drum's half of the core. The skin and the stones cost about one percent
- Two loops, one for the drum and one for the voices, each with its own bar and both always playing. P10 takes into the loop of the mode you are in, a take while it runs replaces it, P11 clears that loop
- The LED is the same in both modes: a flash for everything played, on while recording and off for a beat for everything taken
- The mode not on at power-up starts from its own settings rather than wherever the knobs happen to sit, and picks up from there
- The skin's decay runs 1.2 s to 4 s. The old bottom end was too short to sound like a drum
- The delay lines moved from SDRAM to the core's own memory. Every path runs through them, and the external memory was too slow for it
- The drum's attack is a dull thwack, not a click

## v0.3, 20 September 2026

[glor-na-si-v0.3.bin](https://github.com/squarewave-studio/glor-na-si/releases/tag/v0.3)

- The passage. The instrument is a line in metres from O'Kelly's plan, 3 m outside the entrance to the back of the chamber. Every drum hit and every voice is pinned where you stood when it started, and the walk fader is where you go, at about five metres a second
- Every path is delayed by its distance and loses level and top for every metre: the direct sound, the chamber's answer coming back down the passage and a dull inverted slap from the open mouth
- The chamber rings at 110 Hz, and more quietly at 86 Hz. A voice held on the ring comes back louder
- The passage colours what passes through it: a faint ring between its walls a metre apart, and a note from its height where you stand that slides with the walk
- The loop takes voices too. Tap P10, play in either mode, tap again, and drum hits and vowels come round, each where you stood. A vowel still down when the take ends keeps singing on its own, so the latch toggle goes and S09/S10 is free. A take left alone ends itself
- Walk moves to the left fader, S36, so the right hand stays on the pads. The light box is reserved on S37
- S30 is the loop speed, a quarter to four times, as played at centre. Hits and vowels speed up together and nothing changes pitch
- A drum hit knocks the singers. Their wander, drift and rasp jump, more the nearer the hit landed and the less steady they are, and settle back in half a second
- The swell can be as short as 0.15 s
- S31 in Drum tunes the pads an octave either way, so the drum can be tuned onto the room too. S34 in Drum is the hardness, soft mallet to hard stick. Both hold and pick up across the mode switch like S32, S33 and S35
- Each drum hit starts a little sharp and settles in the first tenth of a second, as a skin does, more for a harder hit
- The drum's tone no longer follows the fader. The passage does that now
- The drum body stops running once it has gone silent. That frees most of its half of the core between hits
- The delay lines live in the Seed's SDRAM

## v0.2, 19 September 2026

[glor-na-si-v0.2.bin](https://github.com/squarewave-studio/glor-na-si/releases/tag/v0.2)

- Chant voice on the S07/S08 toggle. Two voices, each a pair of chants a few cents apart, built from 24 harmonics measured off a recorded chant at C2
- Vowels on the pads. mmm, ooh, oh, aah and ah along the front row at the root, ooh and aah a fifth up on P08 and P09, an octave up on P00 and P01, the overtone shape at the fifth on P02
- Latch on the S09/S10 toggle. Latched voices carry into Drum, so you can play the drum over them
- S31 Pitch, live and not quantised, an octave either way of C2
- S32 Spread and S33 Swell in Voice. S34 Unsteadiness in either mode
- S35 Level, drum level in Drum and voice level in Voice
- Knobs that change job with the mode pick up. After a switch, a knob holds that mode's last setting until you turn it back through it
- Drum loop. Hold P10, play, let go and it loops from the first hit. P11 clears. Keeps going in Voice
- Drum pads on the front row and P09, retuned under 70 Hz, where the body still sounds like skin and wood
- Audio block of 96 at 480 MHz, so the voice's wavetable rebuild fits inside the callback
- Board picture in the README, from TouchString

## v0.1, 18 September 2026

[glor-na-si-v0.1.bin](https://github.com/squarewave-studio/glor-na-si/releases/tag/v0.1)

- Fork of Synthux Academy's TouchString with the string engine replaced
- Six drum pads on a modal body, tuned around the chamber's 110 Hz
- Walk on the right fader. Entrance to chamber, the tail lengthens and darkens and the room rises over the hit
- S32 sets the drum tone and S33 the decay. Both land on the next strike
- LED flash on every strike
