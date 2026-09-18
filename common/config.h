#pragma once

#include <array>
#include <stdint.h>

// Drum ...................................................................
static constexpr uint8_t kDrumPadCount = 6;
static constexpr std::array<uint8_t, kDrumPadCount> kDrumPads    = { 0, 3, 4, 8, 1, 5 };
static constexpr std::array<float,   kDrumPadCount> kDrumPitches = { 55.f, 36.67f, 41.25f, 68.75f, 82.5f, 110.f };

static constexpr float kDrumGain          = 0.8f;
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

// Output .................................................................
static constexpr float kMasterGain = 0.75f;
static constexpr uint8_t kLedStrikeTicks = 15;
