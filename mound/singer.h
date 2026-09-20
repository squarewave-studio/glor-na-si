#pragma once

#include <cmath>
#include <cstdint>
#include <array>

#include "config.h"

// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
// A singing voice: a glottis and a mouth. Two singers a few cents apart
// make one voice, as the chant did. The glottis is a Liljencrants-Fant
// pulse, shaped by tenseness and read from a table so no sample costs a
// sine or an exponential. The mouth is five formant resonators in a row,
// Klatt's cascade, with the upper three again in parallel, since at C2
// the cascade alone leaves them 15 dB short. Everything a singer does
// to a note is in here: the scoop in, the odd flick, the vibrato that
// arrives late, the drift, the settle at the end, jitter and shimmer,
// breath as tone, and, turned up, the wander, rasp and creak of a voice
// coming apart.

namespace synthux {

namespace voice {

static constexpr float kPi = 3.14159265358979f;
static constexpr uint16_t kBlock = 480;          // samples between Block() calls, 10 ms at 48k
static constexpr uint16_t kTable = 512;

static inline float clampf(const float x, const float lo, const float hi) { return x < lo ? lo : (x > hi ? hi : x); }
static inline float lerpf(const float a, const float b, const float t) { return a + (b - a) * t; }

struct Rng {
  uint32_t s = 1;
  void Seed(const uint32_t seed) { s = seed ? seed : 1; }
  float Uni() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s >> 8) * (1.f / 16777216.f); }
  float Bi()  { return Uni() * 2.f - 1.f; }
  float Norm(){ float a = 0; for (int i = 0; i < 4; i++) a += Bi(); return a * 0.866f; }
};

// Two-pole resonator, unity at DC.
struct Reso {
  float b = 0, c = 0, a = 1, y1 = 0, y2 = 0;
  void Set(const float hz, const float bw, const float sr) {
    const float r = expf(-kPi * bw / sr);
    c = -r * r;
    b = 2.f * r * cosf(2.f * kPi * hz / sr);
    a = 1.f - b - c;
  }
  float Process(const float x) { const float y = a * x + b * y1 + c * y2; y2 = y1; y1 = y; return y; }
};

struct OnePole {
  float a = 0.1f, y = 0;
  void SetHz(const float hz, const float sr) { a = 1.f - expf(-2.f * kPi * hz / sr); }
  float Lp(const float x) { y += a * (x - y); return y; }
};

struct Bandpass {   // RBJ, constant skirt
  float b0 = 0, b2 = 0, a1 = 0, a2 = 0, x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  void Set(const float hz, const float q, const float sr) {
    const float w = 2.f * kPi * hz / sr, s = sinf(w), c = cosf(w), al = s / (2.f * q), a0 = 1.f + al;
    b0 = al / a0; b2 = -al / a0; a1 = -2.f * c / a0; a2 = (1.f - al) / a0;
  }
  float Process(const float x) { const float y = b0 * x + b2 * x2 - a1 * y1 - a2 * y2; x2 = x1; x1 = x; y2 = y1; y1 = y; return y; }
};

using Technique = VoiceTechnique;
static constexpr Technique PLAIN = VOICE_PLAIN, UNDER = VOICE_UNDER, OVER = VOICE_OVER;

// ------------------------------------------------------------------
class Glottis {
public:
  void Init(const float sample_rate, const uint32_t seed) {
    _sr = sample_rate;
    _rng.Seed(seed);
    _asp.Set(900.f, 0.5f, _sr);
    _bright.SetHz(700.f, _sr);
    _drift_a = 1.f - expf(-2.f * kPi * 0.15f * kBlock / _sr);
    _vib_phase = _rng.Uni() * 2.f * kPi;
    _tense_now = _tense_pending = 0.78f;
    _fill(_tense_now);
    SetTimes(_attack, _release);
  }

  // The singer's manner. keen 0 is a machine-steady note, 0.5 a person,
  // apart 0 to 1 the voice coming apart.
  void SetManner(const float keen, const float apart) { _keen = keen; _apart = apart; }
  void SetVibratoHz(const float hz) { _vib_hz = hz; }
  void SetTimes(const float attack_s, const float release_s) {
    _attack = attack_s; _release = release_s;
    _attack_k = 1.f - expf(-1.f / (attack_s * _sr));
    _release_k = 1.f - expf(-1.f / (release_s * _sr));
  }
  void SetTechnique(const Technique t) { _technique = t; }
  void SetBreathy(const float amount) { _breathy = amount; }

  void NoteOn(const float hz) {
    const bool grace = _keen > 0.f && _rng.Uni() < 0.3f * _keen;
    if (!_gate) _prev = grace ? hz : hz * exp2f(-(40.f + 120.f * _keen) / 1200.f);
    else _prev = grace ? hz : _note;
    if (grace) _flick = 1.f;
    _note = hz;
    _t_on = _t;
    _gate = true;
  }
  void NoteOff() { _gate = false; _t_off = _t; }
  bool Gate() const { return _gate; }
  float Intensity() const { return _intensity; }

  // Every kBlock samples, before them.
  void Block() {
    const float dt = kBlock / _sr;
    const float since = _t - _t_on;
    const float keen = _keen, apart = _apart;
    // vibrato arriving after the onset
    const float vib_env = _gate ? clampf((since - 0.35f) / 0.6f, 0.f, 1.f) : 1.f;
    _vib_phase += 2.f * kPi * _vib_hz * (1.f - 0.15f * keen) * (1.f + 0.04f * _rng.Bi() + 0.15f * apart * _rng.Bi()) * dt;
    if (_vib_phase > 2.f * kPi) _vib_phase -= 2.f * kPi;
    const float vib = (8.f + 12.f * keen + 40.f * apart) * vib_env * sinf(_vib_phase);
    // a slow drift, more as it comes apart
    _drift_w += _drift_a * (_rng.Norm() - _drift_w);
    const float drift = (3.f + 6.f * keen + 60.f * apart) * _drift_w * 3.f;
    // the scoop, or the flick, into the note
    const float scoop_s = 0.08f + 0.3f * keen;
    float g = clampf(since / scoop_s, 0.f, 1.f); g = g * g * (3.f - 2.f * g);
    const float note = _prev * powf(_note / _prev, g);
    _flick *= expf(-dt / 0.07f); if (_flick < 0.01f) _flick = 0.f;
    // the settle after the note is let go
    const float since_off = _t - _t_off;
    const float settle_len = _release * 1.2f;
    float s = _gate ? 0.f : clampf(since_off / settle_len, 0.f, 1.f);
    const float settle = -(40.f * keen) * (s * s * (3.f - 2.f * s));
    // tenseness: breathier as it comes apart, wandering too
    _tense_w += 0.03f * (_rng.Norm() - _tense_w);
    _tense_pending = clampf(0.78f - 0.25f * keen * 0.5f - 0.2f * apart - 0.25f * _breathy + (0.05f + 0.3f * apart) * _tense_w * 5.f, 0.15f, 0.95f);
    // breathing the level, slowly
    _breath_env = 1.f + (0.06f + 0.1f * apart) * sinf(2.f * kPi * 0.17f * _t + _breath_phase);
    _target = note * exp2f((vib + drift + settle + keen * 180.f * _flick) / 1200.f);
    _jitter = 0.004f + 0.03f * apart;
    _shimmer = 0.03f + 0.12f * apart;
    _creak = 0.5f * apart;
  }

  // One sample of glottal pulse. The aspiration is separate.
  float Process() {
    _t += 1.f / _sr;
    _intensity += ((_gate ? 1.f : 0.f) - _intensity) * (_gate ? _attack_k : _release_k);
    _phase += _period_f0 / _sr;
    _started = false;
    if (_phase >= 1.f) {
      _phase -= 1.f;
      _started = true;
      if (fabsf(_tense_pending - _tense_now) > 0.01f) { _tense_now = _tense_pending; _fill(_tense_now); }
      _toggle = -_toggle;
      const float creak = _creak * _toggle * 0.12f;
      _period_f0 = _target * (1.f + _jitter * _rng.Norm() + creak);
      float amp = 1.f + _shimmer * _rng.Norm() - _creak * 0.3f * _toggle;
      if (_technique == UNDER) amp *= _toggle > 0 ? 1.f : 0.3f;   // the undertone: every second pulse quiet, an octave below appears
      _period_amp = clampf(amp, 0.2f, 1.6f);
    }
    const float x = _phase * kTable;
    const uint16_t i = (uint16_t)x;
    const float fr = x - i;
    const float raw = (_table[i] + (_table[(i + 1) & (kTable - 1)] - _table[i]) * fr) * _period_amp * _loud;
    const float pulse = raw + 2.5f * (raw - _bright.Lp(raw));
    const float mod = 0.1f + (_phase < 0.5f ? 1.6f * _phase * (1.f - 2.f * _phase) : 0.f);   // the open phase, a bump in place of a sine
    const float amp = _intensity * _intensity * _breath_env;
    _asp_out = _asp.Process(_rng.Bi()) * mod * (1.f - sqrtf(_tense_now)) * 1.4f * (1.f + 2.5f * _breathy) * amp;
    return pulse * amp;
  }
  float Aspiration() const { return _asp_out; }
  bool Started() const { return _started; }
  void SetBreathPhase(const float p) { _breath_phase = p; }

private:
  // The LF pulse for one tenseness, Fant's Rd form, as Pink Trombone has it.
  void _fill(const float tenseness) {
    const float Rd = clampf(3.f * (1.f - tenseness), 0.5f, 2.7f);
    const float Ra = -0.01f + 0.048f * Rd;
    const float Rk = 0.224f + 0.118f * Rd;
    const float Rg = (Rk / 4.f) * (0.5f + 1.2f * Rk) / (0.11f * Rd - Ra * (0.5f + 1.2f * Rk));
    const float Ta = Ra, Tp = 1.f / (2.f * Rg), Te = Tp + Tp * Rk;
    const float eps = 1.f / Ta, shift = expf(-eps * (1.f - Te)), Delta = 1.f - shift;
    const float rhs = ((1.f / eps) * (shift - 1.f) + (1.f - Te) * shift) / Delta;
    const float lower = -(Te - Tp) / 2.f + rhs, upper = -lower;
    const float omega = kPi / Tp;
    const float s = sinf(omega * Te);
    const float y = -kPi * s * upper / (Te - Tp);
    const float z = logf(y);
    const float alpha = z / (Tp / 2.f - Te);
    const float E0 = -1.f / (s * expf(alpha * Te));
    _loud = powf(tenseness, 0.25f);
    for (uint16_t i = 0; i < kTable; i++) {
      const float x = (i + 0.5f) / kTable;
      _table[i] = x > Te ? (-expf(-eps * (x - Te)) + shift) / Delta : E0 * expf(alpha * x) * sinf(omega * x);
    }
  }

  float _sr = 48000.f;
  Rng _rng;
  Bandpass _asp; OnePole _bright;
  std::array<float, kTable> _table;
  float _keen = 0.5f, _apart = 0.f, _vib_hz = 5.4f, _attack = 0.12f, _release = 0.25f, _attack_k = 0.001f, _release_k = 0.0005f, _loud = 1.f;
  Technique _technique = PLAIN;
  float _note = 65.4f, _prev = 65.4f, _target = 65.4f, _period_f0 = 65.4f, _period_amp = 1.f;
  float _t = 0, _t_on = 0, _t_off = -10.f; bool _gate = false;
  float _phase = 0, _toggle = 1.f, _flick = 0;
  float _drift_w = 0, _drift_a = 0.01f, _tense_w = 0, _vib_phase = 0, _breath_env = 1.f, _breath_phase = 0;
  float _tense_now = 0.78f, _tense_pending = 0.78f;
  float _jitter = 0.004f, _shimmer = 0.03f, _creak = 0.f, _breathy = 0.f;
  float _intensity = 0, _asp_out = 0; bool _started = false;
};

// ------------------------------------------------------------------
class Mouth {
public:
  void Init(const float sample_rate) {
    _sr = sample_rate;
    _lips.SetHz(800.f, _sr);
    for (uint8_t k = 0; k < 5; k++) { _f[k] = _tf[k] = 680.f; _bw[k] = _tbw[k] = 100.f; }
  }
  // Move to a sound. Fast for a new syllable, slower for a drift.
  void Shape(const VoiceSound& s, const float rate) {
    for (uint8_t k = 0; k < 5; k++) { _tf[k] = s.f[k]; _tbw[k] = s.bw[k]; }
    _tnasal = s.nasal; _rate = rate; _gain = s.gain;
    _breath_left = s.breath ? 0.08f : 0.f;
  }
  void SetWander(const float w) { _wander = w; }
  void SetTechnique(const Technique t) { _technique = t; }
  void SetPitch(const float hz) { _hz = hz; }
  void Block() {
    const float dt = kBlock / _sr;
    for (uint8_t k = 0; k < 5; k++) {
      float target = _tf[k] + _wander * _tf[k] * 0.06f * _rng.Norm();
      float rate = _rate;
      if (_technique == OVER && k == 1) {          // the overtone: the second formant sits on one harmonic of the note
        target = _hz * kOvertoneHarmonic + _wander * 30.f * _rng.Norm();
        rate = 0.5f;
      }
      _f[k] += rate * (target - _f[k]);
      float bw = _tbw[k]; if (_technique == OVER && k == 1) bw = kOvertoneWidth;
      _bw[k] += rate * (bw - _bw[k]);
      _r[k].Set(_f[k], _bw[k], _sr);
    }
    for (uint8_t k = 0; k < 3; k++) _par[k].Set(_f[k + 1], _bw[k + 1] * 0.8f, _sr);
    _nasal += _rate * (_tnasal - _nasal);
    if (_breath_left > 0.f) { _breath_left -= dt; _voicing = 0.f; _breath = 8.f; }
    else { _voicing += 0.35f * (1.f - _voicing); _breath += 0.35f * (1.f - _breath); }
    _rate += 0.1f * (0.12f - _rate);   // a fast move eases back to the normal pace
  }
  float Process(const float pulse, const float aspiration) {
    const float in = pulse * _voicing + aspiration * _breath;
    float x = in;
    for (uint8_t k = 0; k < 5; k++) x = _r[k].Process(x);
    const float up = _par[0].Process(in) * 0.9f - _par[1].Process(in) * 0.5f + _par[2].Process(in) * 0.3f;
    x += up * 1.2f * (1.f - _nasal);
    const float dull = _lips.Lp(x);
    const float technique = _technique == OVER ? kOvertoneGain : (_technique == UNDER ? kUndertoneGain : 1.f);
    return lerpf(x, dull * 0.7f, _nasal) * _gain * technique;
  }
  void SetSeed(const uint32_t seed) { _rng.Seed(seed); }

private:
  float _sr = 48000.f;
  Rng _rng;
  Reso _r[5], _par[3]; OnePole _lips;
  float _f[5], _bw[5], _tf[5], _tbw[5];
  float _nasal = 0, _tnasal = 0, _rate = 0.12f, _wander = 0, _gain = 1.f, _hz = 65.41f;
  float _voicing = 1.f, _breath = 1.f, _breath_left = 0.f;
  Technique _technique = PLAIN;
};

// ------------------------------------------------------------------
class Singer {
public:
  void Init(const float sample_rate, const uint32_t seed) {
    _g.Init(sample_rate, seed); _m.Init(sample_rate); _m.SetSeed(seed + 7);
    _g.SetBreathPhase((seed & 7) * 0.8f);
  }
  Glottis& glottis() { return _g; }
  const Glottis& glottis() const { return _g; }
  Mouth& mouth() { return _m; }
  float Process() { return _m.Process(_g.Process(), _g.Aspiration()); }
  void Block() { _g.Block(); _m.Block(); }
private:
  Glottis _g; Mouth _m;
};

}; // namespace voice

}; // namespace synthux
