# Changelog

Each release keeps its built binary in `firmware/`. Flash one with the Daisy web programmer, or copy it over `build/GlorNaSi.bin` and run `make program-dfu`.

## v0.2, 19 September 2026

`firmware/glor-na-si-v0.2.bin`

- Chant voice on the S07/S08 toggle. Two voices, each a pair of chants a few cents apart, built from 24 harmonics measured off a recorded chant at C2
- Vowels on the pads. mmm, ooh, oh, aah and ah along the front row at the root, ooh and aah a fifth up on P08 and P09, an octave up on P00 and P01, the overtone shape at the fifth on P02
- Latch on the S09/S10 toggle. Latched voices carry into Drum, so the drum can be played over them
- S31 Pitch, live and not quantised, an octave either way of C2
- S32 Spread and S33 Swell in Voice. S34 Unsteadiness in either mode
- S35 Level, drum level in Drum and voice level in Voice
- Knobs that change job with the mode pick up. After a switch, a knob holds that mode's last setting until it is turned back through it
- Drum loop. Hold P10, play, let go and it loops from the first hit. P11 clears. Keeps going in Voice
- Drum pads on the front row and P09, retuned under 70 Hz, where the body still sounds like skin and wood
- Audio block of 96 at 480 MHz, so the voice's wavetable rebuild fits inside the callback
- Engine folder renamed from tomb to mound
- Board picture in the README, from TouchString

## v0.1, 18 September 2026

`firmware/glor-na-si-v0.1.bin`

- Fork of Synthux Academy's TouchString with the string engine replaced
- Six drum pads on a modal body, tuned around the chamber's 110 Hz
- Walk on the right fader. Entrance to chamber, the tail lengthens and darkens and the room rises over the hit
- S32 drum tone and S33 drum decay, landing on the next strike
- LED flash on every strike
