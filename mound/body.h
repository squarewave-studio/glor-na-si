#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include "daisysp.h"
#include "nocopy.h"
#include "config.h"

// What the skin and the stone share: a bank of two-pole modes, and the
// strike, a mallet pulse with a whisper of noise, shorter and brighter
// the harder it is.

namespace synthux {

struct Mode {
  float c1 = 0.f, c2 = 0.f, g = 0.f;
  float y1 = 0.f, y2 = 0.f;
  float r = 0.f;
  // T60 sets the pole radius, the frequency the angle. The input is
  // scaled so an impulse rings at about amplitude one.
  void SetDecay(const float t60, const float sr) {
    r = expf(-6.907755f / (t60 * sr));
  }
  void SetFreq(const float hz, const float sr) {
    auto theta = 6.2831853f * hz / sr;
    c1 = 2.f * r * cosf(theta);
    c2 = r * r;
    g = sinf(theta);
  }
  float Process(const float x) {
    auto y = g * x + c1 * y1 - c2 * y2;
    y2 = y1;
    y1 = y;
    return y;
  }
};

class Exciter {
public:
  Exciter(): _sr { 48000.f }, _pos { 0 }, _n { 0 }, _amp { 0.f }, _noise_amp { 0.f }, _noise_env { 0.f }, _noise_a { 0.f }, _hp_a { 0.f }, _lp_a { 1.f }, _lp { 0.f }, _lp2 { 0.f }, _rng { 0x2545f491u } {}
  void Init(const float sr) { _sr = sr; }

  // The pulse has unit area times the level, so the low modes ring the
  // same however short the contact.
  // The slap is a burst of noise in a band, with its own decay.
  void Hit(float contact, const float level, const float slap, const float slap_seconds, const float low_hz, const float high_hz) {
    contact *= 1.f + kSkinVary * _noise();
    _n = (uint32_t)daisysp::fmax(contact * _sr, 4.f);
    _pos = 0;
    _amp = level * (1.f + kSkinVary * _noise()) * 3.14159265f / (2.f * (float)_n);
    _noise_amp = slap;
    _noise_env = 1.f;
    _noise_a = expf(-1.f / (slap_seconds * _sr));
    _hp_a = 1.f - expf(-6.2831853f * low_hz / _sr);
    _lp_a = 1.f - expf(-6.2831853f * high_hz / _sr);
  }

  // The pulse, what drives the skin.
  float Process() {
    if (_pos >= _n) return 0.f;
    auto x = _amp * sinf(3.14159265f * (float)_pos / (float)_n);
    _pos++;
    return x;
  }

  // The slap, heard straight.
  float Slap() {
    if (_noise_env < 1e-3f) return 0.f;
    auto w = _noise() * _noise_env;
    _noise_env *= _noise_a;
    _lp += _hp_a * (w - _lp);
    auto band = w - _lp;
    _lp2 += _lp_a * (band - _lp2);
    return _lp2 * _noise_amp;
  }

private:
  float _noise() {
    _rng ^= _rng << 13; _rng ^= _rng >> 17; _rng ^= _rng << 5;
    return (_rng & 0xffffff) * (1.f / 8388608.f) - 1.f;
  }
  float _sr;
  uint32_t _pos;
  uint32_t _n;
  float _amp;
  float _noise_amp;
  float _noise_env;
  float _noise_a;
  float _hp_a;
  float _lp_a;
  float _lp;
  float _lp2;
  uint32_t _rng;
};

// J_m(x) by its series. Init only, in double.
inline double bessel_j(const int m, const double x) {
  auto half = x / 2.0;
  auto term = 1.0;
  for (int i = 1; i <= m; i++) term *= half / i;
  auto sum = term;
  for (int k = 1; k < 60; k++) {
    term *= -(half * half) / (double)(k * (k + m));
    sum += term;
    if (fabs(term) < 1e-15) break;
  }
  return sum;
}

};
