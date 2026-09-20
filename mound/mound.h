// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
#pragma once
#include <array>

#include <daisysp.h>

#include "nocopy.h"
#include "config.h"

#include "drum.h"
#include "chant.h"
#include "passage.h"

namespace synthux {

class Mound {
public:
  Mound();
  ~Mound() {}

  void Init(const float sample_rate, const float buffer_size);

  // A strike is pinned where the listener stands, or where you say.
  void Strike(const uint8_t index);
  void Strike(const uint8_t index, const float metres);

  // Where the listener is going, 0 outside the entrance and 1 the back
  // of the chamber, and where they stand now, in metres.
  void SetWalk(const float value) { _passage.SetWalk(value); }
  float Listener() const { return _passage.Listener(); }

  // Sample-and-hold at strike time.
  void SetDrumTone(const float value) { _drum.SetStructure(value); }
  void SetDrumDecay(const float value) { _drum.SetDamping(value); }
  void SetDrumLevel(const float value) { _drum.SetLevel(value); }
  void SetDrumHardness(const float value) { _drum.SetHardness(value); }
  void SetDrumTune(const float value) { _drum.SetTune(value); }

  // Voices run while they sound, in either mode, so a latched voice
  // carries into DRUM. The UI owns the gates: pads, loop and mode. A
  // voice that opens is pinned where the listener stands, or where
  // the loop says.
  void SetSing(const uint8_t voice, const bool on);
  void Sing(const uint8_t voice, const float metres);
  void SetVowel(const uint8_t voice, const float vowel) { _chants[voice].SetVowel(vowel); }
  void SetInterval(const uint8_t voice, const float semitones) { _chants[voice].SetInterval(semitones); }

  // Shared by the voices.
  void SetVoiceLevel(const float value);
  void SetPitch(const float value);
  void SetSpread(const float value);
  void SetSwell(const float value);
  void SetUnsteadiness(const float value);

  void Process(float **out, size_t size);

private:
  NOCOPY(Mound)

  Drum _drum;
  std::array<Chant, kVoiceCount> _chants;
  uint16_t _clock;
  float _seconds_per_sample;
  std::array<float, kVoiceCount> _voice_at;
  Passage _passage;
};

};
