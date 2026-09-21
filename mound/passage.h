#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "daisysp.h"
#include "nocopy.h"
#include "config.h"
#include "reverb.h"

// The mound as a tube, metres along the axis, see config.h. Sound goes
// both ways along it, a section per metre, each the width and height of
// the real one, and turns back a little wherever the section changes and
// wholly at the ends: the open mouth and the stone at the back of the
// chamber. So the passage has its own echoes and standing waves. Four
// sources, the skin, the stone and the two voices, each pinned somewhere
// along it and put into both directions there. One listener, walking,
// who hears the pressure where they stand, both directions summed.
// Close to a source the sound still spreads as 1/r before the passage
// takes it, so each source has a short line of its own for that, which
// also carries its send to the room. The chamber is a reverb and a
// ring, fed by every source, and its answer goes into the tube at the
// chamber like any other sound, so it too meets the mouth and stands in
// the passage. Only the reverb's width comes down a line of its own.

namespace synthux {

class Passage {
public:
  static constexpr uint8_t kSourceCount = 2 + kVoiceCount; // skin, stone, voices
  static constexpr size_t kSectionSize = 256;  // samples, a metre at 48 kHz is 140
  static constexpr size_t kReturnSize = 4096;  // samples, over 28 m at 48 kHz
  static constexpr size_t kNearSize = 2048;    // samples, over kSendMetres
  static constexpr float kSeamMetres = 0.1f;   // the listener's read crosses a section boundary over this
  static constexpr size_t kMemory = 2 * kSectionCount * kSectionSize + kReturnSize + kSourceCount * kNearSize;
  static_assert(kSectionCount == (uint8_t)(kPassageMetres + kChamberMetres), "a section per metre of passage and chamber");
  static_assert(kSectionSize >= 48000.f / kSpeedOfSound + 4.f, "a section has to hold a metre");
  static_assert(kReturnSize >= (kOutsideMetres + kPassageMetres + kChamberMetres) * 48000.f / kSpeedOfSound + 4.f,
                "the return has to reach the whole axis");
  static_assert(kNearSize >= kSendMetres * 48000.f / kSpeedOfSound + 4.f, "the near line has to reach kSendMetres");
  static_assert(kSendMetres >= kSpreadMetres, "the near line carries the near field too");

  Passage();
  ~Passage() {}

  void Init(const float sample_rate);

  // Where the listener is going, 0 outside the entrance, 1 the back of
  // the chamber. The walk there takes as long as it takes.
  void SetWalk(const float value);

  // Where the listener stands now, metres.
  float Listener() const { return _listener; }

  // Pin a source at a position, metres. Audio side only, at the start
  // of a block. A source that is sounding should fade across, one that
  // is silent should not, or its attack softens.
  void PinNow(const uint8_t source, const float metres, const bool fade);

  // Once per block, ahead of the samples, with the block's length so
  // the gains that follow the listener can ramp across it.
  void Update(const size_t block = kAudioBlockSize);

  void Process(const std::array<float, kSourceCount>& in, float& left, float& right);

private:
  NOCOPY(Passage)

  // A ring of samples. Write, then read or add some samples back.
  struct Line {
    float* buf;
    uint32_t mask;
    uint32_t w;
    void Clear();
    void Write(const float x) {
      buf[w] = x;
      w = (w + 1) & mask;
    }
    // Samples ago, 0 is the one just written.
    float Read(const uint32_t d) const {
      return buf[(w - 1 - d) & mask];
    }
    float Read(const float d) const {
      auto i = (uint32_t)d;
      auto f = d - (float)i;
      auto a = buf[(w - 1 - i) & mask];
      auto b = buf[(w - 2 - i) & mask];
      return a + (b - a) * f;
    }
    void Add(const uint32_t d, const float x) {
      buf[(w - 1 - d) & mask] += x;
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

  // One metre of tube: the wave going in towards the chamber and the
  // wave coming out, each a delay of the metre, a pole per metre each
  // way for what the stone takes, and the reflection where this section
  // meets the one before it.
  struct Section {
    Line fwd;
    Line bwd;
    OnePole lp_fwd;
    OnePole lp_bwd;
    float r;
  };

  // Where a source goes into the tube. Inside, into both directions at
  // its metre; outside, into the mouth, quieter the further out.
  struct Tap {
    uint8_t section = 0;
    bool both = true;
    uint32_t k_fwd = 0;
    uint32_t k_bwd = 0;
    float g = 0.f;
  };

  // A pinned source. While 'fade' runs down the sound goes in at both
  // the old place and the new, blended, so a sounding source moves
  // without a step.
  struct Source {
    float at = 0.f;                  // metres
    float from = 0.f;                // metres, before the last pin
    uint32_t fade = 0;               // samples left crossing over
    uint32_t fade_length = 0;
    Tap tap;
    Tap from_tap;
    float fade_inv = 0.f;            // 1 over fade_length
    float g_near = 0.f;              // per block, the 1/r excess at the listener
    float g_near_now = 0.f;          // ramped to it across the block
    float g_near_step = 0.f;
    float from_g_near = 0.f;
    float g_send = 0.f;              // into the room, at the level the tube carries
    float from_g_send = 0.f;
    uint32_t send_delay = 0;         // samples, the distance to the chamber
    uint32_t from_send_delay = 0;
  };

  void _pin(Source& s, const float metres, const bool fade);
  Tap _tap(const float metres) const;
  float _pressure(const float metres) const;
  float _coef(const float hz) const;
  float _duct(const float metres) const;
  float _cutoff(const float metres) const;
  float _near_gain(const float metres) const;
  void _colour(Peak& p, const float hz, const float db);

  float _sr;
  float _spm;                // samples per metre
  uint32_t _metre;           // samples in a section
  float _listener;
  float _target;
  float _pace;               // metres per sample
  float _cutoff_scale;
  float _loss;               // gain per metre

  std::array<Section, kSectionCount> _sections;
  std::array<Source, kSourceCount> _sources;
  std::array<Line, kSourceCount> _near_lines;
  Line _side;
  Tap _chamber_tap;

  OnePole _mouth_lp;
  OnePole _back_lp;
  OnePole _open_lp;
  float _p_mouth;            // the pressure at the opening, what outside hears

  float _g_side;
  float _g_side_now;
  float _g_side_step;
  float _g_out;
  float _g_out_now;
  float _g_out_step;
  float _g_outside;          // 1/r from the mouth for a listener outside
  float _g_outside_now;
  float _g_outside_step;
  float _lossed;             // 1 - the gain per metre
  float _b0;                 // what left the mouth end this sample, filtered
  std::array<float, kSectionCount> _entry_fwd;   // what each section took in this sample, before any source went in
  std::array<float, kSectionCount> _entry_bwd;

  Reverb _reverb;
  std::array<Mode, kChamberModes.size()> _modes;
  OnePole _side_lp;
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
  float _width_g_now;
  float _width_g_step;
  OnePole _width_lp_l;
  OnePole _width_lp_r;
};

};
