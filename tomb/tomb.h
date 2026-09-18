// SYNTHUX ACADEMY .........................................
// GLÓR NA SÍ ..............................................
#pragma once
#include <array>

#include <daisysp.h>

#include "nocopy.h"
#include "config.h"

#include "drum.h"
#include "xfade.h"

namespace synthux {

class Tomb {
public:
  Tomb();
  ~Tomb() {}

  void Init(const float sample_rate, const float buffer_size);

  void Strike(const uint8_t index);

  // Position along the passage, 0 at the entrance and 1 in the chamber.
  void SetWalk(const float value);

  // Sample-and-hold at strike time.
  void SetDrumTone(const float value) { _drum.SetStructure(value); }
  void SetDrumDecay(const float value) { _drum.SetDamping(value); }

  void Process(float **out, size_t size);

private:
  NOCOPY(Tomb)

  Drum _drum;
  daisysp::ReverbSc _reverb;
  XFade _xfade;

  std::array<float, 2> _bus;
  std::array<float, 2> _reverb_out;
  std::array<float, 2> _mix;
};

};
