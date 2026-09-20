#pragma once

#include "body.h"

// A stone struck with the stick: the strike into three short bright
// modes, no skin. Three stones, three sizes.

namespace synthux {

class Stone {
public:
  Stone():
  _sr { 48000.f },
  _hard { 0.5f },
  _gain { kStoneGain },
  _life { 0 },
  _active { false } {}
  ~Stone() {}

  void Init(const float sr) {
    _sr = sr;
    _strike.Init(sr);
    for (auto& m : _modes) m.SetDecay(kStoneDecay, sr);
  }

  void SetHardness(const float value) { _hard = value; }
  void SetLevel(const float value) { _gain = kStoneGain * daisysp::fmap(value, 0.f, kLevelMax); }
  bool Active() const { return _active; }

  // The hit itself. Audio side only, at the start of a block.
  void Hit(const uint8_t index) {
    if (index >= kStonePadCount) return;
    for (uint8_t k = 0; k < kStoneRatios.size(); k++) {
      _modes[k].SetFreq(kStonePads[index].hz * kStoneRatios[k], _sr);
    }
    _strike.Hit(kStoneContact / (0.5f + _hard), 1.f, kStoneSlap, kStoneSlapSeconds, kStoneSlapLow, kStoneSlapHigh);
    _life = (uint32_t)(kStoneDecay * 3.f * _sr);
    _active = true;
  }

  float Process() {
    if (!_active) return 0.f;
    auto x = _strike.Process();
    float y = _strike.Slap();
    for (auto& m : _modes) y += m.Process(x);
    if (_life > 0 && --_life == 0) _active = false;
    return y * _gain;
  }

private:
  NOCOPY(Stone)

  float _sr;
  float _hard;
  float _gain;
  uint32_t _life;
  bool _active;
  Exciter _strike;
  std::array<Mode, kStoneRatios.size()> _modes;
};

};
