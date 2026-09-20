#include "mound.h"

using namespace synthux;
using namespace daisysp;

Mound::Mound():
  _clock { 0 },
  _seconds_per_sample { 1.f / 48000.f } {
  _voice_at.fill(0.f);
};

void Mound::Init(const float sample_rate, const float buffer_size) {
  _seconds_per_sample = 1.f / sample_rate;
  _drum.Init(sample_rate);
  for (uint8_t i = 0; i < kVoiceCount; i++) {
    _chants[i].Init(sample_rate, 11 + 2 * i);
  }
  _passage.Init(sample_rate);
};

void Mound::Strike(const uint8_t index) {
  Strike(index, _passage.Listener());
};

void Mound::Strike(const uint8_t index, const float metres) {
  if (index >= kDrumPadCount) return;
  _passage.Pin(0, metres, _drum.Active());
  _drum.Strike(kDrumPitches[index]);
  // The hit knocks every singer, less the further off it was.
  for (uint8_t v = 0; v < kVoiceCount; v++) {
    if (!_chants[v].IsSounding()) continue;
    auto d = fabsf(metres - _voice_at[v]);
    _chants[v].Knock(kKnockMax * expf(-d / kKnockMetres));
  }
};

void Mound::SetSing(const uint8_t voice, const bool on) {
  if (on) Sing(voice, _passage.Listener());
  else _chants[voice].SetGate(false);
};

void Mound::Sing(const uint8_t voice, const float metres) {
  _voice_at[voice] = metres;
  _passage.Pin(1 + voice, metres, _chants[voice].IsSounding());
  _chants[voice].SetGate(true);
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
// clock passes n slots into the block, so the rebuilds take turns. The
// drum and the voices go into the passage as separate sources, each
// from wherever it was pinned.
void Mound::Process(float **out, size_t size) {
  std::array<bool, kVoiceCount> sing;
  for (uint8_t v = 0; v < kVoiceCount; v++) sing[v] = _chants[v].IsSounding();
  constexpr uint16_t slot = CHANT_BLOCK / kVoiceCount;
  std::array<float, Passage::kSourceCount> in;
  _passage.Update();
  auto seconds = size * _seconds_per_sample;
  _drum.Settle(seconds);
  for (auto& chant : _chants) chant.Settle(seconds);
  for (size_t i = 0; i < size; i++) {
    in[0] = _drum.Process();
    auto tick = _clock;
    if (++_clock >= CHANT_BLOCK) _clock = 0;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
      in[1 + v] = sing[v] ? _chants[v].Process(tick == v * slot) : 0.f;
    }
    float left, right;
    _passage.Process(in, left, right);
    out[0][i] = SoftClip(left) * kMasterGain;
    out[1][i] = SoftClip(right) * kMasterGain;
  }
};
