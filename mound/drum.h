#pragma once

#include <cmath>
#include "daisysp.h"
#include "nocopy.h"
#include "config.h"

// Wraps a single daisysp::ModalVoice (Plaits-style struck resonator).
// Monophonic: every Strike retriggers the same body. Timbre settings are
// held and only reach the voice inside Strike. The body costs the same
// silent as struck, so it stops once its output has sat under the floor
// for a while and runs again on the next strike.

namespace synthux {

class Drum {
public:
  Drum():
  _structure  { .5f },
  _damping    { .5f },
  _brightness { .35f },
  _accent     { .8f },
  _octaves    { 0.f },
  _freq       { 55.f },
  _drop       { 0.f },
  _gain       { kDrumGain },
  _quiet      { 0 },
  _silent_after { 0 },
  _active     { false }
  {}
  ~Drum() {}

  void Init(float sample_rate) {
    _voice.Init(sample_rate);
    _voice.SetSustain(false);
    _silent_after = (uint32_t)(kDrumSilentSeconds * sample_rate);
  }

  void SetStructure(const float value) {
    _structure = daisysp::fmap(value, kDrumStructureMin, kDrumStructureMax);
  }

  void SetDamping(const float value) {
    _damping = daisysp::fmap(value, kDrumDampingMin, kDrumDampingMax);
  }

  // Soft mallet to hard stick, centre as it was.
  void SetHardness(const float value) {
    _brightness = daisysp::fmap(value, kDrumBrightnessMin, kDrumBrightnessMax);
    _accent     = daisysp::fmap(value, kDrumAccentMin, kDrumAccentMax);
  }

  // Pot centre is the pads as tuned, an octave either way.
  void SetTune(const float value) {
    _octaves = daisysp::fmap(value, -kDrumTuneOctaves, kDrumTuneOctaves);
  }

  // Live. Pot centre is kDrumGain.
  void SetLevel(const float value) {
    _gain = kDrumGain * daisysp::fmap(value, 0.f, kLevelMax);
  }

  void Strike(const float freq) {
    _voice.SetStructure(_structure);
    _voice.SetDamping(_damping);
    _voice.SetBrightness(_brightness);
    _voice.SetAccent(_accent);
    _freq = freq * exp2f(_octaves);
    _drop = kDrumDropAmount * _accent;
    _voice.SetFreq(_freq * (1.f + _drop));
    _voice.Trig();
    _quiet = 0;
    _active = true;
  }

  bool Active() const { return _active; }

  // Once a block: the pitch drop settles.
  void Settle(const float seconds) {
    if (!_active || _drop <= 0.f) return;
    _drop *= expf(-seconds / kDrumDropSeconds);
    if (_drop < 1e-4f) _drop = 0.f;
    _voice.SetFreq(_freq * (1.f + _drop));
  }

  float Process() {
    if (!_active) return 0.f;
    auto y = _voice.Process();
    if (fabsf(y) < kDrumSilence) {
      if (++_quiet >= _silent_after) _active = false;
    } else {
      _quiet = 0;
    }
    return y * _gain;
  }

private:
  NOCOPY(Drum)

  float _structure;
  float _damping;
  float _brightness;
  float _accent;
  float _octaves;
  float _freq;
  float _drop;
  float _gain;
  uint32_t _quiet;
  uint32_t _silent_after;
  volatile bool _active;
  daisysp::ModalVoice _voice;
};

};
