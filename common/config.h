#pragma once

#include <array>
#include <stddef.h>
#include <stdint.h>

// Audio ..................................................................
// The drum chain alone is 48% of the core at 480 MHz and the chant table
// rebuild lands on top of it every 480 samples, so the callback needs
// room: 48 overran, 96 at 480 MHz peaks around 80% with one voice.
static constexpr size_t kAudioBlockSize = 96;

// Drum ...................................................................
static constexpr uint8_t kDrumPadCount = 6;
// Front row left to right, rising, then P09 below for the top note. All
// under the chamber's 110 Hz: above about 70 Hz this body turns bell-like.
static constexpr std::array<uint8_t, kDrumPadCount> kDrumPads    = { 3, 4, 5, 6, 7, 9 };
static constexpr std::array<float,   kDrumPadCount> kDrumPitches = { 36.67f, 41.25f, 45.83f, 55.f, 61.88f, 68.75f };

static constexpr float kDrumGain          = 0.5f;
static constexpr float kDrumStructureMin  = 0.05f;
static constexpr float kDrumStructureMax  = 0.6f;
static constexpr float kDrumDampingMin    = 0.25f;
static constexpr float kDrumDampingMax    = 0.8f;
// S34 in Drum is the hardness, soft mallet to hard stick: the exciter's
// brightness and accent together, centre is as it was. S31 in Drum tunes
// the pads an octave either way.
static constexpr float kDrumBrightnessMin = 0.1f;
static constexpr float kDrumBrightnessMax = 0.6f;
static constexpr float kDrumAccentMin     = 0.6f;
static constexpr float kDrumAccentMax     = 1.f;
static constexpr float kDrumTuneOctaves   = 1.f;
// A skin starts sharp and settles as the tension gives: this much sharp
// at a hard hit, settling with this time constant.
static constexpr float kDrumDropAmount    = 0.12f;
static constexpr float kDrumDropSeconds   = 0.06f;
// The body costs the same silent as struck, so it stops once its output
// has sat under this for a while and runs again on the next strike.
static constexpr float kDrumSilence       = 1e-5f;
static constexpr float kDrumSilentSeconds = 0.05f;

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
static constexpr float kChamberReturn     = 1.5f;
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
