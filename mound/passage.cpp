#include <cmath>
#include <cstring>
#include "passage.h"

using namespace synthux;
using namespace daisysp;

// The lines live in SDRAM on the board, which the startup code does not
// clear, so Init does. On the desktop they are ordinary statics.
#ifdef STM32H750xx
#define GLOR_SDRAM __attribute__((section(".sdram_bss")))
#else
#define GLOR_SDRAM
#endif
static float GLOR_SDRAM line_memory[Passage::kLineCount][Passage::kLineSize];

static constexpr float kPi = 3.14159265f;
static constexpr float kLn1000 = 6.907755f;

void Passage::Line::Clear() {
  memset(buf, 0, kLineSize * sizeof(float));
  w = 0;
}

Passage::Passage():
  _sr { 48000.f },
  _spm { 0.f },
  _listener { 0.f },
  _target { 0.f },
  _pace { 0.f },
  _cutoff_scale { 1.f },
  _mouth_to_chamber { 0 },
  _g_mouth_to_chamber { 0.f },
  _g_chamber_ret { 0.f },
  _g_mouth_ret { 0.f },
  _g_out { 1.f },
  _width_w { 0 },
  _width_d { 0 },
  _width_g { 0.f } {
  _width_l.fill(0.f);
  _width_r.fill(0.f);
  _width_lp_l = _width_lp_r = { 0.f, 0.f };
  uint8_t n = 0;
  for (auto& line : _lines) line.buf = line_memory[n++];
  _chamber_l.buf = line_memory[n++];
  _chamber_r.buf = line_memory[n++];
  _mouth.buf = line_memory[n++];
  for (auto& m : _modes) m = { 0.f, 0.f, 0.f, 0.f, 0.f };
  _mouth_lp = _chamber_lp_l = _chamber_lp_r = _mouth_ret_lp = { 0.f, 0.f };
  _colour_l = _colour_r = { 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
}

void Passage::Init(const float sample_rate) {
  _sr = sample_rate;
  _spm = sample_rate / kSpeedOfSound;
  _pace = kWalkSpeed / sample_rate;

  for (auto& line : _lines) line.Clear();
  _chamber_l.Clear();
  _chamber_r.Clear();
  _mouth.Clear();

  // The listener starts at the entrance and every source with them.
  // The drum crosses over fast, so a re-pinned strike keeps its attack.
  _listener = _target = kMouthAt;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    s.fade_length = (uint32_t)((i == 0 ? kDrumPinFadeSeconds : kPinFadeSeconds) * sample_rate);
    _pin(s, kMouthAt, false);
  }

  _mouth_to_chamber = (uint32_t)lrintf(fabsf(kChamberAt - kMouthAt) * _spm);
  _g_mouth_to_chamber = _duct(fabsf(kChamberAt - kMouthAt));
  _mouth_lp.a = _coef(kMouthCutoff);
  _width_d = (uint32_t)lrintf(2.f * kWidthMetres * _spm);
  if (_width_d > kWidthLine - 1) _width_d = kWidthLine - 1;
  _width_lp_l.a = _width_lp_r.a = _coef(kWidthCutoff);

  _reverb.Init(sample_rate);
  _reverb.SetFeedback(kChamberFeedback);
  _reverb.SetLpFreq(kChamberLp);

  for (size_t i = 0; i < kChamberModes.size(); i++) {
    auto& mode = kChamberModes[i];
    auto theta = 2.f * kPi * mode.hz / sample_rate;
    auto r = expf(-kLn1000 / (mode.t60 * sample_rate));
    _modes[i].c1 = 2.f * r * cosf(theta);
    _modes[i].c2 = r * r;
    _modes[i].g = mode.gain * 2.f * (1.f - r) * sinf(theta);
    _modes[i].y1 = _modes[i].y2 = 0.f;
  }

  Update();
}

void Passage::SetWalk(const float value) {
  _target = fmap(fclamp(value, 0.f, 1.f), -kOutsideMetres, kPassageMetres + kChamberMetres);
}

void Passage::Pin(const uint8_t source, const float metres, const bool fade) {
  if (source >= kSourceCount) return;
  _sources[source].request = metres;
  _sources[source].request_fade = fade;
  _sources[source].pending.store(true, std::memory_order_release);
}

// A pin that lands inside a running fade starts the new fade from
// whichever tap was carrying most of the sound, so the step is at most
// half of what it would be, and only when two pins land within 10 ms.
void Passage::_pin(Source& s, const float metres, const bool fade) {
  auto settled = s.fade == 0 || s.fade * 2 < s.fade_length;
  if (settled) {
    s.from = s.at;
    s.from_to_chamber = s.to_chamber;
    s.from_to_mouth = s.to_mouth;
    s.from_g_chamber = s.g_chamber;
    s.from_g_mouth = s.g_mouth;
  }
  s.at = metres;
  s.fade = fade ? s.fade_length : 0;
  s.to_chamber = (uint32_t)lrintf(fabsf(metres - kChamberAt) * _spm);
  s.to_mouth = (uint32_t)lrintf(fabsf(metres - kMouthAt) * _spm);
  s.g_chamber = _duct(fabsf(metres - kChamberAt));
  s.g_mouth = _duct(fabsf(metres - kMouthAt));
}

float Passage::_coef(const float hz) const {
  return 1.f - expf(-2.f * kPi * hz / _sr);
}

float Passage::_duct(const float metres) const {
  return powf(10.f, -kDuctLossDb * metres / 20.f);
}

float Passage::_cutoff(const float metres) const {
  return kCutoffNear * exp2f(-metres / kCutoffHalfMetres) * _cutoff_scale;
}

// RBJ peaking filter, flat when the gain is nothing. Coefficients only,
// the state stays.
void Passage::_colour(Peak& p, const float hz, const float db) {
  if (db < 0.01f) {
    p.b0 = 1.f;
    p.b1 = p.b2 = p.a1 = p.a2 = 0.f;
    return;
  }
  auto amp = powf(10.f, db / 40.f);
  auto w0 = 2.f * kPi * hz / _sr;
  auto alpha = sinf(w0) / (2.f * kDuctColourQ);
  auto a0 = 1.f + alpha / amp;
  p.b0 = (1.f + alpha * amp) / a0;
  p.b1 = -2.f * cosf(w0) / a0;
  p.b2 = (1.f - alpha * amp) / a0;
  p.a1 = p.b1;
  p.a2 = (1.f - alpha / amp) / a0;
}

void Passage::Update() {
  for (auto& s : _sources) {
    if (!s.pending.load(std::memory_order_acquire)) continue;
    s.pending.store(false, std::memory_order_relaxed);
    _pin(s, s.request, s.request_fade);
  }

  auto here = _listener;

  // Outside the entrance the mound is behind you: quieter and duller
  // the further you stand from the opening.
  auto outside = fclamp(-here / kOutsideMetres, 0.f, 1.f);
  _g_out = powf(10.f, -kOutsideLossDb * outside / 20.f);
  _cutoff_scale = 1.f - (1.f - kOutsideCutoff) * outside;

  for (auto& s : _sources) {
    auto d = fabsf(here - s.at);
    auto spread = 1.f / daisysp::fmax(1.f, daisysp::fmin(d, kSpreadMetres));
    s.g_direct = spread * _duct(d);
    s.lp.a = _coef(_cutoff(d));
  }

  auto dc = fabsf(here - kChamberAt);
  _g_chamber_ret = _duct(dc);
  _chamber_lp_l.a = _chamber_lp_r.a = _coef(_cutoff(dc));

  auto dm = fabsf(here - kMouthAt);
  _g_mouth_ret = _duct(dm);
  _mouth_ret_lp.a = _coef(_cutoff(dm));

  // The passage's own note, from its height where you stand.
  auto along = fclamp(here / kPassageMetres, 0.f, 1.f);
  auto height = kHeightEntrance + (kHeightChamber - kHeightEntrance) * along;
  auto past = daisysp::fmax(0.f, daisysp::fmax(-here, here - kPassageMetres));
  auto window = fclamp(1.f - past / kDuctColourFade, 0.f, 1.f);
  auto hz = kSpeedOfSound / (2.f * height);
  _colour(_colour_l, hz, kDuctColourDb * window);
  _colour(_colour_r, hz, kDuctColourDb * window);
  _width_g = kWidthGain * window;
}

void Passage::Process(const std::array<float, kSourceCount>& in, float& left, float& right) {
  // The walk ...................................................
  if (_listener < _target) {
    _listener = daisysp::fmin(_listener + _pace, _target);
  } else if (_listener > _target) {
    _listener = daisysp::fmax(_listener - _pace, _target);
  }

  // How far each source still is from where it was, 0 when settled ...
  std::array<float, kSourceCount> blend;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    blend[i] = s.fade > 0 ? (float)s.fade / (float)s.fade_length : 0.f;
    _lines[i].Write(in[i]);
  }

  // The mouth ..................................................
  float to_mouth = 0.f;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    auto x = _lines[i].Read(s.to_mouth) * s.g_mouth;
    if (blend[i] > 0.f) {
      auto was = _lines[i].Read(s.from_to_mouth) * s.from_g_mouth;
      x += (was - x) * blend[i];
    }
    to_mouth += x;
  }
  _mouth.Write(_mouth_lp.Process(to_mouth) * kMouthReflect);

  // The chamber ................................................
  float to_chamber = _mouth.Read(_mouth_to_chamber) * _g_mouth_to_chamber;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    auto x = _lines[i].Read(s.to_chamber) * s.g_chamber;
    if (blend[i] > 0.f) {
      auto was = _lines[i].Read(s.from_to_chamber) * s.from_g_chamber;
      x += (was - x) * blend[i];
    }
    to_chamber += x;
  }
  to_chamber *= kChamberSend;
  float rl, rr;
  _reverb.Process(to_chamber, to_chamber, &rl, &rr);
  float ring = 0.f;
  for (auto& m : _modes) ring += m.Process(to_chamber);
  _chamber_l.Write(rl * kChamberReturn + ring);
  _chamber_r.Write(rr * kChamberReturn + ring);

  // The listener ...............................................
  float direct = 0.f;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    auto x = _lines[i].Read(fabsf(_listener - s.at) * _spm);
    if (blend[i] > 0.f) {
      auto was = _lines[i].Read(fabsf(_listener - s.from) * _spm);
      x += (was - x) * blend[i];
      s.fade--;
    }
    direct += s.lp.Process(x) * s.g_direct;
  }
  auto dc = fabsf(_listener - kChamberAt) * _spm;
  auto cl = _chamber_lp_l.Process(_chamber_l.Read(dc)) * _g_chamber_ret;
  auto cr = _chamber_lp_r.Process(_chamber_r.Read(dc)) * _g_chamber_ret;
  auto dm = fabsf(_listener - kMouthAt) * _spm;
  auto mouth = _mouth_ret_lp.Process(_mouth.Read(dm)) * _g_mouth_ret;

  auto mono = direct + mouth;
  auto l = mono + cl;
  auto r = mono + cr;

  // The walls a metre apart: everything bounces between them, dull .....
  auto rd = (_width_w - _width_d) & (kWidthLine - 1);
  l += _width_lp_l.Process(_width_l[rd]) * _width_g;
  r += _width_lp_r.Process(_width_r[rd]) * _width_g;
  _width_l[_width_w] = l;
  _width_r[_width_w] = r;
  _width_w = (_width_w + 1) & (kWidthLine - 1);

  left = _colour_l.Process(l) * _g_out;
  right = _colour_r.Process(r) * _g_out;
}
