// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
#pragma once
#include <array>

#include <daisysp.h>

#include "nocopy.h"
#include "config.h"

#include "drum.h"
#include "chant.h"
#include "xfade.h"

namespace synthux {

class Mound {
public:
  Mound();
  ~Mound() {}

  void Init(const float sample_rate, const float buffer_size);

  void Strike(const uint8_t index);

  // Position along the passage, 0 at the entrance and 1 in the chamber.
  void SetWalk(const float value);

  // Sample-and-hold at strike time.
  void SetDrumTone(const float value) { _drum.SetStructure(value); }
  void SetDrumDecay(const float value) { _drum.SetDamping(value); }
  void SetDrumLevel(const float value) { _drum.SetLevel(value); }

  // Voices run while they sound, in either mode, so a latched voice
  // carries into DRUM. The UI owns the gates: pads, latch and mode.
  void SetSing(const uint8_t voice, const bool on) { _chants[voice].SetGate(on); }
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
  daisysp::ReverbSc _reverb;
  XFade _xfade;

  std::array<float, 2> _bus;
  std::array<float, 2> _reverb_out;
  std::array<float, 2> _mix;
};

};
