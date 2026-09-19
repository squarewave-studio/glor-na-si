#include "mound.h"

using namespace synthux;
using namespace daisysp;

Mound::Mound():
  _clock { 0 } {
  _bus.fill(0.f);
  _reverb_out.fill(0.f);
  _mix.fill(0.f);
};

void Mound::Init(const float sample_rate, const float buffer_size) {
  _drum.Init(sample_rate);
  for (uint8_t i = 0; i < kVoiceCount; i++) {
    _chants[i].Init(sample_rate, 11 + 2 * i);
  }
  _reverb.Init(sample_rate);
  SetWalk(0.f);
};

void Mound::Strike(const uint8_t index) {
  if (index >= kDrumPadCount) return;
  _drum.Strike(kDrumPitches[index]);
};

void Mound::SetWalk(const float value) {
  auto walk = fclamp(value, 0.f, 1.f);
  auto back = 1.f - walk;
  _xfade.SetStage(fmap(walk, kWalkMixMin, kWalkMixMax));
  _reverb.SetFeedback(fmap(walk, kWalkFeedbackMin, kWalkFeedbackMax));
  _reverb.SetLpFreq(fmap(back, kWalkLpChamber, kWalkLpEntrance, Mapping::LOG));
  _drum.SetBrightness(fmap(back, kWalkBrightChamber, kWalkBrightEntrance));
  _drum.SetAccent(fmap(back, kWalkAccentChamber, kWalkAccentEntrance));
};

void Mound::SetVoiceLevel(const float value) {
  for (auto& chant : _chants) chant.SetLevel(value);
};

void Mound::SetPitch(const float value) {
  for (auto& chant : _chants) chant.SetPitch(value);
};

void Mound::SetSpread(const float value) {
  for (auto& chant : _chants) chant.SetSpread(value);
};

void Mound::SetSwell(const float value) {
  for (auto& chant : _chants) chant.SetSwell(value);
};

void Mound::SetUnsteadiness(const float value) {
  for (auto& chant : _chants) chant.SetUnsteadiness(value);
};

// One sample clock for every voice. Voice n rebuilds its table when the
// clock passes n slots into the block, so the rebuilds take turns.
void Mound::Process(float **out, size_t size) {
  std::array<bool, kVoiceCount> sing;
  for (uint8_t v = 0; v < kVoiceCount; v++) sing[v] = _chants[v].IsSounding();
  constexpr uint16_t slot = CHANT_BLOCK / kVoiceCount;
  for (size_t i = 0; i < size; i++) {
    auto dry = _drum.Process();
    auto tick = _clock;
    if (++_clock >= CHANT_BLOCK) _clock = 0;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
      if (sing[v]) dry += _chants[v].Process(tick == v * slot);
    }
    _bus[0] = _bus[1] = dry;
    _reverb.Process(_bus[0], _bus[1], &(_reverb_out[0]), &(_reverb_out[1]));
    _xfade.Process(_bus[0], _bus[1], _reverb_out[0], _reverb_out[1], _mix[0], _mix[1]);
    out[0][i] = SoftClip(_mix[0]) * kMasterGain;
    out[1][i] = SoftClip(_mix[1]) * kMasterGain;
  }
};
