#pragma once

#include <array>
#include <stddef.h>
#include <stdint.h>

// Audio ..................................................................
// The drum chain alone is 48% of the core at 480 MHz and the chant table
// rebuild lands on top of it every 480 samples, so the callback needs
// room: 48 overran, 96 at 480 MHz peaks around 80% with one voice.
static constexpr size_t kAudioBlockSize = 96;

// Skin ...................................................................
// A drum skin: eight modes of a circular membrane at the Bessel ratios.
// A strike rings each mode as much as the mode's shape says at the
// strike's radius, so the centre rings the round modes, deep and pure,
// and the rim rings the rest, thin and bright. Pads are places on the
// skin, not notes. Nothing rings at the rim itself, so radii stay under
// 0.9.
struct SkinPad {
  uint8_t pad;
  float   radius;   //0 centre, 1 rim
  float   hard;     //added to the hardness, the stick near the rim
  float   gain;     //the skin barely moves near the rim, so those hits are lifted, all modes alike
};
static constexpr uint8_t kSkinPadCount = 7;
static constexpr std::array<SkinPad, kSkinPadCount> kSkinPads = {{
  { 3, 0.9f, 0.35f, 2.6f }, { 4, 0.65f, 0.1f, 1.3f }, { 5, 0.f, 0.f, 1.f }, { 6, 0.45f, 0.f, 1.1f }, { 7, 0.8f, 0.25f, 2.f }, // front row, a line across the skin
  { 8, 0.92f, 0.3f, 3.f }, { 9, 0.85f, 0.6f, 2.4f },                                                                // below, the stick near the rim
}};

struct SkinMode {
  uint8_t m;        //nodal diameters
  float   zero;     //Bessel zero, the mode's frequency over the first's is zero / 2.4048
  float   gain;     //how well it radiates
};
static constexpr uint8_t kSkinModeCount = 12;
static constexpr std::array<SkinMode, kSkinModeCount> kSkinModes = {{
  { 0, 2.4048f, 1.f  }, { 1, 3.8317f, 0.9f }, { 2, 5.1356f, 0.7f }, { 0, 5.5201f, 0.6f },
  { 3, 6.3802f, 0.6f }, { 1, 7.0156f, 0.5f }, { 4, 7.5883f, 0.45f }, { 2, 8.4172f, 0.4f },
  { 0, 8.6537f, 0.35f }, { 5, 8.7715f, 0.35f }, { 3, 9.7610f, 0.3f }, { 1, 10.1735f, 0.3f },
}};

static constexpr float kSkinFundamental  = 55.f;   //Hz, S31 in Drum at centre
static constexpr float kSkinSizeOctaves  = 1.f;    //S31, half to double the drum
static constexpr float kSkinStiffnessMax = 0.02f;  //S32, stretches the upper modes as a hide does
static constexpr float kSkinDecayMin     = 1.2f;   //s, T60 of the fundamental, S33 minimum
static constexpr float kSkinDecayMax     = 4.f;    //s, S33 maximum
static constexpr float kSkinDecaySlope   = 0.8f;   //higher modes die faster, T60 over ratio to this
static constexpr float kSkinDropAmount   = 0.07f;  //sharp at a hard hit, settling
static constexpr float kSkinDropSeconds  = 0.12f;
// The strike is a mallet: a half-sine pulse of contact time, the same
// push however hard, so harder means shorter and brighter, not louder,
// apart from a little accent. A bigger skin gives longer. The head has
// a size too, a hand at soft to a stick tip at hard, which rounds off
// the upper modes. The thwack stands in for the hundreds of modes above
// the eight: a dull burst of noise between a few hundred hertz and a
// kilohertz that dies in about 20 ms, heard straight. Not a click. The
// tick, the pulse itself above 600 Hz, is off: it clicked. No two hits
// are quite the same. S34 in Drum.
static constexpr float kSkinContactSoft  = 0.008f; //s, at S34 minimum, at the size as built
static constexpr float kSkinContactHard  = 0.0008f;
static constexpr float kSkinHeadSoft     = 0.2f;   //head radius over skin radius, a hand
static constexpr float kSkinHeadHard     = 0.05f;  //a stick tip
static constexpr float kSkinAccent       = 0.3f;   //level is 1 - accent + accent x hardness
static constexpr float kSkinSlap         = 0.08f;  //the thwack, at full hardness; a third of it at soft, and more out at the rim
static constexpr float kSkinSlapSeconds  = 0.006f; //its time constant
static constexpr float kSkinSlapLow      = 300.f;  //Hz, its band
static constexpr float kSkinSlapHigh     = 1500.f;
static constexpr float kSkinVary         = 0.08f;  //random spread of level and contact time, per hit
static constexpr float kSkinTick         = 0.f;    //the pulse above 600 Hz, heard straight: off, it clicks
static constexpr float kSkinGain         = 0.3f;   //S35 at centre

// Stone ..................................................................
// A stone struck with the same stick: the strike into a few short
// bright modes and no skin at all. Three sizes on the top row in Drum.
struct StonePad {
  uint8_t pad;
  float   hz;
};
static constexpr uint8_t kStonePadCount = 3;
static constexpr std::array<StonePad, kStonePadCount> kStonePads = {{ { 0, 1900.f }, { 1, 2700.f }, { 2, 3800.f } }};
static constexpr std::array<float, 3> kStoneRatios = { 1.f, 1.63f, 2.51f };
static constexpr float kStoneDecay   = 0.02f;   //s, T60, short: stone on stone is mostly the strike
static constexpr float kStoneContact = 0.0002f; //s, at S34 centre; harder is shorter
static constexpr float kStoneSlap    = 0.5f;    //the strike itself, heard straight: stone on stone is a click
static constexpr float kStoneSlapSeconds = 0.0006f;
static constexpr float kStoneSlapLow = 1000.f;  //Hz
static constexpr float kStoneSlapHigh = 8000.f;
static constexpr float kStoneGain    = 0.15f;   //S35 at centre

// A strike index is a skin pad, then a stone pad.
static constexpr uint8_t kDrumPadCount = kSkinPadCount + kStonePadCount;

// Passage ................................................................
// The mound as a line, in metres along the axis of the passage. The
// entrance stone is 0. Outside is negative, the passage runs to
// kPassageMetres, the chamber beyond it. O'Kelly's plan for the lengths.
// Sounds are pinned where the listener stood when they started, and the
// listener walks the fader.
static constexpr float kSpeedOfSound      = 343.f;  //m/s
static constexpr float kOutsideMetres     = 3.f;    //fader bottom stands this far outside
static constexpr float kPassageMetres     = 19.f;   //entrance stone to the chamber
static constexpr float kChamberMetres     = 6.f;    //chamber depth on the axis, fader top is the back
static constexpr float kChamberAt         = 22.f;   //where the room lives
static constexpr float kMouthAt           = 0.f;    //where the passage opens
static constexpr float kWalkSpeed         = 5.f;    //m/s, the fader is where you are going, this is how fast

// The passage and chamber as one tube, a section per metre, sound going
// both ways along it. Each section is the width and height of the real
// one: the widths read off O'Kelly's plan (fig. 4, the gap between the
// orthostat rows), the heights off his east-side elevation, floor to
// roof. Where the section changes, some of the wave turns back, as it
// does in a tube, so the passage has its own reflections and standing
// waves. The last six sections are the chamber: the vault at 6 m, the
// end recess lower and narrower.
struct PassageSection {
  float width;  //m
  float height; //m
  constexpr float Area() const { return width * height; }
};
static constexpr uint8_t kSectionCount = 25;
static constexpr std::array<PassageSection, kSectionCount> kSections = {{
  { 1.15f, 1.55f }, { 1.20f, 1.55f }, { 1.25f, 1.55f }, { 1.45f, 1.50f }, { 1.50f, 1.50f }, //  0 to  5 m
  { 1.50f, 1.50f }, { 1.45f, 1.60f }, { 1.50f, 1.60f }, { 1.35f, 1.60f }, { 1.20f, 1.65f }, //  5 to 10 m
  { 1.10f, 1.80f }, { 1.00f, 1.90f }, { 1.10f, 1.95f }, { 1.35f, 2.00f }, { 1.45f, 2.20f }, // 10 to 15 m
  { 1.25f, 2.30f }, { 1.25f, 2.60f }, { 1.00f, 2.80f }, { 1.15f, 3.00f }, { 1.40f, 3.30f }, // 15 to 19 m, the roof climbing
  { 2.00f, 4.50f }, { 3.00f, 6.00f }, { 3.00f, 6.00f }, { 3.00f, 6.00f }, { 2.00f, 3.50f }, // the chamber, then the end recess
}};

// Losses along the tube. Stone hardly takes the low end, so a wave
// carries the length of the passage and back, but the top goes: one
// pole per metre. Near a source the sound still spreads as 1/r until
// the passage takes it, out to kSpreadMetres, so standing on top of a
// sound is dry and close.
static constexpr float kDuctLossDb        = 0.1f;   //dB per metre, broadband, the gaps between the orthostats leaking into the cairn
static constexpr float kDuctCutoff        = 10000.f;//Hz, the pole per metre, about 1.9 kHz by the chamber
static constexpr float kSpreadMetres      = 6.f;    //direct sound falls 1/r this far, then the passage guides it

// The ends. The mouth is open: what arrives reflects inverted, whole at
// the bottom and less and less above the frequency where the opening is
// about a wavelength across, and what does not reflect is what you hear
// outside. The back of the end recess is stone, most of it comes back.
// Where the passage opens into the chamber the plane wave is only a
// plane wave in the low end, so the reflection off that opening is
// low-passed and the room takes the rest.
static constexpr float kMouthReflect      = 0.95f;
static constexpr float kMouthCutoff       = 120.f;  //Hz, the opening is about a wavelength across here, the gate and K1 in front of it
static constexpr float kBackReflect       = 0.9f;
static constexpr float kBackCutoff        = 3000.f; //Hz
static constexpr float kChamberOpenCutoff = 400.f;  //Hz
static constexpr float kOutsideCouple     = 0.5f;   //how much of a sound made outside gets into the mouth
static constexpr float kOutsideLossDb     = 8.f;    //extra loss at the fader bottom
static constexpr float kOutsideCutoff     = 0.3f;   //cutoff scale at the fader bottom

// The room. Every source feeds it at the level the tube carries, later
// by its distance to the chamber, and what comes back goes into the
// tube at kChamberAt like any other sound, so the room's answer meets
// the mouth and the passage the way a voice does. The reverb's width,
// what differs between its two sides, is heard on a line of its own,
// fading with distance from the chamber.
static constexpr float kChamberSend       = 6.f;
static constexpr float kChamberReturn     = 0.8f;
static constexpr float kChamberFeedback   = 0.9f;
static constexpr float kChamberLp         = 4000.f; //Hz
static constexpr float kSendMetres        = 14.f;   //the send is late by its distance, up to this
static constexpr float kSideMetres        = 12.f;   //the width falls off over this many metres from the chamber
static constexpr float kCutoffNear        = 12000.f;//Hz, the width with no distance
static constexpr float kCutoffHalfMetres  = 6.f;    //its cutoff halves every this many metres

// The ring. Jahn, Devereux and Ibison measured the main resonance near
// 110 Hz; the second is a guess at the east recess from the plan. The
// gain is what a tone held on the mode comes back at, so 3 is 9.5 dB up.
// The decay sets the width: 0.8 s is about 3 Hz wide.
struct ChamberMode {
  float hz;
  float t60;
  float gain;
};
static constexpr std::array<ChamberMode, 2> kChamberModes = {{
  { 110.f, 0.8f, 3.f },
  {  86.f, 0.6f, 1.5f },
}};

// The passage colours what comes through it by its height, which rises
// from the entrance to the chamber (fig. 17). One peak that slides with
// the listener, fading out beyond either end of the passage.
static constexpr float kHeightEntrance    = 1.5f;   //m
static constexpr float kHeightChamber     = 3.4f;   //m
static constexpr float kDuctColourDb      = 5.f;
static constexpr float kDuctColourQ       = 4.f;
static constexpr float kDuctColourFade    = 2.f;    //m past either end

// And by its width: sound bounces between the walls a metre apart, a
// 2 m round trip fed back, so everything that comes through the
// passage rings faintly at 171 Hz and its multiples. Same window.
static constexpr float kWidthMetres       = 1.f;
static constexpr float kWidthGain         = 0.4f;
static constexpr float kWidthCutoff       = 1000.f; //Hz, on the reflection

static constexpr float kPinFadeSeconds    = 0.01f;  //a re-pinned voice crosses over this fast
static constexpr float kDrumPinFadeSeconds = 0.002f; //a re-pinned drum, short so the strike keeps its attack

// Loop ...................................................................
// Tap P10, play, tap again and it loops, the tap is the bar. A take with
// nothing played for kLoopIdleTicks (about 4 ms each) ends by itself,
// trimmed to the last thing played. A bar is never shorter than
// kLoopMinTicks.
static constexpr uint8_t  kLoopRecordPad = 10;
static constexpr uint8_t  kLoopClearPad  = 11;
static constexpr uint8_t  kLoopSteps     = 64;
static constexpr uint16_t kLoopIdleTicks = 5000; //about twenty seconds
static constexpr uint16_t kLoopMinTicks  = 25;   //about 100 ms
// S30 runs the loop faster or slower, as played at centre.
static constexpr float    kLoopSpeedOctaves = 2.f; //a quarter to four times

// Voice ..................................................................
// Two voices. Each is two singers a few cents apart. The front row is
// five sounds at the root. The top row is the last sound you sang, a
// fourth, a fifth and an octave up. The bottom two are the last sound
// with a technique: under, the voice breaking into an undertone an
// octave below, and over, the overtone climbing the harmonics.
static constexpr uint8_t kVoiceCount = 2;

// What a sound is: five formants and their bandwidths in Hz, how nasal
// it is, whether it opens with breath before the voice, how breathy it
// is throughout, and its gain, set so the five come out alike.
struct VoiceSound {
  float f[5];
  float bw[5];
  float nasal;
  bool  breath;
  float breathy;
  float gain;
};

static constexpr uint8_t kSoundCount = 5;
static constexpr std::array<VoiceSound, kSoundCount> kSounds = {{
  { { 250, 1000, 2200, 3000, 3600 }, { 120, 200, 250, 300, 350 }, 1.f, true,  0.f, 2.5f }, // hmm, closed lips lose most of it
  { { 300,  700, 2250, 3000, 3600 }, {  80, 100, 140, 180, 220 }, 0.f, false, 0.f, 1.f }, // ooh
  { { 680, 1050, 2450, 3200, 3700 }, {  80,  90, 120, 150, 200 }, 0.f, false, 0.f, 1.f }, // aah
  { { 430,  780, 2350, 3050, 3600 }, {  80,  90, 130, 170, 220 }, 0.f, true,  0.f, 1.f }, // hoo, a rounded oh with a soft h
  { { 580,  880, 2450, 3100, 3600 }, {  80,  90, 120, 150, 200 }, 0.f, false, 0.f, 1.f }, // aww
}};

// The overtone: the bright formant sits on this harmonic of the note
// for as long as the pad is held.
static constexpr float kOvertoneHarmonic = 9.f;
static constexpr float kOvertoneWidth = 40.f; //Hz, the formant's bandwidth while it picks the overtone
static constexpr float kOvertoneGain  = 0.55f; //the picked overtone would otherwise be the loudest thing
static constexpr float kUndertoneGain = 1.35f; //every second pulse is quiet, this makes up for it
enum { SOUND_HMM, SOUND_OOH, SOUND_AAH, SOUND_HOO, SOUND_AWW };
static constexpr uint8_t kLastSound = 255;   // whatever the front row sang last, aah to begin with

enum VoiceTechnique : uint8_t { VOICE_PLAIN, VOICE_UNDER, VOICE_OVER };

struct VoicePad {
  uint8_t pad;
  uint8_t sound;
  float   semitones;
  uint8_t technique;
};

static constexpr uint8_t kVoicePadCount = 10;
static constexpr std::array<VoicePad, kVoicePadCount> kVoicePads = {{
  { 3, SOUND_HMM, 0.f, VOICE_PLAIN }, { 4, SOUND_OOH, 0.f, VOICE_PLAIN }, { 5, SOUND_AAH, 0.f, VOICE_PLAIN },
  { 6, SOUND_HOO, 0.f, VOICE_PLAIN }, { 7, SOUND_AWW, 0.f, VOICE_PLAIN },                          // the front row, the sounds at the root
  { 8, kLastSound, 0.f, VOICE_UNDER }, { 9, kLastSound, 0.f, VOICE_OVER },                        // under and over
  { 0, kLastSound, 5.f, VOICE_PLAIN }, { 1, kLastSound, 7.f, VOICE_PLAIN }, { 2, kLastSound, 12.f, VOICE_PLAIN }, // a fourth, a fifth, an octave up
}};

static constexpr float kVoicePitch         = 65.41f; //Hz, C2, S31 at centre
static constexpr float kVoicePitchOctaves  = 1.f;    //S31 range either side of centre
static constexpr float kVoiceSpreadMax     = 40.f;   //cents between the two singers, S32 fully clockwise
static constexpr float kVoiceSwellMin      = 0.04f;  //s, the attack at S33 minimum
static constexpr float kVoiceSwellMax      = 2.f;    //s, at maximum, log between
static constexpr float kVoiceReleaseRatio  = 1.6f;   //release over attack
static constexpr float kVoiceVibratoHz     = 5.4f;   //the first singer
static constexpr float kVoiceVibratoSpread = 0.5f;   //Hz, the second singer sits this much faster
static constexpr float kVoiceGain          = 0.012f; //S35 at centre; the raw pair peaks near 9 and a held note in the chamber comes back six times over, this keeps that under the clip
static constexpr float kVoiceSilence       = 1e-4f;  //intensity floor, below it the voice stops

// A drum hit knocks the singers near it: their wander, drift and rasp
// jump and settle back. Scaled by S34, so a still singer is unshakeable.
static constexpr float kKnockMax     = 1.5f; //times the unsteadiness, point blank
static constexpr float kKnockMetres  = 4.f;  //falls off by e every this far
static constexpr float kKnockSeconds = 0.5f; //time constant of the settling

// Output .................................................................
static constexpr float kMasterGain = 0.75f;
static constexpr float kLevelMax   = 2.f; //S35 fully clockwise, times the gain above
static constexpr uint8_t kLedStrikeTicks = 15;
