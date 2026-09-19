#pragma once

#include "daisysp.h"
#include "nocopy.h"
#include "config.h"

// Wraps a single daisysp::ModalVoice (Plaits-style struck resonator).
// Monophonic: every Strike retriggers the same body. Timbre settings are
// held and only reach the voice inside Strike.

namespace synthux {

class Drum {
public:
  Drum():
  _structure  { .5f },
  _damping    { .5f },
  _brightness { .5f },
  _accent     { .5f },
  _gain       { kDrumGain }
  {}
  ~Drum() {}

  void Init(float sample_rate) {
    _voice.Init(sample_rate);
    _voice.SetSustain(false);
  }

  void SetStructure(const float value) {
    _structure = daisysp::fmap(value, kDrumStructureMin, kDrumStructureMax);
  }

  void SetDamping(const float value) {
    _damping = daisysp::fmap(value, kDrumDampingMin, kDrumDampingMax);
  }

  void SetBrightness(const float value) {
    _brightness = daisysp::fclamp(value, 0.f, kDrumBrightnessCap);
  }

  void SetAccent(const float value) {
    _accent = value;
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
    _voice.SetFreq(freq);
    _voice.Trig();
  }

  float Process() {
    return _voice.Process() * _gain;
  }

private:
  NOCOPY(Drum)

  float _structure;
  float _damping;
  float _brightness;
  float _accent;
  float _gain;
  daisysp::ModalVoice _voice;
};

};
