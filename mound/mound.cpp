#include "mound.h"

using namespace synthux;
using namespace daisysp;

Mound::Mound():
  _clock { 0 },
  _seconds_per_sample { 1.f / 48000.f } {
  _voice_at.fill(0.f);
  _vowel.fill(0.f);
  _semitones.fill(0.f);
};

void Mound::Init(const float sample_rate, const float buffer_size) {
  _seconds_per_sample = 1.f / sample_rate;
  _skin.Init(sample_rate);
  _stone.Init(sample_rate);
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
  _strikes.Push({ index, metres });
};

void Mound::Sing(const uint8_t voice, const float vowel, const float semitones, const float metres) {
  if (voice >= kVoiceCount) return;
  _sings.Push({ voice, true, vowel, semitones, metres });
};

void Mound::Rest(const uint8_t voice) {
  if (voice >= kVoiceCount) return;
  _sings.Push({ voice, false, 0.f, 0.f, 0.f });
};

void Mound::SetSing(const uint8_t voice, const bool on) {
  if (on) Sing(voice, _passage.Listener());
  else Rest(voice);
};

// Audio side, at the start of a block: every queued strike and
// syllable, each pinned, struck or sung, and knocking the singers, as
// one thing.
void Mound::_drain() {
  StrikeCmd s;
  while (_strikes.Pop(s)) {
    if (s.index < kSkinPadCount) {
      _passage.PinNow(0, s.metres, _skin.Active());
      _skin.Hit(s.index);
    } else {
      _passage.PinNow(1, s.metres, _stone.Active());
      _stone.Hit(s.index - kSkinPadCount);
    }
    _knock(s.metres);
  }
  SingCmd c;
  while (_sings.Pop(c)) {
    if (!c.on) {
      _chants[c.voice].Rest();
      continue;
    }
    _voice_at[c.voice] = c.metres;
    _passage.PinNow(2 + c.voice, c.metres, _chants[c.voice].IsSounding());
    _chants[c.voice].Sing(c.vowel, c.semitones);
  }
};

// The hit knocks every singer, less the further off it was.
void Mound::_knock(const float metres) {
  for (uint8_t v = 0; v < kVoiceCount; v++) {
    if (!_chants[v].IsSounding()) continue;
    auto d = fabsf(metres - _voice_at[v]);
    _chants[v].Knock(kKnockMax * expf(-d / kKnockMetres));
  }
};

void Mound::SetDrumHardness(const float value) {
  _skin.SetHardness(value);
  _stone.SetHardness(value);
};

void Mound::SetDrumLevel(const float value) {
  _skin.SetLevel(value);
  _stone.SetLevel(value);
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
// skin, the stone and the voices go into the passage as separate
// sources, each from wherever it was pinned.
void Mound::Process(float **out, size_t size) {
  _drain();
  std::array<bool, kVoiceCount> sing;
  for (uint8_t v = 0; v < kVoiceCount; v++) sing[v] = _chants[v].IsSounding();
  constexpr uint16_t slot = CHANT_BLOCK / kVoiceCount;
  std::array<float, Passage::kSourceCount> in;
  _passage.Update();
  auto seconds = size * _seconds_per_sample;
  _skin.Settle(seconds);
  for (auto& chant : _chants) chant.Settle(seconds);
  for (size_t i = 0; i < size; i++) {
    in[0] = _skin.Process();
    in[1] = _stone.Process();
    auto tick = _clock;
    if (++_clock >= CHANT_BLOCK) _clock = 0;
    for (uint8_t v = 0; v < kVoiceCount; v++) {
      in[2 + v] = sing[v] ? _chants[v].Process(tick == v * slot) : 0.f;
    }
    float left, right;
    _passage.Process(in, left, right);
    out[0][i] = SoftClip(left) * kMasterGain;
    out[1][i] = SoftClip(right) * kMasterGain;
  }
};
