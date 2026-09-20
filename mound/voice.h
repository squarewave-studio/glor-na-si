#pragma once
// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
// Two singers a few cents apart as one voice, what the mound talks to.
#include "singer.h"

namespace synthux {

class Voice {
public:
  void Init(const float sample_rate, const uint32_t seed) {
    _sr = sample_rate;
    _a.Init(sample_rate, seed); _b.Init(sample_rate, seed + 1);
    _a.glottis().SetVibratoHz(kVoiceVibratoHz);
    _b.glottis().SetVibratoHz(kVoiceVibratoHz + kVoiceVibratoSpread);
    _apply();
  }

  // A syllable: the sound, the interval above the pitch knob, how it is
  // sung. A voice already singing moves to it without stopping.
  void Sing(const uint8_t sound, const float semitones, const voice::Technique technique = voice::PLAIN) {
    _sound = sound < kSoundCount ? sound : 0; _semitones = semitones; _technique = technique;
    for (auto* s : { &_a, &_b }) {
      s->mouth().Shape(kSounds[_sound], 0.35f);
      s->mouth().SetTechnique(technique);
      s->glottis().SetTechnique(technique);
      s->glottis().SetBreathy(kSounds[_sound].breathy);
    }
    _tune();
    _a.glottis().NoteOn(_hz_a); _b.glottis().NoteOn(_hz_b);
  }
  void Rest() { _a.glottis().NoteOff(); _b.glottis().NoteOff(); }

  // Pot centre is C2, an octave either way. Live.
  void SetPitch(const float value) { _octaves = (value - 0.5f) * 2.f * kVoicePitchOctaves; _tune(); }
  void SetInterval(const float semitones) { _semitones = semitones; _tune(); }
  void SetLevel(const float value) { _gain = kVoiceGain * value * kLevelMax; }
  void SetSpread(const float value) { _spread = value * kVoiceSpreadMax; _tune(); }
  void SetSwell(const float value) {
    const float attack = kVoiceSwellMin * powf(kVoiceSwellMax / kVoiceSwellMin, value);
    for (auto* s : { &_a, &_b }) s->glottis().SetTimes(attack, attack * kVoiceReleaseRatio);
  }
  // S34: left a steady voice, centre a person, right a voice coming apart.
  void SetUnsteadiness(const float value) {
    _keen = value < 0.5f ? value : 0.5f;
    _apart = value > 0.5f ? (value - 0.5f) * 2.f : 0.f;
    _apply();
  }
  // A drum hit nearby: the voice comes apart a little more, and settles.
  void Knock(const float amount) { if (amount > _knock) { _knock = amount; _apply(); } }
  void Settle(const float seconds) {
    if (_knock <= 0.f) return;
    _knock *= expf(-seconds / 0.5f);
    if (_knock < 1e-3f) _knock = 0.f;
    _apply();
  }
  bool IsSounding() const {
    return _a.glottis().Gate() || _b.glottis().Gate() || _a.glottis().Intensity() > kVoiceSilence || _b.glottis().Intensity() > kVoiceSilence;
  }
  // block is true on the first sample of each kBlock slot, as the owner clocks it.
  float Process(const bool block) {
    if (block) { _a.Block(); _b.Block(); }
    return (_a.Process() + _b.Process()) * _gain;
  }

private:
  void _tune() {
    const float hz = kVoicePitch * exp2f(_octaves + _semitones / 12.f);
    _hz_a = hz * exp2f(-_spread * 0.5f / 1200.f);
    _hz_b = hz * exp2f(_spread * 0.5f / 1200.f);
    _a.mouth().SetPitch(_hz_a); _b.mouth().SetPitch(_hz_b);
    // a moving pitch knob retunes a sounding note without a new scoop
    if (_a.glottis().Gate()) { _a.glottis().NoteOn(_hz_a); _b.glottis().NoteOn(_hz_b); }
  }
  void _apply() {
    const float apart = voice::clampf(_apart + _knock * (0.3f + _apart), 0.f, 1.f);
    for (auto* s : { &_a, &_b }) { s->glottis().SetManner(_keen, apart); s->mouth().SetWander(apart); }
  }

  float _sr = 48000.f;
  voice::Singer _a, _b;
  uint8_t _sound = SOUND_AAH; float _semitones = 0.f, _octaves = 0.f, _spread = 8.f, _hz_a = 65.41f, _hz_b = 65.41f;
  voice::Technique _technique = voice::PLAIN;
  float _keen = 0.5f, _apart = 0.f, _knock = 0.f, _gain = kVoiceGain;
};

};
