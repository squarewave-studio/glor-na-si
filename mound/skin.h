#pragma once

#include "body.h"

// The drum: a circular skin. Eight modes at the Bessel ratios, each rung
// by a strike as much as its shape says at the strike's radius, so the
// pads are places on one skin. S31 is the size, S32 the stiffness, S33
// the decay, S34 the hardness. Every hit starts sharp and settles.

namespace synthux {

class Skin {
public:
  Skin():
  _sr { 48000.f },
  _octaves { 0.f },
  _stiff { 0.f },
  _t60 { 1.f },
  _hard { 0.5f },
  _hard_now { 0.5f },
  _stiff_now { 0.f },
  _gain { kSkinGain },
  _f0 { kSkinFundamental },
  _drop { 0.f },
  _tick_lp { 0.f },
  _life { 0 },
  _active { false } {
    for (auto& a : _amp) a = 0.f;
  }
  ~Skin() {}

  void Init(const float sr) {
    _sr = sr;
    _strike.Init(sr);
    for (uint8_t p = 0; p < kSkinPadCount; p++) {
      for (uint8_t k = 0; k < kSkinModeCount; k++) {
        auto& mode = kSkinModes[k];
        _shape[p][k] = (float)bessel_j(mode.m, mode.zero * kSkinPads[p].radius) * mode.gain;
      }
    }
    _setdecay();
    _retune();
  }

  // Pot centre is the drum as built, an octave either way. Next strike.
  void SetSize(const float value) { _octaves = daisysp::fmap(value, -kSkinSizeOctaves, kSkinSizeOctaves); }
  void SetStiffness(const float value) { _stiff = value * kSkinStiffnessMax; }
  void SetDecay(const float value) { _t60 = daisysp::fmap(value, kSkinDecayMin, kSkinDecayMax, daisysp::Mapping::LOG); }
  void SetHardness(const float value) { _hard = value; }

  // Live. Pot centre is kSkinGain.
  void SetLevel(const float value) { _gain = kSkinGain * daisysp::fmap(value, 0.f, kLevelMax); }

  bool Active() const { return _active; }

  // Once a block, ahead of the samples: the pitch settling.
  void Settle(const float seconds) {
    if (!_active || _drop <= 0.f) return;
    _drop *= expf(-seconds / kSkinDropSeconds);
    if (_drop < 1e-4f) _drop = 0.f;
    _retune();
  }

  float Process() {
    if (!_active) return 0.f;
    auto x = _strike.Process();
    _tick_lp += 0.075f * (x - _tick_lp);   // about 600 Hz
    float y = (x - _tick_lp) * kSkinTick + _strike.Slap();
    for (uint8_t k = 0; k < kSkinModeCount; k++) y += _modes[k].Process(x * _amp[k]);
    if (_life > 0 && --_life == 0) _active = false;
    return y * _gain;
  }

private:
  NOCOPY(Skin)

public:
  // The hit itself. Audio side only, at the start of a block.
  void Hit(const uint8_t index) {
    if (index >= kSkinPadCount) return;
    _hard_now = daisysp::fclamp(_hard + kSkinPads[index].hard, 0.f, 1.f);
    _stiff_now = _stiff;
    _f0 = kSkinFundamental * exp2f(_octaves);
    // The head: a disc of this radius over the skin's rounds off each
    // mode by 2 J1(z) / z at z = zero x head. The pad's lift is on
    // every mode alike, so the balance at that radius is the skin's.
    auto head = kSkinHeadSoft * powf(kSkinHeadHard / kSkinHeadSoft, _hard_now);
    for (uint8_t k = 0; k < kSkinModeCount; k++) {
      auto z = (double)kSkinModes[k].zero * head;
      auto round = (float)(2.0 * bessel_j(1, z) / z);
      _amp[k] = _shape[index][k] * round * kSkinPads[index].gain;
    }
    // Soft to hard is long to short, so the map runs backwards, by hand.
    // A bigger skin gives longer.
    auto contact = kSkinContactSoft * powf(kSkinContactHard / kSkinContactSoft, _hard_now) * sqrtf(kSkinFundamental / _f0);
    auto slap = kSkinSlap * (0.33f + 0.67f * _hard_now) * (0.6f + 0.8f * kSkinPads[index].radius);
    _strike.Hit(contact, 1.f - kSkinAccent + kSkinAccent * _hard_now, slap, kSkinSlapSeconds, kSkinSlapLow, kSkinSlapHigh);
    _drop = kSkinDropAmount * _hard_now;
    _setdecay();
    _retune();
    _life = (uint32_t)(_t60 * 1.5f * _sr);
    _active = true;
  }

private:

  float _ratio(const uint8_t k) const {
    auto ratio = kSkinModes[k].zero / kSkinModes[0].zero;
    return ratio * (1.f + _stiff_now * (ratio * ratio - 1.f));
  }

  void _setdecay() {
    for (uint8_t k = 0; k < kSkinModeCount; k++) {
      _modes[k].SetDecay(_t60 / powf(_ratio(k), kSkinDecaySlope), _sr);
    }
  }

  void _retune() {
    for (uint8_t k = 0; k < kSkinModeCount; k++) {
      _modes[k].SetFreq(_f0 * _ratio(k) * (1.f + _drop), _sr);
    }
  }

  float _sr;
  float _octaves;
  float _stiff;
  float _t60;
  float _hard;
  float _hard_now;
  float _stiff_now;
  float _gain;
  float _f0;
  float _drop;
  float _tick_lp;
  uint32_t _life;
  bool _active;
  Exciter _strike;
  std::array<Mode, kSkinModeCount> _modes;
  std::array<float, kSkinModeCount> _amp;
  std::array<std::array<float, kSkinModeCount>, kSkinPadCount> _shape;
};

};
