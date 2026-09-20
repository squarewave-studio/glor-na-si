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
// listener walks the fader. Every path is a delay by distance with a
// loss and a dulling per metre.
static constexpr float kSpeedOfSound      = 343.f;  //m/s
static constexpr float kOutsideMetres     = 3.f;    //fader bottom stands this far outside
static constexpr float kPassageMetres     = 19.f;   //entrance stone to the chamber
static constexpr float kChamberMetres     = 6.f;    //chamber depth on the axis, fader top is the back
static constexpr float kChamberAt         = 22.f;   //where the room lives
static constexpr float kMouthAt           = 0.f;    //where the passage opens
static constexpr float kWalkSpeed         = 5.f;    //m/s, the fader is where you are going, this is how fast

static constexpr float kDuctLossDb        = 0.5f;   //dB per metre, every path
static constexpr float kSpreadMetres      = 6.f;    //direct sound falls 1/r this far, then the passage guides it
static constexpr float kCutoffNear        = 12000.f;//Hz at no distance
static constexpr float kCutoffHalfMetres  = 6.f;    //the cutoff halves every this many metres
static constexpr float kOutsideLossDb     = 8.f;    //extra loss at the fader bottom
static constexpr float kOutsideCutoff     = 0.3f;   //cutoff scale at the fader bottom

static constexpr float kChamberSend       = 1.f;
static constexpr float kChamberReturn     = 2.f;
static constexpr float kChamberFeedback   = 0.9f;
static constexpr float kChamberLp         = 4000.f; //Hz

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

static constexpr float kMouthReflect      = -0.5f;  //the open end sends a dull inverted slap back up
static constexpr float kMouthCutoff       = 800.f;  //Hz

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
// Two voices. Each pad is a vowel (0 mmm, 1 ooh, 2 oh, 3 aah, 4 ah,
// 5 overtone) at an interval in semitones above the pitch knob.
static constexpr uint8_t kVoiceCount = 2;

struct VoicePad {
  uint8_t pad;
  uint8_t vowel;
  float   semitones;
};

static constexpr uint8_t kVoicePadCount = 10;
static constexpr std::array<VoicePad, kVoicePadCount> kVoicePads = {{
  { 3, 0, 0.f }, { 4, 1, 0.f }, { 5, 2, 0.f }, { 6, 3, 0.f }, { 7, 4, 0.f }, // front row at the root
  { 8, 1, 7.f }, { 9, 3, 7.f },                                               // ooh and aah a fifth up
  { 0, 1, 12.f }, { 1, 3, 12.f }, { 2, 5, 7.f },                              // top row an octave up, overtone at the fifth
}};

static constexpr float kVoicePitch        = 65.9f; //Hz, C2 as the chant was measured, S31 at centre
static constexpr float kVoicePitchOctaves = 1.f;   //S31 range either side of centre
static constexpr float kVoiceSpreadMax    = 40.f;  //cents between the two singers, S32 fully clockwise
static constexpr float kVoiceUnsteadyMax  = 2.f;   //times the engine's own wander and drift, S34 fully clockwise
static constexpr float kVoiceSwellMin     = 0.15f; //s, on_time at pot minimum
static constexpr float kVoiceSwellMax     = 3.f;   //s, on_time at pot maximum
static constexpr float kVoiceReleaseRatio = 1.3f;  //off_time over on_time
static constexpr float kVoiceGain         = 0.04f; //raw pair peaks near 12, this lands it just under the drum
static constexpr float kVoiceSilence      = 1e-4f; //env floor, below it the pair stops

// A drum hit knocks the singers near it: their wander, drift and rasp
// jump and settle back. Scaled by S34, so a still singer is unshakeable.
static constexpr float kKnockMax     = 1.5f; //times the unsteadiness, point blank
static constexpr float kKnockMetres  = 4.f;  //falls off by e every this far
static constexpr float kKnockSeconds = 0.5f; //time constant of the settling

// Output .................................................................
static constexpr float kMasterGain = 0.75f;
static constexpr float kLevelMax   = 2.f; //S35 fully clockwise, times the gain above
static constexpr uint8_t kLedStrikeTicks = 15;
