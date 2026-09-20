#pragma once

#include <cmath>
#include "daisysp.h"
#include "nocopy.h"
#include "config.h"
#include "chantvoice.h"

// Wraps a pair of chant_voice instances, the additive chant engine. The
// second sits a few cents sharp of the first and the two together are one
// voice. The engine wants chant_block() every CHANT_BLOCK samples, ahead
// of the samples themselves; the owner clocks that so several voices can
// take turns and their table rebuilds never land in the same callback.

namespace synthux {

class Chant {
public:
  Chant():
  _octaves     { 0.f },
  _semitones   { 0.f },
  _wander_ind  { 0.f },
  _drift_cents { 0.f },
  _jitter      { 0.f },
  _shimmer     { 0.f },
  _amount      { 1.f },
  _knock       { 0.f },
  _gain        { kVoiceGain }
  {}
  ~Chant() {}

  void Init(float sample_rate, uint32_t seed) {
    chant_init(&_a, sample_rate, seed);
    chant_init(&_b, sample_rate, seed + 1);
    // The engine's own tuning, kept so unsteadiness can scale it.
    _wander_ind  = _a.wander_ind;
    _drift_cents = _a.drift_cents;
    _jitter      = _a.jitter;
    _shimmer     = _a.shimmer;
  }

  void SetGate(const bool on) {
    chant_gate(&_a, on);
    chant_gate(&_b, on);
  }

  // Pot centre is the measured pitch, an octave either way. Live.
  void SetPitch(const float value) {
    _octaves = daisysp::fmap(value, -kVoicePitchOctaves, kVoicePitchOctaves);
    _tune();
  }

  // Semitones above the pitch knob, from the pad.
  void SetInterval(const float semitones) {
    _semitones = semitones;
    _tune();
  }

  // Live. Pot centre is kVoiceGain.
  void SetLevel(const float value) {
    _gain = kVoiceGain * daisysp::fmap(value, 0.f, kLevelMax);
  }

  // Cents between the two singers. 0 is one voice.
  void SetSpread(const float value) {
    _b.detune = daisysp::fmap(value, 0.f, kVoiceSpreadMax);
  }

  // Scales the per-harmonic wander and the pitch drift. 1 is as tuned.
  // The shared wander breathes the whole level, so it stays as tuned or
  // full unsteadiness surges into the clipper.
  void SetUnsteadiness(const float value) {
    _amount = daisysp::fmap(value, 0.f, kVoiceUnsteadyMax);
    _apply();
  }

  // A knock: the wander, drift and rasp jump by this much, times the
  // unsteadiness, and settle back. The owner calls Settle every block.
  void Knock(const float amount) {
    if (amount > _knock) {
      _knock = amount;
      _apply();
    }
  }

  void Settle(const float seconds) {
    if (_knock <= 0.f) return;
    _knock *= expf(-seconds / kKnockSeconds);
    if (_knock < 1e-3f) _knock = 0.f;
    _apply();
  }

  // Profile index, 0 mmm .. 5 overtone, fractional is a blend.
  void SetVowel(const float vowel) {
    _a.vowel = _b.vowel = daisysp::fclamp(vowel, 0.f, CHANT_NPROFILES - 1);
  }

  void SetSwell(const float value) {
    auto on_time = daisysp::fmap(value, kVoiceSwellMin, kVoiceSwellMax);
    _a.on_time = _b.on_time = on_time;
    _a.off_time = _b.off_time = on_time * kVoiceReleaseRatio;
  }

  // True while a note is open or its tail is still above the floor.
  bool IsSounding() {
    return _a.gate || _b.gate
        || _a.env >= kVoiceSilence || _b.env >= kVoiceSilence;
  }

  // The owner sets block on the first sample of each CHANT_BLOCK.
  float Process(const bool block) {
    if (block) {
      chant_block(&_a);
      chant_block(&_b);
    }
    return (chant_process(&_a) + chant_process(&_b)) * _gain;
  }

private:
  NOCOPY(Chant)

  void _tune() {
    _a.f0 = _b.f0 = kVoicePitch * exp2f(_octaves + _semitones / 12.f);
  }

  void _apply() {
    auto amount = _amount * (1.f + _knock);
    auto rasp = 1.f + _knock * _amount;
    for (auto v : { &_a, &_b }) {
      v->wander_ind  = _wander_ind * amount;
      v->drift_cents = _drift_cents * amount;
      v->jitter      = _jitter * rasp;
      v->shimmer     = _shimmer * rasp;
    }
  }

  chant_voice _a;
  chant_voice _b;
  float _octaves;
  float _semitones;
  float _wander_ind;
  float _drift_cents;
  float _jitter;
  float _shimmer;
  float _amount;
  float _knock;
  float _gain;
};

};
