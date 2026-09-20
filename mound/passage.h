#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include "daisysp.h"
#include "nocopy.h"
#include "config.h"

// The mound as a line, metres along the axis, see config.h. Three
// sources, the drum and the two voices, each pinned somewhere on it. One
// listener, walking. Every source has a delay line: the listener reads
// it at the distance between them, and two fixed taps feed the chamber
// and the mouth. The chamber is the reverb and the ring, and its output
// comes back down a line of its own. The mouth sends a dull inverted
// slap up a third line, to the listener and on into the chamber. Every
// path loses level and top per metre, so where you stand is what you hear.

namespace synthux {

class Passage {
public:
  static constexpr uint8_t kSourceCount = 1 + kVoiceCount;
  static constexpr size_t kLineSize = 4096; // samples, over 28 m at 48 kHz
  static constexpr uint8_t kLineCount = kSourceCount + 3;
  static_assert(kLineSize >= (kOutsideMetres + kPassageMetres + kChamberMetres) * 48000.f / kSpeedOfSound + 4.f,
                "the line has to hold the whole axis");

  Passage();
  ~Passage() {}

  void Init(const float sample_rate);

  // Where the listener is going, 0 outside the entrance, 1 the back of
  // the chamber. The walk there takes as long as it takes.
  void SetWalk(const float value);

  // Where the listener stands now, metres.
  float Listener() const { return _listener; }

  // Ask for a source to be pinned at a position, metres. Taken up by
  // the next Update, so the audio side sees a whole change. A source
  // that is sounding should fade across, one that is silent should not,
  // or its attack softens.
  void Pin(const uint8_t source, const float metres, const bool fade);

  // Once per block, ahead of the samples.
  void Update();

  void Process(const std::array<float, kSourceCount>& in, float& left, float& right);

private:
  NOCOPY(Passage)

  struct Line {
    float* buf;
    uint32_t w;
    void Clear();
    void Write(const float x) {
      buf[w] = x;
      w = (w + 1) & (kLineSize - 1);
    }
    // Samples ago, 0 is the one just written.
    float Read(const uint32_t d) const {
      return buf[(w - 1 - d) & (kLineSize - 1)];
    }
    float Read(const float d) const {
      auto i = (uint32_t)d;
      auto f = d - (float)i;
      auto a = buf[(w - 1 - i) & (kLineSize - 1)];
      auto b = buf[(w - 2 - i) & (kLineSize - 1)];
      return a + (b - a) * f;
    }
  };

  struct OnePole {
    float a;
    float y;
    float Process(const float x) {
      y += a * (x - y);
      return y;
    }
  };

  // Two poles ringing at one frequency. The input is scaled so a tone
  // held on the mode comes back at the mode's gain; an impulse rings
  // far more quietly, as a room does.
  struct Mode {
    float c1, c2, g;
    float y1, y2;
    float Process(const float x) {
      auto y = g * x + c1 * y1 - c2 * y2;
      y2 = y1;
      y1 = y;
      return y;
    }
  };

  struct Peak {
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
    float Process(const float x) {
      auto y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
      x2 = x1; x1 = x;
      y2 = y1; y1 = y;
      return y;
    }
  };

  // A pinned source. While 'fade' runs down, every tap blends from
  // where it was to where it is, so a sounding source moves without
  // a step.
  struct Source {
    float at = 0.f;                  // metres
    float from = 0.f;                // metres, before the last pin
    uint32_t fade = 0;               // samples left crossing over
    uint32_t fade_length = 0;
    uint32_t to_chamber = 0;         // samples
    uint32_t to_mouth = 0;
    uint32_t from_to_chamber = 0;
    uint32_t from_to_mouth = 0;
    float g_chamber = 0.f;
    float g_mouth = 0.f;
    float from_g_chamber = 0.f;
    float from_g_mouth = 0.f;
    float g_direct = 0.f;            // per block
    OnePole lp { 0.f, 0.f };
    float request = 0.f;             // pending pin, metres
    bool request_fade = false;
    std::atomic<bool> pending { false };
  };

  void _pin(Source& s, const float metres, const bool fade);
  float _coef(const float hz) const;
  float _duct(const float metres) const;
  float _cutoff(const float metres) const;
  void _colour(Peak& p, const float hz, const float db);

  float _sr;
  float _spm;                // samples per metre
  float _listener;
  float _target;
  float _pace;               // metres per sample
  float _cutoff_scale;

  std::array<Source, kSourceCount> _sources;
  std::array<Line, kSourceCount> _lines;
  Line _chamber_l;
  Line _chamber_r;
  Line _mouth;

  uint32_t _mouth_to_chamber;
  float _g_mouth_to_chamber;
  float _g_chamber_ret;
  float _g_mouth_ret;
  float _g_out;

  daisysp::ReverbSc _reverb;
  std::array<Mode, kChamberModes.size()> _modes;
  OnePole _mouth_lp;
  OnePole _chamber_lp_l;
  OnePole _chamber_lp_r;
  OnePole _mouth_ret_lp;
  Peak _colour_l;
  Peak _colour_r;

  // The width: a wave bouncing between the walls, one short fed-back
  // line per channel in ordinary RAM.
  static constexpr size_t kWidthLine = 512;
  std::array<float, kWidthLine> _width_l;
  std::array<float, kWidthLine> _width_r;
  uint32_t _width_w;
  uint32_t _width_d;
  float _width_g;
  OnePole _width_lp_l;
  OnePole _width_lp_r;
};

};
