# Changelog

Each release has its built binary attached on the [releases page](https://github.com/squarewave-studio/glor-na-si/releases). Flash one with the [Daisy web programmer](https://electro-smith.github.io/Programmer/), or put the Seed in DFU mode and run

    dfu-util -a 0 -s 0x08000000:leave -D glor-na-si-<version>.bin -d ,0483:df11

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
