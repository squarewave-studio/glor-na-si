#include "tomb.h"

using namespace synthux;
using namespace daisysp;

Tomb::Tomb() {
  _bus.fill(0.f);
  _reverb_out.fill(0.f);
  _mix.fill(0.f);
};

void Tomb::Init(const float sample_rate, const float buffer_size) {
  _drum.Init(sample_rate);
  _reverb.Init(sample_rate);
  SetWalk(0.f);
};

void Tomb::Strike(const uint8_t index) {
  if (index >= kDrumPadCount) return;
  _drum.Strike(kDrumPitches[index]);
};

void Tomb::SetWalk(const float value) {
  auto walk = fclamp(value, 0.f, 1.f);
  auto back = 1.f - walk;
  _xfade.SetStage(fmap(walk, kWalkMixMin, kWalkMixMax));
  _reverb.SetFeedback(fmap(walk, kWalkFeedbackMin, kWalkFeedbackMax));
  _reverb.SetLpFreq(fmap(back, kWalkLpChamber, kWalkLpEntrance, Mapping::LOG));
  _drum.SetBrightness(fmap(back, kWalkBrightChamber, kWalkBrightEntrance));
  _drum.SetAccent(fmap(back, kWalkAccentChamber, kWalkAccentEntrance));
};

void Tomb::Process(float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    _bus[0] = _bus[1] = _drum.Process();
    _reverb.Process(_bus[0], _bus[1], &(_reverb_out[0]), &(_reverb_out[1]));
    _xfade.Process(_bus[0], _bus[1], _reverb_out[0], _reverb_out[1], _mix[0], _mix[1]);
    out[0][i] = SoftClip(_mix[0]) * kMasterGain;
    out[1][i] = SoftClip(_mix[1]) * kMasterGain;
  }
};
