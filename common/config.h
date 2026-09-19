#pragma once

#include <array>
#include <stddef.h>
#include <stdint.h>

// Audio ..................................................................
// Must divide the chant engine's 480-sample block. The drum chain alone
// is 58% of a 400 MHz core and the chant table rebuild lands on top of it
// every 480 samples, so the callback needs room: 48 overran, 96 at 480 MHz
// peaks around 80%.
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
static constexpr float kDrumBrightnessCap = 0.7f;

// Walk ...................................................................
static constexpr float kWalkMixMin         = 0.15f;
static constexpr float kWalkMixMax         = 0.65f;
static constexpr float kWalkFeedbackMin    = 0.65f;
static constexpr float kWalkFeedbackMax    = 0.9f;
static constexpr float kWalkLpEntrance     = 9000.f; //Hz
static constexpr float kWalkLpChamber      = 1800.f; //Hz
static constexpr float kWalkBrightEntrance = 0.35f;
static constexpr float kWalkBrightChamber  = 0.15f;
static constexpr float kWalkAccentEntrance = 0.85f;
static constexpr float kWalkAccentChamber  = 0.7f;

// Loop ...................................................................
// Hold P10 for the length of the bar and play, let go and it loops.
static constexpr uint8_t kLoopRecordPad = 10;
static constexpr uint8_t kLoopClearPad  = 11;
static constexpr uint8_t kLoopSteps     = 64;

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
static constexpr float kVoiceSwellMin     = 0.4f;  //s, on_time at pot minimum
static constexpr float kVoiceSwellMax     = 3.f;   //s, on_time at pot maximum
static constexpr float kVoiceReleaseRatio = 1.3f;  //off_time over on_time
static constexpr float kVoiceGain         = 0.04f; //raw pair peaks near 12, this lands it just under the drum
static constexpr float kVoiceSilence      = 1e-4f; //env floor, below it the pair stops

// Output .................................................................
static constexpr float kMasterGain = 0.75f;
static constexpr float kLevelMax   = 2.f; //S35 fully clockwise, times the gain above
static constexpr uint8_t kLedStrikeTicks = 15;
