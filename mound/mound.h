// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
#pragma once
#include <array>

#include <daisysp.h>

#include "nocopy.h"
#include "config.h"

#include "queue.h"
#include "skin.h"
#include "stone.h"
#include "voice.h"
#include "passage.h"

namespace synthux {

class Mound {
public:
  Mound();
  ~Mound() {}

  void Init(const float sample_rate, const float buffer_size);

  // Strikes and syllables are commands from the UI, queued whole and
  // taken up by the audio side at the start of the next block, with
  // the pin and the knock together.

  // A strike is a place on the skin, or a stone, pinned where the
  // listener stands or where you say.
  void Strike(const uint8_t index);
  void Strike(const uint8_t index, const float metres);

  // A voice opens on a sound at an interval, sung plain or with a
  // technique, pinned where you say. A voice already singing moves to
  // the new syllable.
  void Sing(const uint8_t voice, const uint8_t sound, const float semitones, const uint8_t technique, const float metres);
  void Rest(const uint8_t voice);

  // The same with a remembered sound and interval, for the desktop renders.
  void SetSound(const uint8_t voice, const uint8_t sound) { _sound[voice] = sound; }
  void SetInterval(const uint8_t voice, const float semitones) { _semitones[voice] = semitones; }
  void SetSing(const uint8_t voice, const bool on);
  void Sing(const uint8_t voice, const float metres) { Sing(voice, _sound[voice], _semitones[voice], VOICE_PLAIN, metres); }

  // Where the listener is going, 0 outside the entrance and 1 the back
  // of the chamber, and where they stand now, in metres.
  void SetWalk(const float value) { _passage.SetWalk(value); }
  float Listener() const { return _passage.Listener(); }

  // Held and applied at the next strike.
  void SetDrumTune(const float value) { _skin.SetSize(value); }
  void SetDrumTone(const float value) { _skin.SetStiffness(value); }
  void SetDrumDecay(const float value) { _skin.SetDecay(value); }
  void SetDrumHardness(const float value);
  void SetDrumLevel(const float value);

  // Shared by the voices.
  void SetVoiceLevel(const float value);
  void SetPitch(const float value);
  void SetSpread(const float value);
  void SetSwell(const float value);
  void SetUnsteadiness(const float value);

  void Process(float **out, size_t size);

private:
  NOCOPY(Mound)

  struct StrikeCmd {
    uint8_t index;
    float metres;
  };
  struct SingCmd {
    uint8_t voice;
    bool on;
    uint8_t sound;
    uint8_t technique;
    float semitones;
    float metres;
  };

  void _drain();
  void _knock(const float metres);

  Skin _skin;
  Stone _stone;
  std::array<Voice, kVoiceCount> _voices;
  uint16_t _clock;
  float _seconds_per_sample;
  std::array<float, kVoiceCount> _voice_at;
  std::array<uint8_t, kVoiceCount> _sound;
  std::array<float, kVoiceCount> _semitones;
  Queue<StrikeCmd, 8> _strikes;
  Queue<SingCmd, 8> _sings;
  Passage _passage;
};

};
