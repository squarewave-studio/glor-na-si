#include <cmath>
#include <cstring>
#include "passage.h"

using namespace synthux;
using namespace daisysp;

// The lines live in the core's own DTCM on the board: 128 KB at zero
// wait states, shared only with the stack. SDRAM and the D2 SRAM are
// both uncached and unbuffered under libDaisy's MPU, and the tube reads
// and writes every section every sample. The startup code does not
// clear this section, so Init does. On the desktop it is an ordinary
// static.
#ifdef STM32H750xx
#define GLOR_LINES __attribute__((section(".dtcmram_bss")))
#else
#define GLOR_LINES
#endif
static float GLOR_LINES line_memory[Passage::kMemory];

static constexpr float kPi = 3.14159265f;
static constexpr float kLn1000 = 6.907755f;

void Passage::Line::Clear() {
  memset(buf, 0, (mask + 1) * sizeof(float));
  w = 0;
}

Passage::Passage():
  _sr { 48000.f },
  _spm { 0.f },
  _metre { 140 },
  _listener { 0.f },
  _target { 0.f },
  _pace { 0.f },
  _cutoff_scale { 1.f },
  _loss { 1.f },
  _p_mouth { 0.f },
  _g_side { 0.f },
  _g_side_now { 0.f },
  _g_side_step { 0.f },
  _g_out { 1.f },
  _g_out_now { 1.f },
  _g_out_step { 0.f },
  _g_outside { 1.f },
  _g_outside_now { 1.f },
  _g_outside_step { 0.f },
  _lossed { 0.f },
  _b0 { 0.f },
  _width_w { 0 },
  _width_d { 0 },
  _width_g { 0.f },
  _width_g_now { 0.f },
  _width_g_step { 0.f } {
  _entry_fwd.fill(0.f);
  _entry_bwd.fill(0.f);
  _width_l.fill(0.f);
  _width_r.fill(0.f);
  _width_lp_l = _width_lp_r = { 0.f, 0.f };
  auto* mem = line_memory;
  for (auto& s : _sections) {
    s.fwd = { mem, kSectionSize - 1, 0 }; mem += kSectionSize;
    s.bwd = { mem, kSectionSize - 1, 0 }; mem += kSectionSize;
    s.lp_fwd = s.lp_bwd = { 1.f, 0.f };
    s.r = 0.f;
  }
  _side = { mem, kReturnSize - 1, 0 }; mem += kReturnSize;
  for (auto& n : _near_lines) { n = { mem, kNearSize - 1, 0 }; mem += kNearSize; }
  for (auto& m : _modes) m = { 0.f, 0.f, 0.f, 0.f, 0.f };
  _mouth_lp = _back_lp = _open_lp = _side_lp = { 0.f, 0.f };
  _colour_l = _colour_r = { 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
}

void Passage::Init(const float sample_rate) {
  _sr = sample_rate;
  _spm = sample_rate / kSpeedOfSound;
  _pace = kWalkSpeed / sample_rate;
  _metre = (uint32_t)lrintf(_spm);
  if (_metre > kSectionSize - 4) _metre = kSectionSize - 4;
  _loss = _duct(1.f);
  _lossed = 1.f - _loss;

  // The tube: every section's lines cleared, its poles set, and the
  // reflection where it meets the section before it, from the two
  // areas. A wave going into a wider section reflects inverted, into a
  // narrower one upright.
  for (uint8_t i = 0; i < kSectionCount; i++) {
    auto& s = _sections[i];
    s.fwd.Clear();
    s.bwd.Clear();
    s.lp_fwd.a = s.lp_bwd.a = _coef(kDuctCutoff);
    s.lp_fwd.y = s.lp_bwd.y = 0.f;
    if (i == 0) {
      s.r = 0.f;
    } else {
      auto a = kSections[i - 1].Area();
      auto b = kSections[i].Area();
      s.r = (a - b) / (a + b);
    }
  }
  _mouth_lp = { _coef(kMouthCutoff), 0.f };
  _back_lp = { _coef(kBackCutoff), 0.f };
  _open_lp = { _coef(kChamberOpenCutoff), 0.f };
  _p_mouth = 0.f;

  _side.Clear();
  for (auto& n : _near_lines) n.Clear();

  // The room's answer goes into the tube where the room is, as a sound
  // of its own, whole.
  _chamber_tap = _tap(kChamberAt);
  _chamber_tap.g = 0.5f;

  // The listener starts at the entrance and every source with them.
  // The drum crosses over fast, so a re-pinned strike keeps its attack.
  _listener = _target = kMouthAt;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    s.fade_length = (uint32_t)((i < 2 ? kDrumPinFadeSeconds : kPinFadeSeconds) * sample_rate);
    s.fade_inv = 1.f / (float)s.fade_length;
    s.fade = 0;
    _pin(s, kMouthAt, false);
    s.g_near_now = s.g_near = s.from_g_near = 0.f;
    s.g_near_step = 0.f;
  }

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

void Passage::PinNow(const uint8_t source, const float metres, const bool fade) {
  if (source >= kSourceCount) return;
  _pin(_sources[source], metres, fade);
}

// Where a sound at this many metres goes into the tube. Inside, into
// both directions at its place in its metre, half each, at the level
// the passage carries once the 1/r spread is over. Outside, into the
// mouth, as much of it as reaches the opening.
Passage::Tap Passage::_tap(const float metres) const {
  Tap t;
  if (metres < kMouthAt) {
    t.section = 0;
    t.both = false;
    t.k_fwd = 0;
    t.k_bwd = 0;
    t.g = kOutsideCouple / daisysp::fmax(1.f, kMouthAt - metres);
    return t;
  }
  auto x = daisysp::fmin(metres, (float)kSectionCount - 0.001f);
  auto i = (uint8_t)x;
  t.section = i;
  t.both = true;
  // Both halves go in at the same sample, one in from either end at
  // most: the sample a metre back has already left the section, and a
  // listener crossing the tap then hears one half give way to the other
  // without a bump.
  t.k_fwd = (uint32_t)lrintf((x - (float)i) * _spm);
  if (t.k_fwd < 1) t.k_fwd = 1;
  if (t.k_fwd > _metre - 1) t.k_fwd = _metre - 1;
  t.k_bwd = _metre - t.k_fwd;
  t.g = 0.5f / kSpreadMetres;
  return t;
}

// A pin that lands inside a running fade starts the new fade from
// whichever tap was carrying most of the sound, so the step is at most
// half of what it would be, and only when two pins land within 10 ms.
void Passage::_pin(Source& s, const float metres, const bool fade) {
  if (fade && s.fade > 0 && metres == s.at) return;   // already on its way there
  auto settled = s.fade == 0 || s.fade * 2 < s.fade_length;
  if (settled) {
    s.from = s.at;
    s.from_tap = s.tap;
    s.from_g_near = s.g_near_now;
    s.from_g_send = s.g_send;
    s.from_send_delay = s.send_delay;
  }
  s.at = metres;
  s.tap = _tap(metres);
  s.fade = fade ? s.fade_length : 0;
  s.g_near = s.g_near_now = _near_gain(fabsf(_listener - metres));   // the new place's gain at once, the old one fades with its tap
  s.g_near_step = 0.f;
  auto d = fabsf(metres - kChamberAt);
  s.g_send = (s.tap.both ? 2.f : 1.f) * s.tap.g * _duct(d);
  s.send_delay = (uint32_t)lrintf(daisysp::fmin(d, kSendMetres) * _spm);
  if (s.send_delay > kNearSize - 4) s.send_delay = kNearSize - 4;
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

// What a source still has over the passage's level this close: 1/r
// less the level the tube carries, nothing from kSpreadMetres out.
float Passage::_near_gain(const float metres) const {
  if (metres >= kSpreadMetres) return 0.f;
  return 1.f / daisysp::fmax(1.f, metres) - 1.f / kSpreadMetres;
}

// The pressure in the tube at this many metres: both waves, read where
// they are in their metre, less the loss they have taken so far in it,
// which is small enough to be linear. The metre's pole and its junction
// are lumped at its ends, so over the last tenth of a metre the wave
// going in crosses over to what the next section took in this sample,
// and over the first tenth the wave coming out crosses to what the
// section before took in, or at the mouth to what left it, and a
// walking listener hears no step at the boundary. What a section took
// in is the propagated wave only, before any source went in, so a
// source sitting on a boundary is heard once on each side of it.
float Passage::_pressure(const float metres) const {
  auto x = fclamp(metres, 0.f, (float)kSectionCount - 0.001f);
  auto i = (uint8_t)x;
  auto& s = _sections[i];
  auto f = x - (float)i;
  auto along = f * (float)_metre;
  auto fwd = s.fwd.Read(along) * (1.f - f * _lossed);
  auto bwd = s.bwd.Read((float)_metre - along) * (1.f - (1.f - f) * _lossed);
  if (f > 1.f - kSeamMetres && i + 1 < kSectionCount) {
    auto w = (f - (1.f - kSeamMetres)) / kSeamMetres;
    fwd += (_entry_fwd[i + 1] - fwd) * w;
  }
  if (f < kSeamMetres) {
    auto w = (kSeamMetres - f) / kSeamMetres;
    bwd += ((i > 0 ? _entry_bwd[i - 1] : _b0) - bwd) * w;
  }
  return fwd + bwd;
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

void Passage::Update(const size_t block) {
  auto here = _listener;
  auto per = 1.f / (float)(block > 0 ? block : 1);

  // Outside the entrance the mound is behind you: quieter and duller
  // the further you stand from the opening, and what you hear of the
  // tube is what leaves the mouth, spreading.
  auto outside = fclamp(-here / kOutsideMetres, 0.f, 1.f);
  _g_out = powf(10.f, -kOutsideLossDb * outside / 20.f);
  _cutoff_scale = 1.f - (1.f - kOutsideCutoff) * outside;
  _g_outside = 1.f / daisysp::fmax(1.f, kMouthAt - here);

  // Gains that follow the listener ramp across the block, so a walk
  // does not step every 2 ms.
  _g_out_step = (_g_out - _g_out_now) * per;
  _g_outside_step = (_g_outside - _g_outside_now) * per;
  for (auto& s : _sources) {
    s.g_near = _near_gain(fabsf(here - s.at));
    s.g_near_step = (s.g_near - s.g_near_now) * per;
    if (s.fade == 0) s.from_g_near = s.g_near;
  }

  auto dc = fabsf(here - kChamberAt);
  _g_side = _duct(dc) * daisysp::fmin(1.f, kSideMetres / daisysp::fmax(dc, kSideMetres));
  _g_side_step = (_g_side - _g_side_now) * per;
  _side_lp.a = _coef(_cutoff(dc));

  // The passage's own note, from its height where you stand.
  auto along = fclamp(here / kPassageMetres, 0.f, 1.f);
  auto height = kHeightEntrance + (kHeightChamber - kHeightEntrance) * along;
  auto past = daisysp::fmax(0.f, daisysp::fmax(-here, here - kPassageMetres));
  auto window = fclamp(1.f - past / kDuctColourFade, 0.f, 1.f);
  auto hz = kSpeedOfSound / (2.f * height);
  _colour(_colour_l, hz, kDuctColourDb * window);
  _colour(_colour_r, hz, kDuctColourDb * window);
  _width_g = kWidthGain * window;
  _width_g_step = (_width_g - _width_g_now) * per;
}

// One step of a ramp, stopping on the target.
static inline void ramp(float& now, const float step, const float target) {
  now += step;
  if ((step > 0.f && now > target) || (step < 0.f && now < target)) now = target;
}

void Passage::Process(const std::array<float, kSourceCount>& in, float& left, float& right) {
  // The walk ...................................................
  if (_listener < _target) {
    _listener = daisysp::fmin(_listener + _pace, _target);
  } else if (_listener > _target) {
    _listener = daisysp::fmax(_listener - _pace, _target);
  }

  // The tube: what leaves every section this sample, less what the
  // metre of stone took ........................................
  std::array<float, kSectionCount> a;   // going in, leaving the far end
  std::array<float, kSectionCount> b;   // coming out, leaving the near end
  for (uint8_t i = 0; i < kSectionCount; i++) {
    auto& s = _sections[i];
    a[i] = s.lp_fwd.Process(s.fwd.Read(_metre - 1)) * _loss;
    b[i] = s.lp_bwd.Process(s.bwd.Read(_metre - 1)) * _loss;
  }

  // The mouth: what arrives goes back inverted, less and less above
  // the opening's own frequency, and the rest leaves .............
  auto reflected = -kMouthReflect * _mouth_lp.Process(b[0]);
  _sections[0].fwd.Write(reflected);
  _entry_fwd[0] = reflected;
  _b0 = b[0];

  // Every junction: the part that turns back is added to both waves.
  // Off the chamber's opening only the low end turns back ......
  for (uint8_t j = 1; j < kSectionCount; j++) {
    auto t = _sections[j].r * (a[j - 1] - b[j]);
    if (j == (uint8_t)kPassageMetres) t = _open_lp.Process(t);
    _sections[j].fwd.Write(a[j - 1] + t);
    _sections[j - 1].bwd.Write(b[j] + t);
    _entry_fwd[j] = a[j - 1] + t;
    _entry_bwd[j - 1] = b[j] + t;
  }

  // The back of the end recess ..................................
  auto back = kBackReflect * _back_lp.Process(a[kSectionCount - 1]);
  _sections[kSectionCount - 1].bwd.Write(back);
  _entry_bwd[kSectionCount - 1] = back;

  // The sources go in where they are pinned, and into their own near
  // line for the listener close by. One blend per source per sample,
  // used everywhere it fades ....................................
  std::array<float, kSourceCount> blend;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    auto x = in[i];
    _near_lines[i].Write(x);
    blend[i] = s.fade > 0 ? (float)s.fade * s.fade_inv : 0.f;
    auto put = [this](const Tap& t, const float v) {
      auto& sec = _sections[t.section];
      sec.fwd.Add(t.k_fwd, v);
      if (t.both) sec.bwd.Add(t.k_bwd, v);
    };
    put(s.tap, x * s.tap.g * (1.f - blend[i]));
    if (blend[i] > 0.f) put(s.from_tap, x * s.from_tap.g * blend[i]);
  }

  // The pressure at the opening, with whatever came in from outside,
  // is what a listener outside hears ............................
  _p_mouth = _sections[0].fwd.Read((uint32_t)0) + b[0];

  // The room, fed by every source at the level the tube carries and
  // late by its distance. Its answer goes into the tube at the chamber,
  // its width down the side line ...............................
  float to_chamber = 0.f;
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    auto x = _near_lines[i].Read(s.send_delay) * s.g_send;
    if (blend[i] > 0.f) {
      auto was = _near_lines[i].Read(s.from_send_delay) * s.from_g_send;
      x += (was - x) * blend[i];
    }
    to_chamber += x;
  }
  to_chamber *= kChamberSend;
  float rl, rr;
  _reverb.Process(to_chamber, to_chamber, &rl, &rr);
  float ring = 0.f;
  for (auto& m : _modes) ring += m.Process(to_chamber);
  auto answer = (rl + rr) * 0.5f * kChamberReturn + ring;
  _sections[_chamber_tap.section].fwd.Add(_chamber_tap.k_fwd, answer * _chamber_tap.g);
  _sections[_chamber_tap.section].bwd.Add(_chamber_tap.k_bwd, answer * _chamber_tap.g);
  _side.Write((rl - rr) * 0.5f * kChamberReturn);

  // The listener: the tube where they stand, or what leaves the
  // mouth if they are outside, plus the near sound of anything close,
  // plus the room's width .......................................
  ramp(_g_outside_now, _g_outside_step, _g_outside);
  float mono = _listener < kMouthAt ? _p_mouth * _g_outside_now : _pressure(_listener);
  for (uint8_t i = 0; i < kSourceCount; i++) {
    auto& s = _sources[i];
    ramp(s.g_near_now, s.g_near_step, s.g_near);
    if (s.g_near_now <= 0.f && s.from_g_near <= 0.f) continue;
    auto x = _near_lines[i].Read(fabsf(_listener - s.at) * _spm) * s.g_near_now;
    if (blend[i] > 0.f) {
      auto was = _near_lines[i].Read(fabsf(_listener - s.from) * _spm) * s.from_g_near;
      x += (was - x) * blend[i];
    }
    mono += x;
  }
  for (auto& s : _sources) if (s.fade > 0) s.fade--;
  ramp(_g_side_now, _g_side_step, _g_side);
  auto dc = daisysp::fmin(fabsf(_listener - kChamberAt) * _spm, (float)(kReturnSize - 4));
  auto side = _side_lp.Process(_side.Read(dc)) * _g_side_now;
  auto l = mono + side;
  auto r = mono - side;

  // The walls a metre apart: everything bounces between them, dull .....
  ramp(_width_g_now, _width_g_step, _width_g);
  auto rd = (_width_w - _width_d) & (kWidthLine - 1);
  l += _width_lp_l.Process(_width_l[rd]) * _width_g_now;
  r += _width_lp_r.Process(_width_r[rd]) * _width_g_now;
  _width_l[_width_w] = l;
  _width_r[_width_w] = r;
  _width_w = (_width_w + 1) & (kWidthLine - 1);

  ramp(_g_out_now, _g_out_step, _g_out);
  left = _colour_l.Process(l) * _g_out_now;
  right = _colour_r.Process(r) * _g_out_now;
}
